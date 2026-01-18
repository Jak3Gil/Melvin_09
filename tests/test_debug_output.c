/*
 * Debug test to understand why output generation fails
 */

#include "../src/melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    printf("=== Debug Output Generation Test ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("debug_test.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create MFile\n");
        return 1;
    }
    
    // Train on "hello world"
    printf("Training: 'hello world'\n");
    const char *training = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
    melvin_m_process_input(mfile);
    
    printf("  Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Clear and test with "hello"
    printf("\nTesting: 'hello'\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("  Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[256];
        size_t read = melvin_m_universal_output_read(mfile, output, sizeof(output));
        output[read] = '\0';
        printf("  Output: '%s'\n", output);
        printf("  Expected: ' world'\n");
        
        if (read == 6 && memcmp(output, " world", 6) == 0) {
            printf("\n✓ SUCCESS: Output matches expected!\n");
            melvin_m_close(mfile);
            return 0;
        } else {
            printf("\n✗ FAIL: Output doesn't match\n");
        }
    } else {
        printf("  ✗ FAIL: No output generated\n");
    }
    
    melvin_m_close(mfile);
    return 1;
}
