/*
 * Melvin: Universal Dataset Processor
 * 
 * Auto-detects file type and routes to correct port
 * Processes any dataset (text/audio/image/video/code) through the pipeline
 */

#include "../src/melvin.h"
#include "../src/melvin_in_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

/* File type detection */
typedef enum {
    FILE_TYPE_UNKNOWN,
    FILE_TYPE_TEXT,
    FILE_TYPE_AUDIO,
    FILE_TYPE_IMAGE,
    FILE_TYPE_VIDEO,
    FILE_TYPE_CODE,
    FILE_TYPE_BINARY
} FileType;

/* Detect file type from extension */
static FileType detect_file_type_from_extension(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return FILE_TYPE_UNKNOWN;
    
    ext++;  // Skip '.'
    
    // Text files
    if (strcasecmp(ext, "txt") == 0 || strcasecmp(ext, "md") == 0 ||
        strcasecmp(ext, "json") == 0 || strcasecmp(ext, "csv") == 0 ||
        strcasecmp(ext, "xml") == 0 || strcasecmp(ext, "html") == 0 ||
        strcasecmp(ext, "log") == 0) {
        return FILE_TYPE_TEXT;
    }
    
    // Audio files
    if (strcasecmp(ext, "wav") == 0 || strcasecmp(ext, "mp3") == 0 ||
        strcasecmp(ext, "flac") == 0 || strcasecmp(ext, "ogg") == 0 ||
        strcasecmp(ext, "aac") == 0 || strcasecmp(ext, "m4a") == 0) {
        return FILE_TYPE_AUDIO;
    }
    
    // Image files
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0 ||
        strcasecmp(ext, "png") == 0 || strcasecmp(ext, "gif") == 0 ||
        strcasecmp(ext, "bmp") == 0 || strcasecmp(ext, "tiff") == 0 ||
        strcasecmp(ext, "webp") == 0) {
        return FILE_TYPE_IMAGE;
    }
    
    // Video files
    if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 ||
        strcasecmp(ext, "mov") == 0 || strcasecmp(ext, "mkv") == 0 ||
        strcasecmp(ext, "webm") == 0 || strcasecmp(ext, "flv") == 0) {
        return FILE_TYPE_VIDEO;
    }
    
    // Code files
    if (strcasecmp(ext, "c") == 0 || strcasecmp(ext, "cpp") == 0 ||
        strcasecmp(ext, "h") == 0 || strcasecmp(ext, "hpp") == 0 ||
        strcasecmp(ext, "py") == 0 || strcasecmp(ext, "js") == 0 ||
        strcasecmp(ext, "java") == 0 || strcasecmp(ext, "go") == 0 ||
        strcasecmp(ext, "rs") == 0 || strcasecmp(ext, "swift") == 0 ||
        strcasecmp(ext, "kt") == 0 || strcasecmp(ext, "ts") == 0 ||
        strcasecmp(ext, "sh") == 0 || strcasecmp(ext, "bash") == 0) {
        return FILE_TYPE_CODE;
    }
    
    return FILE_TYPE_UNKNOWN;
}

/* Detect file type from magic bytes */
static FileType detect_file_type_from_magic(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return FILE_TYPE_UNKNOWN;
    
    uint8_t magic[16];
    size_t read = fread(magic, 1, 16, f);
    fclose(f);
    
    if (read < 4) return FILE_TYPE_UNKNOWN;
    
    // PNG
    if (magic[0] == 0x89 && magic[1] == 0x50 && 
        magic[2] == 0x4E && magic[3] == 0x47) {
        return FILE_TYPE_IMAGE;
    }
    
    // JPEG
    if (magic[0] == 0xFF && magic[1] == 0xD8) {
        return FILE_TYPE_IMAGE;
    }
    
    // GIF
    if (magic[0] == 'G' && magic[1] == 'I' && magic[2] == 'F') {
        return FILE_TYPE_IMAGE;
    }
    
    // WAV
    if (read >= 12 && magic[0] == 'R' && magic[1] == 'I' && 
        magic[2] == 'F' && magic[3] == 'F' &&
        magic[8] == 'W' && magic[9] == 'A' && 
        magic[10] == 'V' && magic[11] == 'E') {
        return FILE_TYPE_AUDIO;
    }
    
    // MP4
    if (read >= 8 && magic[4] == 'f' && magic[5] == 't' &&
        magic[6] == 'y' && magic[7] == 'p') {
        return FILE_TYPE_VIDEO;
    }
    
    // MP3 (ID3 tag or frame sync)
    if ((magic[0] == 0xFF && (magic[1] & 0xE0) == 0xE0) ||
        (magic[0] == 'I' && magic[1] == 'D' && magic[2] == '3')) {
        return FILE_TYPE_AUDIO;
    }
    
    return FILE_TYPE_UNKNOWN;
}

