/* test_error_rate.c
 * Measures error rate improvement: Does the system learn and get better?
 * Tests prediction accuracy over repeated learning
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Calculate error rate: how many bytes are wrong?
float calculate_error_rate(const uint8_t *output, size_t output_len, 
                           const char *expected, size_t expected_len) {
    if (output_len == 0 && expected_len == 0) return 0.0f;
    if (output_len == 0 || expected_len == 0) return 1.0f;  // 100% error
    
    size_t errors = 0;
    size_t check_len = (output_len < expected_len) ? output_len : expected_len;
    
    // Count mismatches
    for (size_t i = 0; i < check_len; i++) {
        if (output[i] != (uint8_t)expected[i]) {
            errors++;
        }
    }
    
    // Add penalty for length mismatch
    if (output_len < expected_len) {
        errors += (expected_len - output_len);
    }
    
    return (float)errors / (float)expected_len;
}

int main() {
    printf("=== Error Rate Improvement Test ===\n");
    printf("Measuring: Does error rate DECREASE with learning?\n\n");
    
    const char *path = "test_error_rate.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    size_t pattern_len = strlen(pattern);
    
    printf("Learning pattern: '%s' (%zu bytes)\n", pattern, pattern_len);
    printf("Expected output: Full pattern completion\n\n");
    
    printf("Iteration | Nodes | Edges | Output Len | Error Rate | Status\n");
    printf("----------|-------|-------|------------|------------|--------\n");
    
    float initial_error = -1.0f;
    float best_error = 1.0f;
    int improvements = 0;
    
    for (int iter = 1; iter <= 200; iter++) {
        // Learn the pattern
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, pattern_len);
        melvin_m_process_input(mfile);
        
        // Test every 20 iterations
        if (iter % 20 == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            // Get output
            size_t output_size = melvin_m_universal_output_size(mfile);
            uint8_t output[1024] = {0};
            size_t read_size = 0;
            
            if (output_size > 0) {
                read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
            }
            
            // Calculate error rate
            float error_rate = calculate_error_rate(output, read_size, pattern, pattern_len);
            
            // Track improvements
            if (initial_error < 0.0f) {
                initial_error = error_rate;
            }
            
            const char *status = "";
            if (error_rate < best_error) {
                best_error = error_rate;
                improvements++;
                status = "✓ IMPROVED";
            } else if (error_rate == best_error) {
                status = "= Same";
            } else {
                status = "↑ Worse";
            }
            
            printf("%9d | %5zu | %5zu | %10zu | %9.1f%% | %s\n",
                   iter, nodes, edges, read_size, error_rate * 100.0f, status);
        }
    }
    
    printf("\n=== Results ===\n");
    printf("Initial error rate: %.1f%%\n", initial_error * 100.0f);
    printf("Best error rate:    %.1f%%\n", best_error * 100.0f);
    printf("Improvements:       %d times\n", improvements);
    
    float improvement = initial_error - best_error;
    printf("\nError reduction:    %.1f%%\n", improvement * 100.0f);
    
    printf("\n=== Verdict ===\n");
    if (improvement > 0.2f) {
        printf("✅ PASS: Error rate improved significantly (%.1f%% reduction)\n", improvement * 100.0f);
        printf("   System is LEARNING and getting BETTER\n");
    } else if (improvement > 0.05f) {
        printf("⚠️  PARTIAL: Some improvement (%.1f%% reduction)\n", improvement * 100.0f);
        printf("   System is learning but slowly\n");
    } else {
        printf("❌ FAIL: No significant improvement (%.1f%% reduction)\n", improvement * 100.0f);
        printf("   System is NOT learning effectively\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

