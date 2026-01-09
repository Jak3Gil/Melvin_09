#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"
#include "melvin_in_port.h"

// Calculate error rate: how many output bytes don't match expected
float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    size_t expected_len = strlen(expected);
    size_t errors = 0;
    size_t max_len = (actual_len > expected_len) ? actual_len : expected_len;
    
    // Count mismatches
    for (size_t i = 0; i < max_len; i++) {
        if (i >= actual_len || i >= expected_len) {
            errors++; // Length mismatch
        } else if (actual[i] != (uint8_t)expected[i]) {
            errors++; // Character mismatch
        }
    }
    
    return (max_len > 0) ? ((float)errors / (float)max_len) : 1.0f;
}

int main() {
    printf("=== Error Rate Over Iterations (No Learning Rate) ===\n\n");
    
    const char *test_file = "test_simple_error.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    const char *input_prefix = "hello ";
    const char *expected_output = "world";
    
    printf("Training pattern: '%s'\n", pattern);
    printf("Test input: '%s'\n", input_prefix);
    printf("Expected output: '%s'\n\n", expected_output);
    
    printf("Iteration | Nodes | Edges | Error Rate | Output\n");
    printf("----------|-------|-------|------------|--------\n");
    
    for (int iter = 1; iter <= 200; iter++) {
        // Train using PORT PIPELINE (port 0 = standard input)
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 20 iterations
        if (iter % 20 == 0) {
            // Input prefix using PORT PIPELINE
            melvin_in_port_process_device(mfile, 0, (uint8_t*)input_prefix, strlen(input_prefix));
            
            // Get output
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            melvin_m_universal_output_read(mfile, output, output_len);
            output[output_len] = '\0';
            
            // Calculate error
            float error_rate = calculate_error_rate(expected_output, output, output_len);
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            printf("%9d | %5zu | %5zu | %9.1f%% | %.*s\n", 
                   iter, nodes, edges, error_rate * 100.0f, 
                   (int)(output_len < 20 ? output_len : 20), output);
            
            free(output);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("\n=== Test Complete ===\n");
    printf("System uses direct Hebbian learning (no learning rate)\n");
    printf("Decay and competition provide natural bounds\n");
    
    return 0;
}

