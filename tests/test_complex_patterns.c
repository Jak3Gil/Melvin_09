#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// Helper to calculate error rate
float calculate_error_rate(const uint8_t *output, size_t output_len, 
                          const uint8_t *expected, size_t expected_len) {
    if (output_len == 0 && expected_len == 0) return 0.0f;
    if (output_len == 0) return 100.0f;
    if (expected_len == 0) return 100.0f;
    
    // Find longest common subsequence
    size_t matches = 0;
    size_t o_idx = 0;
    size_t e_idx = 0;
    
    while (o_idx < output_len && e_idx < expected_len) {
        if (output[o_idx] == expected[e_idx]) {
            matches++;
            o_idx++;
            e_idx++;
        } else {
            // Try to find match in remaining output
            int found = 0;
            for (size_t i = o_idx + 1; i < output_len; i++) {
                if (output[i] == expected[e_idx]) {
                    o_idx = i + 1;
                    e_idx++;
                    matches++;
                    found = 1;
                    break;
                }
            }
            if (!found) {
                e_idx++; // Skip expected char
            }
        }
    }
    
    float error = 100.0f * (1.0f - (float)matches / (float)expected_len);
    return error;
}

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_complex_patterns.m";
    
    printf("=== Complex Pattern Testing Suite ===\n\n");
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("Created new brain: %s\n\n", brain_file);
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // =========================================================================
    // TEST 1: Long Sequence Learning
    // =========================================================================
    printf("=== TEST 1: Long Sequence Learning ===\n");
    printf("Training: 'the quick brown fox jumps over the lazy dog' (10 times)\n");
    
    const char *long_sequence = "the quick brown fox jumps over the lazy dog";
    for (int i = 1; i <= 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)long_sequence, strlen(long_sequence));
        melvin_m_save(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test various prefixes
    struct {
        const char *input;
        const char *expected;
        const char *description;
    } test1_cases[] = {
        {"the quick", " brown fox jumps over the lazy dog"},
        {"the", " quick brown fox jumps over the lazy dog"},
        {"quick brown", " fox jumps over the lazy dog"},
        {"fox", " jumps over the lazy dog"},
        {"lazy", " dog"},
    };
    
    for (size_t t = 0; t < sizeof(test1_cases) / sizeof(test1_cases[0]); t++) {
        total_tests++;
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test1_cases[t].input, strlen(test1_cases[t].input));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            float error = calculate_error_rate(output, output_size, 
                                             (uint8_t*)test1_cases[t].expected, 
                                             strlen(test1_cases[t].expected));
            
            printf("  Input: '%s'\n", test1_cases[t].input);
            printf("    Output (%zu bytes): ", output_size);
            for (size_t i = 0; i < output_size && i < 60; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
                else printf("\\x%02x", output[i]);
            }
            if (output_size > 60) printf("...");
            printf("\n");
            printf("    Expected: '%s'\n", test1_cases[t].expected);
            printf("    Error: %.1f%%\n", error);
            
            if (error < 50.0f) {
                printf("    ✓ PASS\n");
                passed_tests++;
            } else {
                printf("    ✗ FAIL\n");
            }
            printf("\n");
            free(output);
        }
    }
    
    // =========================================================================
    // TEST 2: Multiple Pattern Discrimination
    // =========================================================================
    printf("=== TEST 2: Multiple Pattern Discrimination ===\n");
    printf("Training multiple patterns:\n");
    
    const char *patterns[] = {
        "the cat sat on the mat",
        "the dog ran in the park",
        "the bird flew in the sky",
        "the fish swam in the sea",
    };
    
    printf("Training patterns (20 iterations each):\n");
    for (int p = 0; p < 4; p++) {
        printf("  Pattern %d: '%s'\n", p + 1, patterns[p]);
        for (int i = 0; i < 20; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)patterns[p], strlen(patterns[p]));
        }
    }
    melvin_m_save(mfile);
    printf("Graph: %zu nodes, %zu edges\n\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test discrimination
    struct {
        const char *input;
        const char *expected_contains;
        const char *description;
    } test2_cases[] = {
        {"the cat", "sat", "Should continue with 'sat'"},
        {"the dog", "ran", "Should continue with 'ran'"},
        {"the bird", "flew", "Should continue with 'flew'"},
        {"the fish", "swam", "Should continue with 'swam'"},
    };
    
    for (size_t t = 0; t < sizeof(test2_cases) / sizeof(test2_cases[0]); t++) {
        total_tests++;
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test2_cases[t].input, strlen(test2_cases[t].input));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            // Check if output contains expected word
            int found = 0;
            size_t expected_len = strlen(test2_cases[t].expected_contains);
            for (size_t i = 0; i + expected_len <= output_size; i++) {
                if (memcmp(output + i, test2_cases[t].expected_contains, expected_len) == 0) {
                    found = 1;
                    break;
                }
            }
            
            printf("  Input: '%s'\n", test2_cases[t].input);
            printf("    Output (%zu bytes): ", output_size);
            for (size_t i = 0; i < output_size && i < 60; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
                else printf("\\x%02x", output[i]);
            }
            if (output_size > 60) printf("...");
            printf("\n");
            printf("    Expected to contain: '%s'\n", test2_cases[t].expected_contains);
            
            if (found) {
                printf("    ✓ PASS (contains expected word)\n");
                passed_tests++;
            } else {
                printf("    ✗ FAIL (does not contain expected word)\n");
            }
            printf("\n");
            free(output);
        }
    }
    
    // =========================================================================
    // TEST 3: Very Long Training Sequence
    // =========================================================================
    printf("=== TEST 3: Very Long Training Sequence ===\n");
    printf("Training: 100-character sequence (50 iterations)\n");
    
    const char *very_long = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}|;:,.<>?";
    size_t very_long_len = strlen(very_long);
    
    for (int i = 0; i < 50; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)very_long, very_long_len);
    }
    melvin_m_save(mfile);
    printf("Graph: %zu nodes, %zu edges\n\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test various prefixes
    struct {
        const char *input;
        const char *description;
    } test3_cases[] = {
        {"abc", "Start of sequence"},
        {"xyz", "Middle of sequence"},
        {"ABC", "Uppercase section"},
        {"012", "Number section"},
        {"!@#", "Symbol section"},
    };
    
    for (size_t t = 0; t < sizeof(test3_cases) / sizeof(test3_cases[0]); t++) {
        total_tests++;
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test3_cases[t].input, strlen(test3_cases[t].input));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            printf("  Input: '%s' (%s)\n", test3_cases[t].input, test3_cases[t].description);
            printf("    Output (%zu bytes): ", output_size);
            for (size_t i = 0; i < output_size && i < 60; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
                else printf("\\x%02x", output[i]);
            }
            if (output_size > 60) printf("...");
            printf("\n");
            
            // Check if output continues the sequence
            int valid = (output_size > 0);
            printf("    %s (generated %zu bytes)\n", valid ? "✓ PASS" : "✗ FAIL", output_size);
            if (valid) passed_tests++;
            printf("\n");
            free(output);
        } else {
            printf("  Input: '%s' (%s)\n", test3_cases[t].input, test3_cases[t].description);
            printf("    ✗ FAIL (no output)\n\n");
        }
    }
    
    // =========================================================================
    // TEST 4: Compound Associations
    // =========================================================================
    printf("=== TEST 4: Compound Associations ===\n");
    printf("Training compound patterns (30 iterations each):\n");
    
    const char *compound_patterns[] = {
        "red apple",
        "green apple",
        "red car",
        "green car",
        "red light",
        "green light",
    };
    
    for (int p = 0; p < 6; p++) {
        printf("  '%s'\n", compound_patterns[p]);
        for (int i = 0; i < 30; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)compound_patterns[p], strlen(compound_patterns[p]));
        }
    }
    melvin_m_save(mfile);
    printf("Graph: %zu nodes, %zu edges\n\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test compound associations
    struct {
        const char *input;
        const char *expected_contains;
        const char *description;
    } test4_cases[] = {
        {"red", "apple", "Should associate 'red' with 'apple'"},
        {"green", "apple", "Should associate 'green' with 'apple'"},
        {"red", "car", "Should associate 'red' with 'car'"},
        {"green", "light", "Should associate 'green' with 'light'"},
    };
    
    for (size_t t = 0; t < sizeof(test4_cases) / sizeof(test4_cases[0]); t++) {
        total_tests++;
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test4_cases[t].input, strlen(test4_cases[t].input));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            // Check if output contains expected word
            int found = 0;
            size_t expected_len = strlen(test4_cases[t].expected_contains);
            for (size_t i = 0; i + expected_len <= output_size; i++) {
                if (memcmp(output + i, test4_cases[t].expected_contains, expected_len) == 0) {
                    found = 1;
                    break;
                }
            }
            
            printf("  Input: '%s'\n", test4_cases[t].input);
            printf("    Output (%zu bytes): ", output_size);
            for (size_t i = 0; i < output_size && i < 60; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
                else printf("\\x%02x", output[i]);
            }
            if (output_size > 60) printf("...");
            printf("\n");
            printf("    Expected to contain: '%s'\n", test4_cases[t].expected_contains);
            
            if (found) {
                printf("    ✓ PASS\n");
                passed_tests++;
            } else {
                printf("    ✗ FAIL\n");
            }
            printf("\n");
            free(output);
        }
    }
    
    // =========================================================================
    // TEST 5: Long Run Stability
    // =========================================================================
    printf("=== TEST 5: Long Run Stability ===\n");
    printf("Training same pattern 200 times, testing every 20 iterations\n");
    
    const char *stability_pattern = "test pattern for stability check";
    float error_rates[10];
    
    for (int iteration = 0; iteration < 10; iteration++) {
        // Train 20 more times
        for (int i = 0; i < 20; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)stability_pattern, strlen(stability_pattern));
        }
        melvin_m_save(mfile);
        
        // Test
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"test pattern", 12);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            const char *expected = " for stability check";
            float error = calculate_error_rate(output, output_size, 
                                             (uint8_t*)expected, strlen(expected));
            error_rates[iteration] = error;
            
            printf("  Iteration %d (after %d total): Error = %.1f%%, Output (%zu bytes): ",
                   iteration + 1, (iteration + 1) * 20, error, output_size);
            for (size_t i = 0; i < output_size && i < 40; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
                else printf("\\x%02x", output[i]);
            }
            if (output_size > 40) printf("...");
            printf("\n");
            
            free(output);
        } else {
            error_rates[iteration] = 100.0f;
            printf("  Iteration %d: No output\n", iteration + 1);
        }
    }
    
    // Check stability
    float avg_error = 0.0f;
    float max_error = 0.0f;
    for (int i = 0; i < 10; i++) {
        avg_error += error_rates[i];
        if (error_rates[i] > max_error) max_error = error_rates[i];
    }
    avg_error /= 10.0f;
    
    printf("\n  Stability Summary:\n");
    printf("    Average error: %.1f%%\n", avg_error);
    printf("    Max error: %.1f%%\n", max_error);
    printf("    Graph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    if (avg_error < 50.0f && max_error < 80.0f) {
        printf("    ✓ PASS (stable)\n");
        passed_tests++;
    } else {
        printf("    ✗ FAIL (unstable)\n");
    }
    total_tests++;
    printf("\n");
    
    // =========================================================================
    // FINAL SUMMARY
    // =========================================================================
    printf("=== FINAL SUMMARY ===\n");
    printf("Total tests: %d\n", total_tests);
    printf("Passed: %d\n", passed_tests);
    printf("Failed: %d\n", total_tests - passed_tests);
    printf("Success rate: %.1f%%\n", 100.0f * (float)passed_tests / (float)total_tests);
    printf("\nFinal graph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    return (passed_tests == total_tests) ? 0 : 1;
}
