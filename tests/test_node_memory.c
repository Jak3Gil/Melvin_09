#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"
#include "melvin_in_port.h"

int main() {
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║          MELVIN NODE MEMORY ANALYSIS (Target: < 100 bytes/node)          ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n\n");
    
    const char *test_file = "node_memory_test.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Train on simple pattern
    const char *pattern = "hello";
    for (int i = 0; i < 10; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    printf("Graph Statistics:\n");
    printf("  Nodes: %zu\n", nodes);
    printf("  Edges: %zu\n", edges);
    printf("  Pattern: '%s' (%zu bytes)\n", pattern, strlen(pattern));
    printf("\n");
    
    // Estimate memory usage
    // Node structure size (from code analysis)
    size_t node_struct_size = sizeof(void*) * 20 + sizeof(size_t) * 10 + sizeof(float) * 10 + sizeof(uint32_t) * 3 + 9; // Approximate
    size_t estimated_node_memory = node_struct_size * nodes;
    
    // Edge structure size
    size_t edge_struct_size = sizeof(void*) * 4 + sizeof(size_t) * 2 + sizeof(uint8_t) + sizeof(float) * 2; // Approximate
    size_t estimated_edge_memory = edge_struct_size * edges;
    
    // Context tags (each edge can have multiple)
    // This is the likely culprit - each context tag has a SparseContext
    // SparseContext has: Node** array, float* array, uint8_t* array, uint32_t* array
    // Each can be large if many nodes are active
    
    printf("Memory Analysis:\n");
    printf("  Estimated Node struct size: ~%zu bytes\n", node_struct_size);
    printf("  Estimated Edge struct size: ~%zu bytes\n", edge_struct_size);
    printf("  Estimated node memory: ~%zu bytes (%zu KB)\n", 
           estimated_node_memory, estimated_node_memory / 1024);
    printf("  Estimated edge memory: ~%zu bytes (%zu KB)\n", 
           estimated_edge_memory, estimated_edge_memory / 1024);
    printf("\n");
    
    printf("Per-Node Memory:\n");
    printf("  Target: < 100 bytes/node\n");
    printf("  Estimated: ~%zu bytes/node (from structure)\n", node_struct_size);
    
    if (node_struct_size < 100) {
        printf("  ✓ Node structure is within target\n");
    } else {
        printf("  ⚠️  Node structure exceeds target\n");
    }
    
    printf("\n");
    printf("Key Insight:\n");
    printf("  The 11MB/node seen in tests is likely from:\n");
    printf("  1. Context tags on edges (SparseContext objects)\n");
    printf("  2. Edge arrays (outgoing/incoming edges per node)\n");
    printf("  3. MiniNet structures within nodes\n");
    printf("  4. Embeddings and other cached data\n");
    printf("\n");
    printf("  The Node structure itself should be < 100 bytes,\n");
    printf("  but total memory per node includes all connected edges\n");
    printf("  and their context tags.\n");
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
