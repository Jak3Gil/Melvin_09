/*
 * Melvin: Input Port Handler
 * 
 * External input port handler that connects devices to .m files.
 * Handles packaging data as PortFrame and writing to universal input.
 * 
 * This file is external to melvin.c - it handles device-specific I/O
 * while melvin.c processes bytes according to README.md rules.
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* ============================================================================
 * PORT FRAME SERIALIZATION
 * ============================================================================ */

/* Serialize PortFrame to buffer
 * Format: [port_id (1)] + [timestamp (8)] + [data_size (4)] + [data (N)]
 * Returns: Size of serialized data, or 0 on error
 */
size_t port_frame_serialize(const PortFrame *frame, uint8_t *buffer, size_t buffer_size) {
    if (!frame || !buffer || buffer_size < 13) return 0;  // Minimum: 1+8+4
    
    size_t total_size = 1 + 8 + 4 + frame->data_size;
    if (buffer_size < total_size) return 0;
    
    size_t offset = 0;
    
    // Port ID (1 byte)
    buffer[offset++] = frame->port_id;
    
    // Timestamp (8 bytes, little-endian)
    uint64_t timestamp = frame->timestamp;
    for (int i = 0; i < 8; i++) {
        buffer[offset++] = (uint8_t)(timestamp & 0xFF);
        timestamp >>= 8;
    }
    
    // Data size (4 bytes, little-endian)
    uint32_t data_size = frame->data_size;
    for (int i = 0; i < 4; i++) {
        buffer[offset++] = (uint8_t)(data_size & 0xFF);
        data_size >>= 8;
    }
    
    // Data (N bytes)
    if (frame->data_size > 0 && frame->data) {
        memcpy(buffer + offset, frame->data, frame->data_size);
        offset += frame->data_size;
    }
    
    return offset;
}

/* Deserialize buffer to PortFrame
 * Note: This function is for reference only - flexible array members
 * cannot be assigned. Use port_frame_serialize() and access data
 * directly from buffer at offset 13.
 * Returns: 0 on success, -1 on error
 */
int port_frame_deserialize(const uint8_t *buffer, size_t buffer_size, PortFrame *frame) {
    if (!buffer || !frame || buffer_size < 13) return -1;
    
    size_t offset = 0;
    
    // Port ID (1 byte)
    frame->port_id = buffer[offset++];
    
    // Timestamp (8 bytes, little-endian)
    frame->timestamp = 0;
    for (int i = 0; i < 8; i++) {
        frame->timestamp |= ((uint64_t)buffer[offset++]) << (i * 8);
    }
    
    // Data size (4 bytes, little-endian)
    frame->data_size = 0;
    for (int i = 0; i < 4; i++) {
        frame->data_size |= ((uint32_t)buffer[offset++]) << (i * 8);
    }
    
    // Validate size
    if (buffer_size < offset + frame->data_size) return -1;
    
    // Note: frame->data is a flexible array member and cannot be assigned
    // The data starts at buffer[offset], but we can't assign to frame->data
    // This function is mainly for extracting metadata
    
    return 0;
}

/* Get current timestamp (microseconds since epoch) */
static uint64_t get_timestamp(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

/* Forward declaration for chunked processing */
static int melvin_in_port_handle_text_file_chunked_internal(MelvinMFile *mfile,
                                                             uint8_t port_id,
                                                             const char *filename,
                                                             size_t chunk_size);

/* ============================================================================
 * MAIN INPUT PORT HANDLER
 * ============================================================================ */

/* Process data from a device
 * - Packages data as PortFrame
 * - Writes to .m file universal input
 * - Triggers processing
 * Returns: 0 on success, -1 on error
 */
int melvin_in_port_process_device(MelvinMFile *mfile, 
                                   uint8_t port_id,
                                   const uint8_t *raw_data,
                                   size_t data_size) {
    if (!mfile || !raw_data || data_size == 0) return -1;
    
    // Store port_id in mfile for routing (NOT in graph data)
    // This is metadata for output routing, not part of the learned patterns
    melvin_m_set_last_input_port_id(mfile, port_id);
    
    // Write ONLY raw data to universal input (no PortFrame wrapper)
    // Nodes will get port_id set during processing
    melvin_m_universal_input_write(mfile, raw_data, data_size);
    
    // Process (triggers wave propagation, learning, output generation)
    int result = melvin_m_process_input(mfile);
    
    return result;
}

/* ============================================================================
 * DEVICE-SPECIFIC HANDLERS (Examples)
 * ============================================================================ */

/* Handle text file input
 * - Reads file content
 * - Packages as port frame
 * - Processes through .m file
 * - Automatically uses chunked processing for files > 100MB
 */
int melvin_in_port_handle_text_file(MelvinMFile *mfile, 
                                     uint8_t port_id, 
                                     const char *filename) {
    if (!mfile || !filename) return -1;
    
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    
    // Get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    fclose(f);
    
    if (file_size <= 0) {
        return 0;  // Empty file, not an error
    }
    
    // For files > 100MB, automatically use chunked processing
    if (file_size > 100 * 1024 * 1024) {
        return melvin_in_port_handle_text_file_chunked_internal(mfile, port_id, filename, 0);
    }
    
    // Small files: load entire file into memory
    f = fopen(filename, "rb");
    if (!f) return -1;
    
    uint8_t *text = malloc(file_size);
    if (!text) {
        fclose(f);
        return -1;
    }
    
    size_t bytes_read = fread(text, 1, file_size, f);
    fclose(f);
    
    if (bytes_read != (size_t)file_size) {
        free(text);
        return -1;
    }
    
    // Set port_id for routing
    melvin_m_set_last_input_port_id(mfile, port_id);
    
    // Write raw data directly to universal input (no PortFrame wrapper)
    melvin_m_universal_input_write(mfile, text, bytes_read);
    int result = melvin_m_process_input(mfile);
    
    free(text);
    return result;
}

/* Handle stdin input
 * - Reads from stdin
 * - Packages as port frame
 * - Processes through .m file
 */
int melvin_in_port_handle_stdin(MelvinMFile *mfile, uint8_t port_id) {
    if (!mfile) return -1;
    
    uint8_t buffer[4096];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), stdin);
    
    if (bytes_read == 0) {
        return 0;  // No data, not an error
    }
    
    return melvin_in_port_process_device(mfile, port_id, buffer, bytes_read);
}

