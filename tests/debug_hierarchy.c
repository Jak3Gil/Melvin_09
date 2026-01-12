/*
 * DEBUG: Why aren't hierarchies forming?
 * 
 * Brain analog: Synaptic consolidation - when patterns are repeated,
 * they should be "chunked" into single units (like learning "hello" as one word)
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING HIERARCHY FORMATION                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_hier.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_hier.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train "ab" many times - simplest possible hierarchy
    printf("\n=== Training 'ab' (simplest pattern) ===\n");
    const char *pattern = "ab";
    
    for (int i = 1; i <= 50; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_output_clear(mfile);
        
        if (i == 1 || i == 5 || i == 10 || i == 20 || i == 50) {
            printf("After %d iterations:\n", i);
            printf("  Nodes: %zu, Edges: %zu\n",
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\n=== Training 'hello' (5-char pattern) ===\n");
    const char *pattern2 = "hello";
    
    for (int i = 1; i <= 50; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern2, strlen(pattern2));
        melvin_m_universal_output_clear(mfile);
        
        if (i == 1 || i == 5 || i == 10 || i == 20 || i == 50) {
            printf("After %d iterations:\n", i);
            printf("  Nodes: %zu, Edges: %zu\n",
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\n=== Final Stats ===\n");
    printf("Total Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Total Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    printf("\nLook at stderr for [HIERARCHY] messages...\n");
    printf("If none appear, hierarchies are NOT being formed!\n");
    
    melvin_m_close(mfile);
    remove("/tmp/debug_hier.m");
    
    return 0;
}
