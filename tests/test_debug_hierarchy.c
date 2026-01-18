#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main() {
    printf("=== Debug Hierarchy Formation ===\n\n");
    
    const char *test_file = "test_debug_hierarchy.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train just once to see initial state
    printf("Training 'cat meow' once...\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
    melvin_m_process_input(mfile);
    melvin_m_universal_output_clear(mfile);
    
    printf("After 1 iteration:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Train more
    for (int i = 1; i < 10; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After 10 iterations:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
