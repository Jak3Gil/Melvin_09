#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Hierarchy Consolidation Test ===\n\n");
    
    const char *test_file = "test_consolidation.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Training on '%s' to test consolidation...\n\n", pattern);
    printf("Expected: Graph should consolidate redundant hierarchies over time\n");
    printf("(When 'hello' hierarchy exists, 'he', 'el', 'll', 'lo' become redundant)\n\n");
    
    printf("Progress:\n");
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    int consolidation_detected = 0;
    
    for (int i = 1; i <= 1000; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        size_t current_nodes = melvin_m_get_node_count(mfile);
        size_t current_edges = melvin_m_get_edge_count(mfile);
        
        if (i % 100 == 0 || (i <= 200 && i % 20 == 0)) {
            printf("  Iteration %4d: %2zu nodes, %3zu edges", i, current_nodes, current_edges);
            
            // Check if consolidation is happening (node count decreases after initial growth)
            if (i > 200 && current_nodes < prev_nodes) {
                printf(" [CONSOLIDATING! -%zu nodes]", prev_nodes - current_nodes);
                consolidation_detected = 1;
            } else if (i > 200 && current_nodes == prev_nodes && prev_nodes > 10) {
                printf(" [STABLE]");
            }
            printf("\n");
        }
        
        prev_nodes = current_nodes;
        prev_edges = current_edges;
    }
    
    printf("\nFinal: %zu nodes, %zu edges\n", prev_nodes, prev_edges);
    printf("\nAnalysis:\n");
    printf("  - Minimum nodes (4 bytes): 4\n");
    printf("  - Ideal consolidated: ~5-10 nodes (bytes + 'hello' hierarchy)\n");
    printf("  - Actual: %zu nodes\n", prev_nodes);
    
    if (consolidation_detected) {
        printf("\n✓ Consolidation detected! Redundant hierarchies are being pruned.\n");
    } else if (prev_nodes <= 15) {
        printf("\n~ Graph is reasonably consolidated (within 4x of ideal).\n");
    } else {
        printf("\n✗ Consolidation may need more time or tuning.\n");
        printf("  (Note: Consolidation accelerates decay but nodes only removed when inactive)\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

