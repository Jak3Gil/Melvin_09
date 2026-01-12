/*
 * DEBUG: Node deduplication and cycle detection
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING NODE DEDUPLICATION                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_nodes.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_nodes.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train "hello world" - contains 'l' twice in "hello" and once in "world"
    printf("\n=== Training 'hello world' ===\n");
    printf("String contains: h-e-l-l-o- -w-o-r-l-d\n");
    printf("Note: 'l' appears 3 times, 'o' appears 2 times\n\n");
    
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
    melvin_m_universal_output_clear(mfile);
    
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    
    printf("After training:\n");
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    
    // "hello world" has 8 unique chars: h, e, l, o, space, w, r, d
    // If deduplication works, we should have ~8 char nodes + some hierarchy nodes
    printf("\n=== Analysis ===\n");
    printf("Unique characters in 'hello world': h, e, l, o, ' ', w, r, d = 8\n");
    printf("If nodes are deduplicated, expect ~8 char nodes + hierarchies\n");
    printf("Actual nodes: %zu\n", node_count);
    
    if (node_count <= 12) {
        printf("✓ Reasonable node count - deduplication likely working\n");
    } else if (node_count > 20) {
        printf("⚠ Too many nodes - possible deduplication issue\n");
    }
    
    // Now let's examine what edges 'd' has
    printf("\n=== Examining 'd' node's edges ===\n");
    printf("'d' is the last char of 'world'\n");
    printf("It should have weak/no outgoing edges (end of pattern)\n");
    printf("But if it connects to 'l', cycles can form!\n");
    
    melvin_m_close(mfile);
    remove("/tmp/debug_nodes.m");
    
    return 0;
}
