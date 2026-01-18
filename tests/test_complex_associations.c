/* Test complex associations with overlapping patterns */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Complex Associations Test ===\n\n");
    
    const char *test_file = "test_complex_assoc.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    // Complex patterns with overlapping words
    const char *patterns[] = {
        "the cat sat",
        "the dog ran",
        "the bird flew",
        "cat meow",
        "dog bark",
        "bird tweet",
        "red cat",
        "blue dog",
        "green bird",
        "hello world",
        "good morning",
        "good night",
    };
    
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Training %zu complex patterns (10 iterations each)...\n\n", num_patterns);
    
    // Train all patterns
    for (size_t i = 0; i < num_patterns; i++) {
        printf("Training %zu/%zu: '%s'\n", i + 1, num_patterns, patterns[i]);
        for (int iter = 0; iter < 10; iter++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Edge-to-Node Ratio: %.2f:1\n", 
           melvin_m_get_node_count(mfile) > 0 ? 
           (float)melvin_m_get_edge_count(mfile) / melvin_m_get_node_count(mfile) : 0.0f);
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    printf("\n=== Testing Continuations ===\n\n");
    
    // Test various continuations
    typedef struct {
        const char *input;
        const char *expected_contains;  // Output should contain this
    } TestCase;
    
    TestCase tests[] = {
        {"the cat", " sat"},
        {"the dog", " ran"},
        {"the bird", " flew"},
        {"cat", " meow"},
        {"dog", " bark"},
        {"bird", " tweet"},
        {"red", " cat"},
        {"blue", " dog"},
        {"green", " bird"},
        {"hello", " world"},
        {"good", " morning"},  // or " night"
    };
    
    size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    size_t passed = 0;
    
    for (size_t i = 0; i < num_tests; i++) {
        printf("Test %zu: '%s' -> ", i + 1, tests[i].input);
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)tests[i].input, strlen(tests[i].input));
        melvin_m_process_input(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        
        if (output_size > 0) {
            uint8_t *output = malloc(output_size + 1);
            if (output) {
                size_t read = melvin_m_universal_output_read(mfile, output, output_size);
                output[read] = '\0';
                
                // Print output
                printf("'");
                for (size_t j = 0; j < read && j < 50; j++) {
                    if (output[j] >= 32 && output[j] < 127) {
                        printf("%c", output[j]);
                    } else {
                        printf(".");
                    }
                }
                printf("'");
                
                // Check if contains expected
                int found = 0;
                for (size_t j = 0; j < read; j++) {
                    if (output[j] == tests[i].expected_contains[0]) {
                        int match = 1;
                        for (size_t k = 0; k < strlen(tests[i].expected_contains) && (j + k) < read; k++) {
                            if (output[j + k] != tests[i].expected_contains[k]) {
                                match = 0;
                                break;
                            }
                        }
                        if (match) {
                            found = 1;
                            break;
                        }
                    }
                }
                
                if (found) {
                    printf(" ✅");
                    passed++;
                } else {
                    printf(" ❌ (expected contains '%s')", tests[i].expected_contains);
                }
                
                free(output);
            } else {
                printf("(alloc failed) ❌");
            }
        } else {
            printf("(empty) ❌");
        }
        
        printf("\n");
    }
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%zu (%.1f%%)\n", passed, num_tests,
           num_tests > 0 ? (100.0f * passed / num_tests) : 0.0f);
    
    if (passed == num_tests) {
        printf("\n✅ ALL TESTS PASSED!\n");
    } else {
        printf("\n⚠️  Some tests failed (may be acceptable for complex patterns)\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return (passed == num_tests) ? 0 : 1;
}
