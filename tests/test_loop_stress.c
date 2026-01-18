/*
 * Loop Stress Test
 * Tests patterns that are most likely to create loops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int detect_loop_detailed(const uint8_t *output, size_t len) {
    if (!output || len < 2) return 0;
    
    int loop_found = 0;
    
    // Check for 2-character repeating patterns (safely)
    if (len >= 4) {
        for (size_t i = 0; i < len - 3 && i < len; i++) {
            if (output[i] == output[i+2] && output[i+1] == output[i+3]) {
                // Found potential 2-char pattern
                int repeat_count = 1;
                size_t pattern_len = 2;
                for (size_t j = i + pattern_len; j < len - pattern_len && j < len; j += pattern_len) {
                    if (j + 1 < len &&
                        output[i] == output[j] && output[i+1] == output[j+1]) {
                        repeat_count++;
                    } else {
                        break;
                    }
                }
                if (repeat_count >= 3) {
                    loop_found = 1;
                    break;  // Found loop, no need to continue
                }
            }
        }
    }
    
    // Check for single character repetition (4+ times)
    if (!loop_found && len >= 4) {
        for (size_t i = 0; i < len - 3 && i < len; i++) {
            if (i + 3 < len &&
                output[i] == output[i+1] && 
                output[i] == output[i+2] && 
                output[i] == output[i+3]) {
                loop_found = 1;
                break;
            }
        }
    }
    
    return loop_found;
}

int main() {
    printf("=== Loop Stress Test ===\n");
    printf("Testing patterns most likely to create loops\n\n");
    
    const char *test_file = "test_loop_stress.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // High-risk patterns for loops
    struct {
        const char *training;
        const char *query;
        const char *description;
    } stress_tests[] = {
        {
            .training = "lolololol",
            .query = "lo",
            .description = "Direct loop pattern 'lo' repeated"
        },
        {
            .training = "hello hello hello",
            .query = "hello",
            .description = "Word repeated 3 times"
        },
        {
            .training = "abcabcabc",
            .query = "abc",
            .description = "Pattern 'abc' repeated"
        },
        {
            .training = "test test test test",
            .query = "test",
            .description = "Word repeated 4 times"
        },
        {
            .training = "aa bb aa bb",
            .query = "aa",
            .description = "Alternating pattern"
        },
        {
            .training = "repeat repeat",
            .query = "repeat",
            .description = "Word repeated 2 times"
        },
        {
            .training = "abababab",
            .query = "ab",
            .description = "Two-char pattern repeated"
        },
        {
            .training = "hello world hello world",
            .query = "hello",
            .description = "Phrase repeated"
        },
        {
            .training = "xyz xyz xyz",
            .query = "xyz",
            .description = "Three-char pattern repeated"
        },
        {
            .training = "a a a a a",
            .query = "a",
            .description = "Single char repeated 5 times"
        }
    };
    
    int num_tests = sizeof(stress_tests) / sizeof(stress_tests[0]);
    int passed = 0;
    int total = 0;
    
    printf("Running %d stress tests...\n\n", num_tests);
    
    for (int test_idx = 0; test_idx < num_tests; test_idx++) {
        // Reset for each test
        melvin_m_close(mfile);
        unlink(test_file);
        mfile = melvin_m_create(test_file);
        if (!mfile) {
            fprintf(stderr, "ERROR: Failed to recreate brain file\n");
            return 1;
        }
        
        printf("--- Test %d: %s ---\n", test_idx + 1, stress_tests[test_idx].description);
        printf("Training: '%s'\n", stress_tests[test_idx].training);
        printf("Query: '%s'\n", stress_tests[test_idx].query);
        
        // Train heavily (20 times)
        for (int i = 0; i < 20; i++) {
            melvin_m_universal_input_write(mfile, 
                (const uint8_t*)stress_tests[test_idx].training, 
                strlen(stress_tests[test_idx].training));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        // Query multiple times to check consistency
        int loop_detected = 0;
        int queries_with_loops = 0;
        int total_queries = 5;
        
        for (int q = 0; q < total_queries; q++) {
            melvin_m_universal_input_write(mfile, 
                (const uint8_t*)stress_tests[test_idx].query, 
                strlen(stress_tests[test_idx].query));
            melvin_m_process_input(mfile);
            
            size_t output_size = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_size + 1);
            if (!output) {
                fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
                continue;
            }
            
            size_t read = melvin_m_universal_output_read(mfile, output, output_size);
            if (read > output_size) read = output_size;  // Safety check
            if (read > 0 && read <= output_size) {
                output[read] = '\0';
            } else {
                read = 0;
                output[0] = '\0';
            }
            
            if (q == 0) {
                printf("Output (query 1): '%.*s' (%zu bytes)\n", (int)read, output, read);
            }
            
            if (read > 0 && detect_loop_detailed(output, read)) {
                loop_detected = 1;
                queries_with_loops++;
                if (q == 0) {
                    printf("❌ LOOP DETECTED in output!\n");
                }
            }
            
            free(output);
            melvin_m_universal_output_clear(mfile);
        }
        
        if (loop_detected) {
            printf("❌ FAILED: Loops detected in %d/%d queries\n", queries_with_loops, total_queries);
        } else {
            printf("✅ PASSED: No loops detected in any query\n");
            passed++;
        }
        total++;
        printf("\n");
    }
    
    printf("=== Final Results ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    printf("Success rate: %.1f%%\n", (passed * 100.0f) / total);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    if (passed == total) {
        printf("\n✅ ALL STRESS TESTS PASSED - No loops even in high-risk patterns!\n");
        return 0;
    } else {
        printf("\n❌ SOME STRESS TESTS FAILED - Loops detected in loop-prone patterns\n");
        return 1;
    }
}
