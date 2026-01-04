/*
 * Test processing same pattern multiple times
 */

#include <stdio.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("Creating .m file...\n");
    MelvinMFile *mfile = melvin_m_create("test_same.m");
    
    const char *pattern = "hello";
    
    for (int i = 0; i < 20; i++) {
        printf("Iteration %d...\n", i+1);
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        printf("  Nodes: %zu, Edges: %zu\n",
               melvin_m_get_node_count(mfile),
               melvin_m_get_edge_count(mfile));
    }
    
    melvin_m_close(mfile);
    printf("✓ Test completed\n");
    
    return 0;
}

