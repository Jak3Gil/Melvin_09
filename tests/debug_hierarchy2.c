/*
 * DEBUG: Where are the hierarchies going?
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING HIERARCHY FORMATION v2                   ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_hier2.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_hier2.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    fprintf(stderr, "\n=== BEFORE ANY TRAINING ===\n");
    fprintf(stderr, "Nodes: %zu, Edges: %zu\n", 
            melvin_m_get_node_count(mfile),
            melvin_m_get_edge_count(mfile));
    
    // Train just once
    fprintf(stderr, "\n=== TRAINING 'hello' ONCE ===\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    fprintf(stderr, "\n=== AFTER 1 TRAINING ===\n");
    fprintf(stderr, "Nodes: %zu, Edges: %zu\n", 
            melvin_m_get_node_count(mfile),
            melvin_m_get_edge_count(mfile));
    
    melvin_m_universal_output_clear(mfile);
    
    // Train 4 more times
    for (int i = 0; i < 4; i++) {
        fprintf(stderr, "\n=== TRAINING %d ===\n", i+2);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
        fprintf(stderr, "Nodes: %zu, Edges: %zu\n", 
                melvin_m_get_node_count(mfile),
                melvin_m_get_edge_count(mfile));
        melvin_m_universal_output_clear(mfile);
    }
    
    fprintf(stderr, "\n=== FINAL SUMMARY ===\n");
    fprintf(stderr, "Total Nodes: %zu\n", melvin_m_get_node_count(mfile));
    fprintf(stderr, "Total Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    remove("/tmp/debug_hier2.m");
    
    return 0;
}
