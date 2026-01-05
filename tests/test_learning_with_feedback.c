/*
 * Test Learning with Error Feedback
 * 
 * Tests learning rate with error feedback enabled
 * Usage: ./test_learning_with_feedback <train_file> <query_file> <brain.m> <expected_output>
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 5) {
        fprintf(stderr, "Usage: %s <train_file> <query_file> <brain.m> <expected_output> [no_feedback]\n", argv[0]);
        fprintf(stderr, "  If 'no_feedback' is provided, error feedback is not applied (measurement only)\n");
        return 1;
    }
    
    const char *train_file = argv[1];
    const char *query_file = argv[2];
    const char *brain_path = argv[3];
    const char *expected_output = argv[4];
    int apply_feedback = (argc < 6 || strcmp(argv[5], "no_feedback") != 0);
    
    // Load or create brain
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to load brain file\n");
        return 1;
    }
    
    // Process query input ONLY (training is done by the script separately)
    // This prevents double-training which causes edges to strengthen too fast
    int result = melvin_in_port_handle_text_file(mfile, 0, query_file);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to process query file\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    // Read output
    uint8_t output[8192];
    size_t output_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    // Compute error signal and apply feedback (if enabled)
    float error_signal = 1.0f;
    if (expected_output && strlen(expected_output) > 0) {
        error_signal = melvin_out_port_compare_output(output, output_size,
                                                       (const uint8_t*)expected_output, strlen(expected_output));
        if (apply_feedback) {
            melvin_m_feedback_error(mfile, error_signal);
        }
    }
    
    // Output results (for parsing by script)
    printf("OUTPUT: ");
    for (size_t i = 0; i < output_size; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("\n");
    
    printf("ERROR_SIGNAL: %.3f\n", error_signal);
    printf("NODES: %zu\n", melvin_m_get_node_count(mfile));
    printf("EDGES: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Count hierarchies (abstraction_level > 0)
    size_t hierarchy_count = 0;
    // Note: We can't easily count hierarchies from the public API,
    // so we'll use 0 for now (could add API later if needed)
    printf("HIERARCHIES: %zu\n", hierarchy_count);
    
    // Clear output buffer
    melvin_m_universal_output_clear(mfile);
    
    // Cleanup
    melvin_m_close(mfile);
    
    return 0;
}

