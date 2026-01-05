#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    MelvinMFile *mfile = melvin_m_create("debug.m");
    if (!mfile) return 1;
    
    printf("Testing readiness calculation...\n\n");
    
    // Feed input
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    // Check graph state
    printf("Graph state:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Try to process again to see readiness
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    printf("\nAfter second input:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Output size: %zu\n", melvin_m_universal_output_size(mfile));
    
    melvin_m_close(mfile);
    return 0;
}
