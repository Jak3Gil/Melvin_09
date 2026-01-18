/*
 * Test: Context matching fix - verify output nodes are included in context
 * This should prevent loops by ensuring context matches training context tags
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Testing Context Matching Fix ===\n\n");
    
    // Create test brain
    const char *test_file = "test_context_fix.m";
    unlink(test_file);  // Remove if exists
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // Train on "hello world" pattern
    printf("Training on 'hello world'...\n");
    const char *training = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
    melvin_m_process_input(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Train a few more times to strengthen edges
    for (int i = 0; i < 3; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph stats: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Test: Query "hello" - should output " world" not "lololo"
    printf("Testing query 'hello'...\n");
    const char *query = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)query, strlen(query));
    melvin_m_process_input(mfile);
    
    // Read output
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_size + 1);
    if (!output) {
        fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    size_t read = melvin_m_universal_output_read(mfile, output, output_size);
    output[read] = '\0';
    
    printf("Input:  '%s'\n", query);
    printf("Output: '%.*s'\n", (int)read, output);
    printf("Expected: ' world' (or similar continuation)\n\n");
    
    // Check for loops (repeating patterns)
    int has_loop = 0;
    if (read >= 4) {
        // Check for 2+ character repeating patterns
        for (size_t i = 0; i < read - 3; i++) {
            if (output[i] == output[i+2] && output[i+1] == output[i+3]) {
                has_loop = 1;
                printf("WARNING: Detected repeating pattern at position %zu: '%.*s'\n", 
                       i, 4, &output[i]);
            }
        }
    }
    
    // Check if output is reasonable (not just loops)
    int is_reasonable = 0;
    if (read > 0) {
        // Check if output contains space (expected for " world")
        for (size_t i = 0; i < read; i++) {
            if (output[i] == ' ') {
                is_reasonable = 1;
                break;
            }
        }
        // Or if it's short and doesn't repeat
        if (read <= 10 && !has_loop) {
            is_reasonable = 1;
        }
    }
    
    printf("=== Test Results ===\n");
    printf("Output length: %zu bytes\n", read);
    printf("Has loop: %s\n", has_loop ? "YES (BAD)" : "NO (GOOD)");
    printf("Is reasonable: %s\n", is_reasonable ? "YES (GOOD)" : "NO (BAD)");
    
    free(output);
    melvin_m_close(mfile);
    unlink(test_file);
    
    if (has_loop) {
        printf("\n❌ TEST FAILED: System still creating loops\n");
        return 1;
    } else if (!is_reasonable) {
        printf("\n⚠️  TEST WARNING: Output doesn't look reasonable\n");
        return 1;
    } else {
        printf("\n✅ TEST PASSED: Context matching prevents loops\n");
        return 0;
    }
}
