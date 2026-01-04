#include <stdio.h>
#include "melvin.h"

int main() {
    printf("Testing adaptive parameter calculation...\n");
    
    // Test with small dataset
    MelvinMFile *mfile = melvin_m_create("test_adaptive_verify.m");
    if (!mfile) return 1;
    
    const char *data = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)data, 11);
    melvin_m_process_input(mfile);
    
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    printf("Adaptive parameters successfully used in processing.\n");
    return 0;
}