/* Handle raw buffer input
 * - Direct buffer input (for programmatic use)
 * - Packages as port frame
 * - Processes through .m file
 */
int melvin_in_port_handle_buffer(MelvinMFile *mfile, 
                                  uint8_t port_id,
                                  const uint8_t *buffer,
                                  size_t buffer_size) {
    if (!mfile || !buffer || buffer_size == 0) return -1;
    
    return melvin_in_port_process_device(mfile, port_id, buffer, buffer_size);
}

/* Handle large text file in chunks (for datasets like 40GB)
 * - Processes file in chunks to avoid loading entire file into memory
 * - Shows progress for large files
 * - Memory-efficient for any file size
 * - chunk_size: Size of each chunk (0 = default 1MB)
 * Returns: Number of chunks processed, or -1 on error
 */
static int melvin_in_port_handle_text_file_chunked_internal(MelvinMFile *mfile,
                                             uint8_t port_id,
                                             const char *filename,
                                             size_t chunk_size) {
    if (!mfile || !filename) return -1;
    
    // Default chunk size: 1MB (good balance for 40GB files)
    if (chunk_size == 0) {
        chunk_size = 1024 * 1024;  // 1MB
    }
    
    FILE *f = fopen(filename, "rb");
    if (!f) return -1;
    
    // Get file size for progress tracking
    fseek(f, 0, SEEK_END);
    long long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(f);
        return 0;  // Empty file
    }
    
    // Allocate chunk buffer
    uint8_t *buffer = malloc(chunk_size);
    if (!buffer) {
        fclose(f);
        return -1;
    }
    
    int chunks_processed = 0;
    long long bytes_processed = 0;
    
    // Process file in chunks
    while (!feof(f)) {
        size_t bytes_read = fread(buffer, 1, chunk_size, f);
        
        if (bytes_read == 0) break;
        
        // Process this chunk
        if (melvin_in_port_process_device(mfile, port_id, buffer, bytes_read) == 0) {
            chunks_processed++;
            bytes_processed += bytes_read;
            
            // Progress output (every 100 chunks or every 100MB)
            if (chunks_processed % 100 == 0 || bytes_processed % (100 * 1024 * 1024) == 0) {
                double percent = (double)bytes_processed * 100.0 / file_size;
                fprintf(stderr, "\rProcessing: %.2f%% (%lld / %lld bytes, %d chunks)",
                       percent, bytes_processed, file_size, chunks_processed);
                fflush(stderr);
            }
        } else {
            // Processing error
            free(buffer);
            fclose(f);
            return -1;
        }
    }
    
    fprintf(stderr, "\nCompleted: %lld bytes in %d chunks\n", bytes_processed, chunks_processed);
    
    free(buffer);
    fclose(f);
    return chunks_processed;
}

/* Public wrapper for chunked processing */
int melvin_in_port_handle_text_file_chunked(MelvinMFile *mfile,
                                            uint8_t port_id,
                                            const char *filename,
                                            size_t chunk_size) {
    return melvin_in_port_handle_text_file_chunked_internal(mfile, port_id, filename, chunk_size);
}

