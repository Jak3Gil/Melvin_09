#include <stdio.h>
#include "melvin.h"

int main() {
    // Create brain
    MelvinMFile *mfile = melvin_m_create("test_debug.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    // Input: "hello"
    const char *input = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, 5);
    
    printf("Before process_input:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Process
    int result = melvin_m_process_input(mfile);
    
    printf("\nAfter process_input:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Output size: %zu\n", melvin_m_universal_output_size(mfile));
    printf("  Result: %d\n", result);
    
    // Close
    melvin_m_close(mfile);
    
    return 0;
}
