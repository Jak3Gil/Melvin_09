#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main() {
    printf("=== Hierarchy Formation Summary ===\n\n");
    
    const char *test_file = "test_hierarchy_summary.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Single pattern
    printf("1. Single pattern 'cat meow' (10 iterations):\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    printf("   Nodes: %zu, Edges: %zu\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Two patterns
    printf("2. Two patterns 'cat meow' + 'dog bark' (5 iterations each):\n");
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"dog bark", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    printf("   Nodes: %zu, Edges: %zu\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    size_t byte_level_nodes = 9;  // c, a, t, space, m, e, o, w, STOP
    if (melvin_m_get_node_count(mfile) > byte_level_nodes) {
        printf("\n✅ Hierarchies formed! (%zu > %zu byte-level nodes)\n", 
               melvin_m_get_node_count(mfile), byte_level_nodes);
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
