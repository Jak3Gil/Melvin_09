/*
 * Error Rate Test Over Iterations
 * Tests how error rate changes as the system learns with error feedback
 */

#include "src/melvin.h"
#include "src/melvin_in_port.h"
#include "src/melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Simple Levenshtein distance for error calculation
static float levenshtein_distance(const uint8_t *a, size_t len_a, 
                                  const uint8_t *b, size_t len_b) {
    if (len_a == 0) return (float)len_b;
    if (len_b == 0) return (float)len_a;
    
    // Simple character-by-character comparison
    size_t matches = 0;
    size_t min_len = (len_a < len_b) ? len_a : len_b;
    
    for (size_t i = 0; i < min_len; i++) {
        if (a[i] == b[i]) matches++;
    }
    
    size_t max_len = (len_a > len_b) ? len_a : len_b;
    float match_ratio = (max_len > 0) ? ((float)matches / (float)max_len) : 0.0f;
    return 1.0f - match_ratio;  // Return error (0.0 = perfect, 1.0 = completely wrong)
}

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "error_test.m";
    int iterations = (argc > 2) ? atoi(argv[2]) : 50;
    
    // Test patterns: (input, expected_output)
    struct {
        const char *input;
        const char *expected;
    } patterns[] = {
        {"hello", "lo"},
        {"world", "ld"},
        {"test", "t"},
        {"learn", "rn"},
        {"quick", "ck"},
    };
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("========================================\n");
    printf("Error Rate Test Over Iterations\n");
    printf("========================================\n");
    printf("Brain file: %s\n", brain_file);
    printf("Iterations per pattern: %d\n", iterations);
    printf("Total patterns: %zu\n", num_patterns);
    printf("\n");
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    // Test each pattern
    for (size_t p = 0; p < num_patterns; p++) {
        const char *input = patterns[p].input;
        const char *expected = patterns[p].expected;
        size_t input_len = strlen(input);
        size_t expected_len = strlen(expected);
        
        printf("========================================\n");
        printf("Pattern: '%s' -> Expected: '%s'\n", input, expected);
        printf("========================================\n");
        printf("Iter | Output | Expected | Error %% | Nodes | Edges | Avg Error %%\n");
        printf("-----|--------|----------|---------|-------|-------|------------\n");
        
        float total_error = 0.0f;
        
        for (int iter = 1; iter <= iterations; iter++) {
            // Clear output
            melvin_m_universal_output_clear(mfile);
            
            // Process input
            melvin_m_universal_input_write(mfile, (const uint8_t *)input, input_len);
            melvin_m_set_last_input_port_id(mfile, 0x01);  // Text port
            melvin_m_process_input(mfile);
            
            // Read output
            uint8_t output[256];
            size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
            
            // Calculate error
            float error_rate = 0.0f;
            if (output_len > 0 && expected_len > 0) {
                float distance = levenshtein_distance(output, output_len, 
                                                     (const uint8_t *)expected, expected_len);
                error_rate = distance * 100.0f;
            } else if (output_len == 0 && expected_len > 0) {
                error_rate = 100.0f;  // No output = 100% error
            } else if (output_len > 0 && expected_len == 0) {
                error_rate = 100.0f;  // Unexpected output = 100% error
            }
            
            // Calculate error signal (1.0 = perfect, 0.0 = completely wrong)
            float error_signal = 1.0f - (error_rate / 100.0f);
            
            // Apply error feedback
            melvin_m_feedback_error(mfile, error_signal);
            
            // Accumulate for average
            total_error += error_rate;
            float avg_error = total_error / (float)iter;
            
            // Get graph stats
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            // Format output string
            char output_str[32];
            if (output_len > 0) {
                size_t copy_len = (output_len < sizeof(output_str) - 1) ? output_len : sizeof(output_str) - 1;
                memcpy(output_str, output, copy_len);
                output_str[copy_len] = '\0';
            } else {
                strcpy(output_str, "(none)");
            }
            
            // Print every 5th iteration and last
            if (iter % 5 == 0 || iter == iterations) {
                printf("%4d | %-6s | %-8s | %6.2f%% | %5zu | %5zu | %10.2f%%\n",
                       iter, output_str, expected, error_rate, nodes, edges, avg_error);
            }
        }
        
        // Final stats
        float final_avg_error = total_error / (float)iterations;
        size_t final_nodes = melvin_m_get_node_count(mfile);
        size_t final_edges = melvin_m_get_edge_count(mfile);
        
        printf("\n");
        printf("Final Stats:\n");
        printf("  Average Error Rate: %.2f%%\n", final_avg_error);
        printf("  Final Nodes: %zu\n", final_nodes);
        printf("  Final Edges: %zu\n", final_edges);
        printf("\n");
    }
    
    // Save and close
    melvin_m_save(mfile);
    melvin_m_close(mfile);
    
    printf("========================================\n");
    printf("Test Complete\n");
    printf("========================================\n");
    
    return 0;
}

