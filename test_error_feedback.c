/*
 * Test Error Feedback System
 * 
 * Tests the dual learning system: frequency-based (Hebbian) + error-based (feedback)
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m>\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    
    printf("=== Error Feedback System Test ===\n\n");
    
    // Load or create brain
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to load brain file\n");
        return 1;
    }
    
    printf("Initial state:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("\n");
    
    // Test 1: Train with "hello world"
    printf("Test 1: Training with 'hello world'\n");
    const char *input1 = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)input1, strlen(input1));
    melvin_m_process_input(mfile);
    
    printf("  After training:\n");
    printf("    Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("    Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Test 2: Query with "hello" and check output
    printf("\nTest 2: Query with 'hello' (expected: ' world')\n");
    const char *input2 = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)input2, strlen(input2));
    melvin_m_process_input(mfile);
    
    // Read output
    uint8_t output[256];
    size_t output_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("  Output: \"");
    for (size_t i = 0; i < output_size; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("\" (%zu bytes)\n", output_size);
    
    // Test 3: Compare output to expected and feed back error
    printf("\nTest 3: Error feedback\n");
    const char *expected = " world";
    float error_signal = melvin_out_port_compare_output(output, output_size,
                                                         (const uint8_t*)expected, strlen(expected));
    printf("  Expected: \"%s\"\n", expected);
    printf("  Error signal: %.3f (1.0 = perfect, 0.0 = wrong)\n", error_signal);
    
    // Feed error back
    melvin_m_feedback_error(mfile, error_signal);
    printf("  Error feedback applied\n");
    printf("  Nodes: %zu, Edges: %zu\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test 4: Query again and see if output improved
    printf("\nTest 4: Query again (should improve with error feedback)\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)input2, strlen(input2));
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
    printf("  Output: \"");
    for (size_t i = 0; i < output_size; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("\" (%zu bytes)\n", output_size);
    
    float error_signal2 = melvin_out_port_compare_output(output, output_size,
                                                          (const uint8_t*)expected, strlen(expected));
    printf("  Error signal: %.3f\n", error_signal2);
    printf("  Improvement: %+.3f\n", error_signal2 - error_signal);
    
    // Test 5: Use convenience function
    printf("\nTest 5: Using convenience function (melvin_out_port_process_with_feedback)\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)input2, strlen(input2));
    melvin_m_process_input(mfile);
    
    melvin_out_port_process_with_feedback(mfile, 
                                          (const uint8_t*)expected, strlen(expected));
    printf("  Processed with feedback (output routed to stdout)\n");
    
    // Cleanup
    melvin_m_close(mfile);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}

