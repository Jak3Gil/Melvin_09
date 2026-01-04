#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Hierarchy Progression Test ===\n\n");
    
    const char *test_file = "test_hier_prog.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Training on '%s' and checking hierarchy growth:\n\n", pattern);
    
    for (int i = 1; i <= 500; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i == 1 || i == 10 || i == 50 || i == 100 || i == 200 || i == 500) {
            printf("After iteration %d:\n", i);
            printf("  Total nodes: %zu, edges: %zu\n", 
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
            printf("\n");
        }
    }
    
    printf("\n=== Testing Output ===\n\n");
    
    // Test 1
    printf("Test 1: Input 'hel' → Expected 'lo'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hel", 3);
    melvin_m_process_input(mfile);
    
    uint8_t output[256];
    size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("  Output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", out_len);
    
    if (out_len == 2 && output[0] == 'l' && output[1] == 'o') {
        printf("  ✓ PERFECT!\n");
    } else {
        printf("  ✗ Failed\n");
    }
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Test 2
    printf("\nTest 2: Input 'h' → Expected 'ello'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    melvin_m_process_input(mfile);
    
    out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("  Output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", out_len);
    
    if (out_len == 4 && memcmp(output, "ello", 4) == 0) {
        printf("  ✓ PERFECT!\n");
    } else {
        printf("  ✗ Failed\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

