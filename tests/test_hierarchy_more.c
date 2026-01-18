#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main() {
    printf("=== Hierarchy Formation (More Iterations) ===\n\n");
    
    const char *test_file = "test_hierarchy_more.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train a simple pattern many times to see hierarchy formation
    const char *pattern = "cat meow";
    int iterations = 50;
    
    printf("Training '%s' %d times...\n", pattern, iterations);
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    
    for (int i = 0; i < iterations; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Check every 10 iterations
        if ((i + 1) % 10 == 0 || i == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            printf("  Iteration %d: %zu nodes (+%zu), %zu edges (+%zu)\n", 
                   i + 1, nodes, nodes - prev_nodes, edges, edges - prev_edges);
            prev_nodes = nodes;
            prev_edges = edges;
        }
    }
    
    printf("\nFinal Statistics:\n");
    printf("  Total nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Total edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    // If we have more than 9 nodes (byte-level + STOP), hierarchies formed!
    if (melvin_m_get_node_count(mfile) > 9) {
        printf("\n✅ Hierarchies formed! (more than 9 byte-level nodes)\n");
    } else {
        printf("\n⚠️  No hierarchies formed yet (still at byte level)\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
