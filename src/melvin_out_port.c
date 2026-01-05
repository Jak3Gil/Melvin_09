/*
 * Melvin: Output Port Handler
 * 
 * External output port handler that reads from .m files and routes to devices.
 * Handles output routing based on input port ID and sends to output devices.
 * 
 * This file is external to melvin.c - it handles device-specific I/O
 * while melvin.c processes bytes according to README.md rules.
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/* ============================================================================
 * ROUTING TABLE
 * ============================================================================ */

/* Port routing entry */
typedef struct PortRoute {
    uint8_t input_port;
    uint8_t output_port;
} PortRoute;

/* Static routing table */
static PortRoute routing_table[256];
static size_t routing_table_size = 0;

/* Set routing table
 * - Configures input_port → output_port mappings
 * - Called by application code
 */
void melvin_out_port_set_routing_table(const PortRoute *routes, size_t count) {
    if (!routes || count == 0) {
        routing_table_size = 0;
        return;
    }
    
    size_t copy_count = (count > 256) ? 256 : count;
    memcpy(routing_table, routes, copy_count * sizeof(PortRoute));
    routing_table_size = copy_count;
}

/* Get output port for input port
 * - Looks up routing table
 * - Returns 0 if no route found
 */
uint8_t melvin_out_port_get_output_port(uint8_t input_port) {
    for (size_t i = 0; i < routing_table_size; i++) {
        if (routing_table[i].input_port == input_port) {
            return routing_table[i].output_port;
        }
    }
    return 0;  // No route found
}

/* ============================================================================
 * DEVICE-SPECIFIC OUTPUT HANDLERS
 * ============================================================================ */

/* Forward declaration */
static int melvin_out_port_send_to_device(uint8_t port_id, 
                                           const uint8_t *data, 
                                           size_t size);

/* ============================================================================
 * OUTPUT READING AND ROUTING
 * ============================================================================ */

/* Read output and route to device
 * - Reads from .m file universal output
 * - Gets input port ID for routing decision
 * - Routes to appropriate output device
 * Returns: 0 on success (or no output), -1 on error
 */
int melvin_out_port_read_and_route(MelvinMFile *mfile) {
    if (!mfile) return -1;
    
    // 1. Check if output exists
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size == 0) {
        return 0;  // No output (pure thinking mode), not an error
    }
    
    // 2. Get input port (for routing decision)
    uint8_t input_port = melvin_m_get_last_input_port_id(mfile);
    
    // 3. Lookup output port
    uint8_t output_port = melvin_out_port_get_output_port(input_port);
    if (output_port == 0) {
        // No route configured, clear output and return
        melvin_m_universal_output_clear(mfile);
        return 0;  // Not an error, just no route
    }
    
    // 4. Read output from .m file
    uint8_t output[8192];  // Buffer for output
    size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    if (read_size == 0) {
        return -1;  // Error reading output
    }
    
    // 5. Send to output device
    int result = melvin_out_port_send_to_device(output_port, output, read_size);
    
    // 6. Clear output buffer after reading
    melvin_m_universal_output_clear(mfile);
    
    return result;
}

/* Send data to specific device
 * - Routes based on port_id
 * - Handles different device types
 * Returns: 0 on success, -1 on error
 */
static int melvin_out_port_send_to_device(uint8_t port_id, 
                                    const uint8_t *data, 
                                    size_t size) {
    if (!data || size == 0) return -1;
    
    // Route to appropriate device based on port_id
    // For now, implement common cases:
    
    // Port 0: stdout (text output)
    if (port_id == 0) {
        fwrite(data, 1, size, stdout);
        fflush(stdout);
        return 0;
    }
    
    // Port 1: stderr (error output)
    if (port_id == 1) {
        fwrite(data, 1, size, stderr);
        fflush(stderr);
        return 0;
    }
    
    // Port 2: File output (example: "output.txt")
    if (port_id == 2) {
        FILE *f = fopen("output.txt", "ab");  // Append mode
        if (!f) return -1;
        size_t written = fwrite(data, 1, size, f);
        fclose(f);
        return (written == size) ? 0 : -1;
    }
    
    // Default: stdout
    fwrite(data, 1, size, stdout);
    fflush(stdout);
    return 0;
}

/* Handle text file output
 * - Writes output to specified file
 */
int melvin_out_port_handle_text_file(uint8_t port_id, 
                                      const char *filename,
                                      const uint8_t *data,
                                      size_t size) {
    (void)port_id;  // Unused parameter
    if (!filename || !data || size == 0) return -1;
    
    FILE *f = fopen(filename, "ab");  // Append mode
    if (!f) return -1;
    
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    
    return (written == size) ? 0 : -1;
}

