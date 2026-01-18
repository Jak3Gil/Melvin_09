/* Test multiple associations in a single brain file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

typedef struct {
    const char *input;
    const char *expected_output;
    const char *pattern;
} Association;

int main() {
    printf("=== Multiple Associations Test ===\n\n");
    
    const char *test_file = "test_multi_assoc.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    // Define multiple associations to learn
    Association associations[] = {
        {"cat", " meow", "cat meow"},
        {"dog", " bark", "dog bark"},
        {"bird", " tweet", "bird tweet"},
        {"cow", " moo", "cow moo"},
        {"duck", " quack", "duck quack"},
        {"hello", " world", "hello world"},
        {"good", " morning", "good morning"},
        {"red", " apple", "red apple"},
    };
    
    size_t num_associations = sizeof(associations) / sizeof(associations[0]);
    
    printf("Training %zu associations (5 iterations each)...\n\n", num_associations);
    
    // Train all patterns multiple times
    for (size_t i = 0; i < num_associations; i++) {
        printf("Training: '%s' -> '%s'\n", associations[i].input, associations[i].expected_output);
        for (int iter = 0; iter < 5; iter++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)associations[i].pattern, strlen(associations[i].pattern));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    printf("\n=== Testing All Associations ===\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    
    // Test each association
    for (size_t i = 0; i < num_associations; i++) {
        total_tests++;
        
        printf("Test %zu: '%s' -> ", i + 1, associations[i].input);
        
        // Clear and write input
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)associations[i].input, strlen(associations[i].input));
        melvin_m_process_input(mfile);
        
        // Read output
        size_t output_size = melvin_m_universal_output_size(mfile);
        uint8_t *output = NULL;
        size_t actual_output_size = 0;
        
        if (output_size > 0) {
            output = malloc(output_size + 1);
            if (output) {
                size_t read = melvin_m_universal_output_read(mfile, output, output_size);
                actual_output_size = read;
                output[actual_output_size] = '\0';
            }
        }
        
        // Check result
        int success = 0;
        if (output && actual_output_size > 0) {
            // Print output
            printf("'");
            for (size_t j = 0; j < actual_output_size && j < 50; j++) {
                if (output[j] >= 32 && output[j] < 127) {
                    printf("%c", output[j]);
                } else {
                    printf(".");
                }
            }
            printf("'");
            
            // Check if matches expected (allowing partial match)
            const char *expected = associations[i].expected_output;
            size_t expected_len = strlen(expected);
            
            if (actual_output_size >= expected_len) {
                int match = 1;
                for (size_t j = 0; j < expected_len; j++) {
                    if (output[j] != expected[j]) {
                        match = 0;
                        break;
                    }
                }
                if (match) {
                    success = 1;
                    passed_tests++;
                    printf(" ✅ PASS");
                } else {
                    printf(" ❌ FAIL (expected '%s')", expected);
                }
            } else {
                printf(" ❌ FAIL (too short, expected '%s')", expected);
            }
        } else {
            printf("(empty) ❌ FAIL (no output)");
        }
        
        printf("\n");
        
        if (output) free(output);
    }
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed_tests, total_tests, 
           total_tests > 0 ? (100.0f * passed_tests / total_tests) : 0.0f);
    
    if (passed_tests == total_tests) {
        printf("\n✅ ALL TESTS PASSED!\n");
    } else {
        printf("\n❌ Some tests failed\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return (passed_tests == total_tests) ? 0 : 1;
}
