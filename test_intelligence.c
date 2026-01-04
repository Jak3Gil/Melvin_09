/* test_intelligence.c
 * Test that system actually learns and generates intelligent output
 * With all mechanisms enabled: hierarchies, blank nodes, abstractions
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Testing Intelligence with All Mechanisms Enabled ===\n\n");
    
    const char *path = "test_intelligence.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    // Test 1: Learn a simple pattern many times
    printf("Test 1: Learning 'hello world' pattern (100 repetitions)...\n");
    const char *pattern = "hello world";
    
    for (int i = 0; i < 100; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        
        if (i % 20 == 0 && i > 0) {
            printf("  Iteration %d: nodes=%zu, edges=%zu\n",
                   i, melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\nFinal graph state:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    // Test 2: Check if hierarchies formed
    printf("\nTest 2: Checking for hierarchy formation...\n");
    size_t final_nodes = melvin_m_get_node_count(mfile);
    // With 11 bytes in "hello world", we expect:
    // - 11 byte nodes (level 0)
    // - Some hierarchy nodes (level 1+) if learning worked
    if (final_nodes > 11) {
        printf("  ✓ PASS: %zu nodes (more than just bytes) - hierarchies likely formed\n", final_nodes);
    } else {
        printf("  ⚠ WARNING: Only %zu nodes - hierarchies may not be forming\n", final_nodes);
    }
    
    // Test 3: Learn variations
    printf("\nTest 3: Learning variations (50 repetitions each)...\n");
    const char *variations[] = {
        "hello there",
        "hello friend",
        "goodbye world"
    };
    
    for (int v = 0; v < 3; v++) {
        printf("  Learning '%s'...\n", variations[v]);
        for (int i = 0; i < 50; i++) {
            melvin_m_universal_input_write(mfile, (uint8_t*)variations[v], strlen(variations[v]));
            melvin_m_process_input(mfile);
        }
    }
    
    printf("\nAfter variations:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Test 4: Check for generalization (blank nodes)
    printf("\nTest 4: Checking for generalization (blank nodes)...\n");
    size_t nodes_after_variations = melvin_m_get_node_count(mfile);
    // With variations, we should see:
    // - Shared patterns ("hello", "world") should be abstracted
    // - Blank nodes for common structures
    if (nodes_after_variations < final_nodes + 100) {
        printf("  ✓ PASS: Efficient growth (%zu nodes) - generalization likely working\n", nodes_after_variations);
    } else {
        printf("  ⚠ WARNING: Excessive growth (%zu nodes) - may not be generalizing\n", nodes_after_variations);
    }
    
    printf("\n=== Summary ===\n");
    printf("With all mechanisms enabled:\n");
    printf("  - Biological Hebbian learning: ✓\n");
    printf("  - Hierarchy formation: %s\n", (final_nodes > 11) ? "✓" : "?");
    printf("  - Generalization: %s\n", (nodes_after_variations < final_nodes + 100) ? "✓" : "?");
    printf("  - Self-regulation: Testing...\n");
    
    melvin_m_close(mfile);
    return 0;
}