/* Detect file type (extension + magic bytes) */
static FileType detect_file_type(const char *filename) {
    FileType type = detect_file_type_from_extension(filename);
    if (type != FILE_TYPE_UNKNOWN) {
        return type;
    }
    
    return detect_file_type_from_magic(filename);
}

/* Map file type to port */
static uint8_t file_type_to_port(FileType type) {
    switch (type) {
        case FILE_TYPE_TEXT:
        case FILE_TYPE_CODE:
            return PORT_TEXT;
        case FILE_TYPE_AUDIO:
            return PORT_AUDIO;
        case FILE_TYPE_IMAGE:
        case FILE_TYPE_VIDEO:
            return PORT_VIDEO;
        default:
            return PORT_TEXT;  // Default to text
    }
}

/* Format file size for display */
static void format_size(long long size, char *buffer, size_t buffer_size) {
    if (size >= 1024LL * 1024LL * 1024LL) {
        snprintf(buffer, buffer_size, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    } else if (size >= 1024LL * 1024LL) {
        snprintf(buffer, buffer_size, "%.2f MB", size / (1024.0 * 1024.0));
    } else if (size >= 1024LL) {
        snprintf(buffer, buffer_size, "%.2f KB", size / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%lld B", size);
    }
}

/* Process file with progress display */
static int process_file_with_progress(MelvinMFile *mfile,
                                      const char *filename,
                                      uint8_t port_id,
                                      int passes,
                                      int watch_mode) {
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Error: Cannot access file %s\n", filename);
        return -1;
    }
    
    long long file_size = st.st_size;
    char size_str[64];
    format_size(file_size, size_str, sizeof(size_str));
    
    const char *port_name = "Unknown";
    switch (port_id) {
        case PORT_TEXT: port_name = "Text"; break;
        case PORT_AUDIO: port_name = "Audio"; break;
        case PORT_VIDEO: port_name = "Video"; break;
        case PORT_CANBUS: port_name = "CAN Bus"; break;
        case PORT_MOTOR: port_name = "Motor"; break;
        case PORT_SENSOR: port_name = "Sensor"; break;
    }
    
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║ Melvin Dataset Processor                                  ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║ File:    %-45s ║\n", filename);
    printf("║ Size:    %-45s ║\n", size_str);
    printf("║ Port:    0x%02X (%-40s) ║\n", port_id, port_name);
    printf("║ Passes:  %-45d ║\n", passes);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");
    
    // Get initial stats
    size_t initial_nodes = melvin_m_get_node_count(mfile);
    size_t initial_edges = melvin_m_get_edge_count(mfile);
    uint64_t initial_adaptations = melvin_m_get_adaptation_count(mfile);
    
    time_t start_time = time(NULL);
    
    for (int pass = 1; pass <= passes; pass++) {
        if (passes > 1) {
            printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
            printf("Pass %d/%d\n", pass, passes);
            printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        }
        
        // Use chunked processing for large files (>100MB)
        int result;
        if (file_size > 100 * 1024 * 1024) {
            // Large file: use chunked processing
            result = melvin_in_port_handle_text_file_chunked(
                mfile, port_id, filename, 0);
        } else {
            // Small file: process directly
            result = melvin_in_port_handle_text_file(
                mfile, port_id, filename);
        }
        
        if (result < 0) {
            fprintf(stderr, "Error processing file\n");
            return -1;
        }
        
        // Show progress
        size_t current_nodes = melvin_m_get_node_count(mfile);
        size_t current_edges = melvin_m_get_edge_count(mfile);
        uint64_t current_adaptations = melvin_m_get_adaptation_count(mfile);
        
        size_t new_nodes = current_nodes - initial_nodes;
        size_t new_edges = current_edges - initial_edges;
        uint64_t new_adaptations = current_adaptations - initial_adaptations;
        
        time_t current_time = time(NULL);
        double elapsed = difftime(current_time, start_time);
        double rate = (file_size > 0 && elapsed > 0) ? 
                     (file_size / elapsed) : 0;
        
        char rate_str[64];
        format_size((long long)rate, rate_str, sizeof(rate_str));
        
        printf("\nProgress:\n");
        printf("  Nodes:      %zu (+%zu)\n", current_nodes, new_nodes);
        printf("  Edges:      %zu (+%zu)\n", current_edges, new_edges);
        printf("  Adaptations: %llu (+%llu)\n", 
               (unsigned long long)current_adaptations,
               (unsigned long long)new_adaptations);
        printf("  Rate:       %s/s\n", rate_str);
        printf("  Time:       %.1f seconds\n", elapsed);
        
        if (watch_mode) {
            // Show detailed stats
            double node_growth = initial_nodes > 0 ? 
                (100.0 * new_nodes / initial_nodes) : 0.0;
            double edge_growth = initial_edges > 0 ?
                (100.0 * new_edges / initial_edges) : 0.0;
            double edge_per_node = current_nodes > 0 ?
                (double)current_edges / current_nodes : 0.0;
            
            printf("\nStatistics:\n");
            printf("  Node growth:    %.2f%%\n", node_growth);
            printf("  Edge growth:    %.2f%%\n", edge_growth);
            printf("  Edges/Node:     %.2f\n", edge_per_node);
        }
        
        // Save checkpoint
        if (melvin_m_save(mfile) == 0) {
            printf("  ✓ Checkpoint saved\n");
        } else {
            printf("  ⚠ Checkpoint save failed\n");
        }
        
        if (pass < passes) {
            printf("\n");
        }
    }
    
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("Complete\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    printf("Final: %zu nodes, %zu edges\n", final_nodes, final_edges);
    
    return 0;
}

/* Print usage */
static void print_usage(const char *prog_name) {
    printf("Usage: %s <dataset_file> <brain.m> [options]\n\n", prog_name);
    printf("Options:\n");
    printf("  --port PORT_ID    Force port (0x01=text, 0x02=audio, 0x03=video, etc.)\n");
    printf("  --passes N        Number of passes (default: 1)\n");
    printf("  --watch           Watch mode (detailed statistics)\n");
    printf("  --help            Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s dataset.txt brain.m\n", prog_name);
    printf("  %s audio.wav brain.m --passes 3\n", prog_name);
    printf("  %s image.jpg brain.m --watch\n", prog_name);
    printf("  %s code.c brain.m --port 0x01\n", prog_name);
    printf("\nSupported file types:\n");
    printf("  Text:   .txt, .md, .json, .csv, .xml, .html, .log\n");
    printf("  Audio:  .wav, .mp3, .flac, .ogg, .aac, .m4a\n");
    printf("  Image:  .jpg, .jpeg, .png, .gif, .bmp, .tiff, .webp\n");
    printf("  Video:  .mp4, .avi, .mov, .mkv, .webm, .flv\n");
    printf("  Code:   .c, .cpp, .h, .py, .js, .java, .go, .rs, etc.\n");
}

/* Main function */
int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Check for help
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    const char *dataset_file = argv[1];
    const char *brain_file = argv[2];
    
    // Parse options
    uint8_t port_id = 0;  // 0 = auto-detect
    int passes = 1;
    int watch_mode = 0;
    
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port_id = (uint8_t)strtol(argv[++i], NULL, 0);
        } else if (strcmp(argv[i], "--passes") == 0 && i + 1 < argc) {
            passes = atoi(argv[++i]);
            if (passes < 1) passes = 1;
        } else if (strcmp(argv[i], "--watch") == 0) {
            watch_mode = 1;
        }
    }
    
    // Auto-detect port if not specified
    if (port_id == 0) {
        FileType type = detect_file_type(dataset_file);
        port_id = file_type_to_port(type);
        
        const char *type_name = "Unknown";
        switch (type) {
            case FILE_TYPE_TEXT: type_name = "Text"; break;
            case FILE_TYPE_AUDIO: type_name = "Audio"; break;
            case FILE_TYPE_IMAGE: type_name = "Image"; break;
            case FILE_TYPE_VIDEO: type_name = "Video"; break;
            case FILE_TYPE_CODE: type_name = "Code"; break;
            default: type_name = "Binary/Unknown"; break;
        }
        
        printf("Auto-detected: %s -> %s (Port 0x%02X)\n", 
               dataset_file, type_name, port_id);
    }
    
    // Load or create brain
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        printf("Creating new brain: %s\n", brain_file);
        mfile = melvin_m_create(brain_file);
        if (!mfile) {
            fprintf(stderr, "Error: Cannot create brain file\n");
            return 1;
        }
    } else {
        size_t nodes = melvin_m_get_node_count(mfile);
        size_t edges = melvin_m_get_edge_count(mfile);
        printf("Loaded existing brain: %s (%zu nodes, %zu edges)\n", 
               brain_file, nodes, edges);
    }
    
    // Process file
    int result = process_file_with_progress(mfile, dataset_file, 
                                           port_id, passes, watch_mode);
    
    melvin_m_close(mfile);
    return result;
}

