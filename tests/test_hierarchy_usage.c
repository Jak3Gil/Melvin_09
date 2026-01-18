/*
 * Test: Verify hierarchies are being formed and used
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Hierarchy Formation and Usage Test ===\n\n");
    
    const char *test_file = "test_hierarchy_usage.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // Train on "hello world" many times to form hierarchies
    printf("Training on 'hello world' (20 times)...\n");
    const char *training = "hello world";
    for (int i = 0; i < 20; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph stats: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Query "hello" - should use hierarchy if formed
    printf("Testing query 'hello'...\n");
    const char *query = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)query, strlen(query));
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_size + 1);
    if (!output) {
        fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    size_t read = melvin_m_universal_output_read(mfile, output, output_size);
    if (read > output_size) read = output_size;
    if (read > 0 && read <= output_size) {
        output[read] = '\0';
    } else {
        read = 0;
        output[0] = '\0';
    }
    
    printf("Input:  '%s'\n", query);
    printf("Output: '%.*s' (%zu bytes)\n", (int)read, output, read);
    printf("Expected: ' world'\n\n");
    
    // Check if output matches expected
    int matches = (read >= 6 && memcmp(output, " world", 6) == 0);
    
    printf("=== Results ===\n");
    if (matches) {
        printf("✅ PASSED: Output matches expected (hierarchies working)\n");
    } else {
        printf("⚠️  Output doesn't match expected\n");
        printf("   This suggests hierarchies may not be forming or being used\n");
    }
    
    free(output);
    melvin_m_close(mfile);
    unlink(test_file);
    
    return matches ? 0 : 1;
}
