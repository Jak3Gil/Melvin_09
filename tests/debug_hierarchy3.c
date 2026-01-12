/*
 * DEBUG: Verify hierarchies exist after training
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// External function to check hierarchies
extern size_t melvin_m_count_hierarchies(MelvinMFile *mfile);

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         VERIFYING HIERARCHY CREATION                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_hier3.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_hier3.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    printf("\n=== Before Training ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    
    // Train "hello" 5 times
    printf("\n=== Training 'hello' 5 times ===\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After training:\n");
    printf("  Total Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Total Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Count nodes (we know hierarchies have abstraction_level > 0)
    // The char nodes are: h, e, l, o = 4 unique chars
    // If hierarchies formed: he, el, ll, lo = 4 hierarchies
    // Total should be ~8 nodes
    
    size_t node_count = melvin_m_get_node_count(mfile);
    printf("\n=== Analysis ===\n");
    printf("If node_count > 4 (unique chars in 'hello'), hierarchies formed!\n");
    printf("Node count: %zu\n", node_count);
    
    if (node_count > 4) {
        printf("✓ HIERARCHIES ARE BEING CREATED!\n");
        printf("  Approx %zu hierarchy nodes\n", node_count - 4);
    } else {
        printf("✗ No hierarchies formed\n");
    }
    
    // Save and reload to verify persistence
    printf("\n=== Testing Persistence ===\n");
    melvin_m_save(mfile);
    melvin_m_close(mfile);
    
    mfile = melvin_m_load("/tmp/debug_hier3.m");
    if (mfile) {
        printf("After reload:\n");
        printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
        printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
        melvin_m_close(mfile);
    } else {
        printf("  (load failed or no persistence API)\n");
    }
    
    remove("/tmp/debug_hier3.m");
    
    return 0;
}
