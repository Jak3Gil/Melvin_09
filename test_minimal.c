/*
 * Minimal test - train just 2 patterns sequentially
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("Creating .m file...\n");
    MelvinMFile *mfile = melvin_m_create("test_minimal.m");
    
    printf("\n=== Pattern 1: hello world ===\n");
    for (int i = 0; i < 10; i++) {
        printf("Iteration %d...\n", i+1);
        const char *p1 = "hello world";
        melvin_m_universal_input_write(mfile, (const uint8_t*)p1, strlen(p1));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After pattern 1: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    printf("\n=== Pattern 2: good morning ===\n");
    for (int i = 0; i < 10; i++) {
        printf("Iteration %d...\n", i+1);
        const char *p2 = "good morning";
        melvin_m_universal_input_write(mfile, (const uint8_t*)p2, strlen(p2));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After pattern 2: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    printf("✓ Test completed\n");
    
    return 0;
}

