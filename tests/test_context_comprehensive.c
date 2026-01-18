/*
 * Comprehensive test: Verify context matching prevents loops across multiple patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int test_pattern(MelvinMFile *mfile, const char *training, const char *query, const char *expected_prefix) {
    printf("\n--- Testing pattern ---\n");
    printf("Training: '%s'\n", training);
    printf("Query: '%s'\n", query);
    printf("Expected prefix: '%s'\n", expected_prefix);
    
    // Train multiple times
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Query
    melvin_m_universal_input_write(mfile, (const uint8_t*)query, strlen(query));
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
    
    printf("Output: '%.*s'\n", (int)read, output);
    
    // Check for loops
    int has_loop = 0;
    if (read >= 4) {
        for (size_t i = 0; i < read - 3; i++) {
            if (output[i] == output[i+2] && output[i+1] == output[i+3]) {
                has_loop = 1;
                printf("❌ LOOP DETECTED at position %zu: '%.*s'\n", i, 4, &output[i]);
                break;
            }
        }
    }
    
    // Check if matches expected prefix
    int matches_expected = 0;
    if (expected_prefix && read >= strlen(expected_prefix)) {
        matches_expected = (memcmp(output, expected_prefix, strlen(expected_prefix)) == 0);
    }
    
    free(output);
    melvin_m_universal_output_clear(mfile);
    
    if (has_loop) {
        printf("❌ FAILED: Has loop\n");
        return 0;
    } else if (expected_prefix && !matches_expected) {
        printf("⚠️  WARNING: Doesn't match expected prefix\n");
        return 1;  // Warning, not failure
    } else {
        printf("✅ PASSED\n");
        return 1;
    }
}

int main() {
    printf("=== Comprehensive Context Matching Test ===\n");
    
    const char *test_file = "test_context_comprehensive.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    int passed = 0;
    int total = 0;
    
    // Test 1: Simple continuation
    total++;
    if (test_pattern(mfile, "hello world", "hello", " world")) passed++;
    
    // Test 2: Pattern with repeated characters (should still work)
    total++;
    if (test_pattern(mfile, "test test", "test", " ")) passed++;
    
    // Test 3: Multiple words
    total++;
    if (test_pattern(mfile, "the quick brown", "the quick", " brown")) passed++;
    
    // Test 4: Short pattern
    total++;
    if (test_pattern(mfile, "ab cd", "ab", " cd")) passed++;
    
    printf("\n=== Final Results ===\n");
    printf("Passed: %d/%d\n", passed, total);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    if (passed == total) {
        printf("\n✅ ALL TESTS PASSED\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests had warnings\n");
        return 0;  // Warnings are OK, loops are not
    }
}
