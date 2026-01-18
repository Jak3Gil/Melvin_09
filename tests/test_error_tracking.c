#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"
#include "melvin_in_port.h"

// Calculate error rate: how many output bytes don't match expected
float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    if (!expected || !actual) return 1.0f;
    
    size_t expected_len = strlen(expected);
    if (expected_len == 0 && actual_len == 0) return 0.0f;
    if (expected_len == 0 || actual_len == 0) return 1.0f;
    
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
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     MELVIN ERROR RATE TRACKING OVER ITERATIONS             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    const char *test_file = "error_tracking_test.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    const char *input_prefix = "hello ";
    const char *expected_output = "world";
    
    printf("Configuration:\n");
    printf("  Training pattern: '%s'\n", pattern);
    printf("  Test input: '%s'\n", input_prefix);
    printf("  Expected output: '%s'\n", expected_output);
    printf("  Total iterations: 200\n\n");
    
    printf("Iteration | Nodes | Edges | Error Rate | Output        | Trend\n");
    printf("----------|-------|-------|------------|---------------|--------\n");
    
    float prev_error = 1.0f;
    float total_error = 0.0f;
    int correct_count = 0;
    
    for (int iter = 1; iter <= 200; iter++) {
        // Train on full pattern
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 10 iterations (or every iteration for first 50)
        int test_interval = (iter <= 50) ? 1 : 10;
        if (iter % test_interval == 0 || iter <= 20) {
            // Input prefix
            melvin_in_port_process_device(mfile, 0, (uint8_t*)input_prefix, strlen(input_prefix));
            
            // Get output
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            if (output) {
                melvin_m_universal_output_read(mfile, output, output_len);
                output[output_len] = '\0';
                
                // Calculate error
                float error_rate = calculate_error_rate(expected_output, output, output_len);
                total_error += error_rate;
                
                // Check if correct
                int is_correct = (error_rate < 0.01f);
                if (is_correct) correct_count++;
                
                size_t nodes = melvin_m_get_node_count(mfile);
                size_t edges = melvin_m_get_edge_count(mfile);
                
                // Determine trend
                const char *trend = "";
                if (iter > 1) {
                    if (error_rate < prev_error - 0.05f) trend = "↓ IMPROVING";
                    else if (error_rate > prev_error + 0.05f) trend = "↑ WORSE";
                    else trend = "→ STABLE";
                }
                
                // Format output for display
                char output_display[16] = {0};
                if (output_len > 0) {
                    snprintf(output_display, sizeof(output_display), "%.*s", 
                            (int)(output_len < 14 ? output_len : 14), output);
                } else {
                    strcpy(output_display, "(empty)");
                }
                
                printf("%9d | %5zu | %5zu | %9.1f%% | %-14s | %s\n", 
                       iter, nodes, edges, error_rate * 100.0f, output_display, trend);
                
                prev_error = error_rate;
                free(output);
            }
            
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    // Summary
    float avg_error = total_error / 200.0f;
    float success_rate = ((float)correct_count / 200.0f) * 100.0f;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                        SUMMARY                              ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("  Average Error Rate: %.2f%%\n", avg_error * 100.0f);
    printf("  Success Rate: %.2f%% (perfect matches)\n", success_rate);
    printf("  Final Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Final Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Final Error Rate: %.2f%%\n", prev_error * 100.0f);
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
