#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main() {
    printf("=== New Edge Creation Rules Test ===\n\n");
    
    const char *test_file = "test_new_rules.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train patterns to form hierarchies and test edge creation
    const char *patterns[] = {"cat meow", "dog bark", "bird chirp"};
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Training %d patterns (20 iterations each)...\n", num_patterns);
    
    for (int iter = 0; iter < 20; iter++) {
        for (int p = 0; p < num_patterns; p++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    float edges_per_node = nodes > 0 ? (float)edges / (float)nodes : 0.0f;
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", nodes);
    printf("  Edges: %zu\n", edges);
    printf("  Edges per node: %.2f\n", edges_per_node);
    
    if (edges_per_node > 3.15f) {
        printf("\n✅ Edge density improved! (%.2f > 3.15 edges/node)\n", edges_per_node);
        printf("   (Previous: ~3.15 edges/node)\n");
    } else if (edges_per_node > 3.0f) {
        printf("\n✅ Edge density maintained (%.2f edges/node)\n", edges_per_node);
    } else {
        printf("\n⚠️  Edge density: %.2f edges/node\n", edges_per_node);
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
