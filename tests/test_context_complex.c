/*
 * Complex Context Matching Test
 * Tests the system with challenging patterns that could easily create loops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../src/melvin.h"

// Calculate Levenshtein distance (simplified)
int levenshtein_distance(const char *s1, const char *s2) {
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    
    if (len1 == 0) return (int)len2;
    if (len2 == 0) return (int)len1;
    
    int *d = malloc((len1 + 1) * (len2 + 1) * sizeof(int));
    if (!d) return -1;
    
    for (size_t i = 0; i <= len1; i++) d[i * (len2 + 1)] = (int)i;
    for (size_t j = 0; j <= len2; j++) d[j] = (int)j;
    
    for (size_t i = 1; i <= len1; i++) {
        for (size_t j = 1; j <= len2; j++) {
            int cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            int a = d[(i-1) * (len2 + 1) + j] + 1;
            int b = d[i * (len2 + 1) + (j-1)] + 1;
            int c = d[(i-1) * (len2 + 1) + (j-1)] + cost;
            d[i * (len2 + 1) + j] = (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
        }
    }
    
    int result = d[len1 * (len2 + 1) + len2];
    free(d);
    return result;
}

// Check for loops in output
int detect_loop(const uint8_t *output, size_t len) {
    if (len < 4) return 0;
    
    // Check for 2+ character repeating patterns
    for (size_t i = 0; i < len - 3; i++) {
        if (output[i] == output[i+2] && output[i+1] == output[i+3]) {
            // Found 2-char repeat, check if it continues
            size_t pattern_len = 2;
            int repeat_count = 1;
            for (size_t j = i + pattern_len; j < len - pattern_len; j += pattern_len) {
                if (j + pattern_len <= len &&
                    output[i] == output[j] && output[i+1] == output[j+1]) {
                    repeat_count++;
                } else {
                    break;
                }
            }
            if (repeat_count >= 3) {
                return 1;  // Loop detected
            }
        }
    }
    
    // Check for single character repetition (4+ times)
    for (size_t i = 0; i < len - 3; i++) {
        if (output[i] == output[i+1] && 
            output[i] == output[i+2] && 
            output[i] == output[i+3]) {
            return 1;  // Single char loop
        }
    }
    
    return 0;
}

typedef struct {
    const char *training;
    const char *query;
    const char *expected;
    const char *description;
} TestCase;

int run_test_case(MelvinMFile *mfile, TestCase *test, int iteration) {
    printf("\n--- Test: %s (iteration %d) ---\n", test->description, iteration);
    printf("Training: '%s'\n", test->training);
    printf("Query: '%s'\n", test->query);
    printf("Expected: '%s'\n", test->expected);
    
    // Train multiple times to strengthen patterns
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)test->training, strlen(test->training));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Query
    melvin_m_universal_input_write(mfile, (const uint8_t*)test->query, strlen(test->query));
    melvin_m_process_input(mfile);
    
    // Read output
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_size + 1);
    if (!output) {
        fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
        return 0;
    }
    
    size_t read = melvin_m_universal_output_read(mfile, output, output_size);
    output[read] = '\0';
    
    printf("Output: '%.*s' (%zu bytes)\n", (int)read, output, read);
    
    // Analyze output
    int has_loop = detect_loop(output, read);
    int error_distance = -1;
    float error_rate = 1.0f;
    
    if (test->expected && read > 0) {
        error_distance = levenshtein_distance((const char*)output, test->expected);
        size_t max_len = (read > strlen(test->expected)) ? read : strlen(test->expected);
        if (max_len > 0) {
            error_rate = (float)error_distance / (float)max_len;
        }
    }
    
    printf("Has loop: %s\n", has_loop ? "YES ❌" : "NO ✅");
    if (error_distance >= 0) {
        printf("Error distance: %d\n", error_distance);
        printf("Error rate: %.2f%%\n", error_rate * 100.0f);
    }
    
    free(output);
    melvin_m_universal_output_clear(mfile);
    
    return has_loop ? 0 : 1;  // Pass if no loop
}

int main() {
    printf("=== Complex Context Matching Test ===\n");
    printf("Testing challenging patterns that could create loops\n\n");
    
    const char *test_file = "test_context_complex.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // Test cases designed to stress the system
    TestCase tests[] = {
        {
            .training = "hello world",
            .query = "hello",
            .expected = " world",
            .description = "Basic continuation"
        },
        {
            .training = "test test test",
            .query = "test",
            .expected = " test",
            .description = "Repeated word pattern"
        },
        {
            .training = "the quick brown fox",
            .query = "the quick",
            .expected = " brown",
            .description = "Multi-word continuation"
        },
        {
            .training = "abc abc abc",
            .query = "abc",
            .expected = " abc",
            .description = "Short repeated pattern"
        },
        {
            .training = "hello hello world",
            .query = "hello",
            .expected = " hello",  // Could be " hello" or " world"
            .description = "Ambiguous continuation (two options)"
        },
        {
            .training = "test testing tested",
            .query = "test",
            .expected = "ing",  // Most common continuation
            .description = "Multiple continuations"
        },
        {
            .training = "aa bb cc dd",
            .query = "aa",
            .expected = " bb",
            .description = "Two-char pattern"
        },
        {
            .training = "one two three four",
            .query = "one two",
            .expected = " three",
            .description = "Sequence continuation"
        },
        {
            .training = "repeat repeat repeat",
            .query = "repeat",
            .expected = " repeat",
            .description = "High repetition pattern"
        },
        {
            .training = "a b c d e f",
            .query = "a b c",
            .expected = " d",
            .description = "Single char sequence"
        }
    };
    
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    int passed = 0;
    int total = 0;
    
    printf("Running %d test cases...\n", num_tests);
    
    // Run each test multiple times to check consistency
    for (int test_idx = 0; test_idx < num_tests; test_idx++) {
        // Reset brain for each test to avoid interference
        melvin_m_close(mfile);
        unlink(test_file);
        mfile = melvin_m_create(test_file);
        if (!mfile) {
            fprintf(stderr, "ERROR: Failed to recreate brain file\n");
            return 1;
        }
        
        // Run test 3 times to check consistency
        int test_passed = 1;
        for (int iter = 0; iter < 3; iter++) {
            int result = run_test_case(mfile, &tests[test_idx], iter + 1);
            if (!result) {
                test_passed = 0;
            }
        }
        
        if (test_passed) {
            passed++;
            printf("✅ Test %d PASSED (no loops in any iteration)\n", test_idx + 1);
        } else {
            printf("❌ Test %d FAILED (loop detected)\n", test_idx + 1);
        }
        total++;
    }
    
    printf("\n=== Final Results ===\n");
    printf("Tests passed: %d/%d\n", passed, total);
    printf("Success rate: %.1f%%\n", (passed * 100.0f) / total);
    
    // Graph statistics
    printf("\nFinal graph stats:\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    if (passed == total) {
        printf("\n✅ ALL TESTS PASSED - No loops detected!\n");
        return 0;
    } else {
        printf("\n❌ SOME TESTS FAILED - Loops detected\n");
        return 1;
    }
}