/* Handle stdout output
 * - Writes to stdout
 */
int melvin_out_port_handle_stdout(const uint8_t *data, size_t size) {
    if (!data || size == 0) return -1;
    
    fwrite(data, 1, size, stdout);
    fflush(stdout);
    return 0;
}

/* ============================================================================
 * ERROR FEEDBACK (Error-Based Learning)
 * ============================================================================ */

/* Compare output to expected and compute error signal
 * Returns: error signal in [0.0, 1.0] where 1.0 = perfect match
 * 
 * This is the EXTERNAL error computation - can be customized per application
 * (e.g., LLM comparison, human feedback, test framework, etc.)
 * 
 * Algorithm:
 * - Character-level matching (simple edit distance)
 * - Exact matches get highest score
 * - Penalizes length differences
 */
float melvin_out_port_compare_output(const uint8_t *output, size_t output_size,
                                     const uint8_t *expected, size_t expected_size) {
    if (!output || output_size == 0) {
        // No output = wrong (unless expected is also empty)
        return (expected_size == 0) ? 1.0f : 0.0f;
    }
    
    if (!expected || expected_size == 0) {
        // No expected = can't compute error, return neutral
        return 0.5f;
    }
    
    // Compute character-level match (simple edit distance)
    size_t matches = 0;
    size_t min_len = (output_size < expected_size) ? output_size : expected_size;
    size_t max_len = (output_size > expected_size) ? output_size : expected_size;
    
    // Count exact character matches
    for (size_t i = 0; i < min_len; i++) {
        if (output[i] == expected[i]) {
            matches++;
        }
    }
    
    // Error signal = match ratio (penalize length differences)
    float match_ratio = (max_len > 0) ? ((float)matches / (float)max_len) : 0.0f;
    
    // Optional: boost exact matches
    if (output_size == expected_size && matches == output_size) {
        match_ratio = 1.0f;  // Perfect match
    }
    
    return match_ratio;
}

/* Process output with error feedback
 * - Reads output from .m file
 * - Compares to expected (if provided)
 * - Feeds error signal back to .m file (ERROR-BASED LEARNING)
 * - Routes output to device (existing functionality)
 * 
 * This combines error-based learning with output routing.
 * If expected_output is NULL, operates in frequency-only mode (no error feedback).
 * 
 * Returns: 0 on success, -1 on error
 */
int melvin_out_port_process_with_feedback(MelvinMFile *mfile,
                                          const uint8_t *expected_output,
                                          size_t expected_size) {
    if (!mfile) return -1;
    
    // 1. Read output
    uint8_t output[8192];
    size_t output_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    if (output_size == 0) {
        return 0;  // No output
    }
    
    // 2. Compute error signal (EXTERNAL)
    float error_signal = 1.0f;  // Default: assume correct (no feedback)
    
    if (expected_output && expected_size > 0) {
        error_signal = melvin_out_port_compare_output(output, output_size,
                                                      expected_output, expected_size);
    }
    
    // 3. Feed error back to core (ERROR-BASED LEARNING)
    if (expected_output) {  // Only if we have expected output
        melvin_m_feedback_error(mfile, error_signal);
    }
    
    // 4. Route output to device (existing functionality)
    uint8_t input_port = melvin_m_get_last_input_port_id(mfile);
    uint8_t output_port = melvin_out_port_get_output_port(input_port);
    
    if (output_port != 0) {
        melvin_out_port_send_to_device(output_port, output, output_size);
    }
    
    // 5. Clear output buffer
    melvin_m_universal_output_clear(mfile);
    
    return 0;
}

/* Display output in terminal (for LLM-like interaction)
 * - Reads from .m file universal output
 * - Displays directly to stdout
 * - No routing needed, just shows the generated output
 * Returns: 0 on success, -1 on error
 */
int melvin_out_port_display_output(MelvinMFile *mfile) {
    if (!mfile) return -1;
    
    // Check if output exists
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size == 0) {
        return 0;  // No output, not an error
    }
    
    // Read output
    uint8_t output[8192];
    size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    if (read_size == 0) {
        return -1;
    }
    
    // Display in terminal (with better formatting)
    if (read_size > 0) {
        printf("Output: \"");
        // Print readable characters, show hex for non-printable
        for (size_t i = 0; i < read_size; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                putchar(output[i]);
            } else {
                printf("\\x%02x", output[i]);
            }
        }
        printf("\" (%zu bytes)\n", read_size);
    } else {
        printf("Output: (none - pure thinking mode)\n");
    }
    fflush(stdout);
    
    // Clear output buffer
    melvin_m_universal_output_clear(mfile);
    
    return 0;
}

