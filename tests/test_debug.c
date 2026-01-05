/*
 * Debug test to identify the issue
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("Creating .m file...\n");
    MelvinMFile *mfile = melvin_m_create("test_debug.m");
    if (!mfile) {
        printf("ERROR: Failed to create mfile\n");
        return 1;
    }
    
    printf("Initial state: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    printf("Training pattern 1...\n");
    const char *pattern = "hello world";
    
    for (int i = 0; i < 5; i++) {
        printf("  Iteration %d\n", i+1);
        
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        printf("    Input written\n");
        
        int result = melvin_m_process_input(mfile);
        printf("    Processed (result=%d)\n", result);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("    Output size: %zu\n", output_size);
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        printf("    Cleared\n");
    }
    
    printf("\nFinal state: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    printf("✓ Test completed\n");
    
    return 0;
}

