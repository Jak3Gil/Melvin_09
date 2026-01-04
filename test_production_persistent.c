#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m> [input1] [input2] ...\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    uint8_t port_id = 1;
    
    // Open or create .m file (persistent across all inputs)
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        mfile = melvin_m_create(brain_path);
        if (!mfile) {
            fprintf(stderr, "Error: Failed to create/open brain file\n");
            return 1;
        }
    }
    
    printf("==========================================\n");
    printf("Melvin Production Test - Persistent Brain\n");
    printf("==========================================\n");
    printf("Brain file: %s\n", brain_path);
    printf("\n");
    
    // Initial statistics
    size_t initial_nodes = melvin_m_get_node_count(mfile);
    size_t initial_edges = melvin_m_get_edge_count(mfile);
    printf("Initial state:\n");
    printf("  Nodes: %zu\n", initial_nodes);
    printf("  Edges: %zu\n", initial_edges);
    printf("\n");
    
    // Process all inputs sequentially (same .m file, grows)
    for (int i = 2; i < argc; i++) {
        const char *input_file = argv[i];
        
        printf("--- Processing: %s ---\n", input_file);
        
        // Process input using melvin_in_port
        int result = melvin_in_port_handle_text_file(mfile, port_id, input_file);
        if (result < 0) {
            fprintf(stderr, "Error: Failed to process %s\n", input_file);
            continue;
        }
        
        // Save after each input
        melvin_m_save(mfile);
        
        // Get statistics
        size_t node_count = melvin_m_get_node_count(mfile);
        size_t edge_count = melvin_m_get_edge_count(mfile);
        uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
        
        printf("  Nodes: %zu (+%zu)\n", node_count, node_count - initial_nodes);
        printf("  Edges: %zu (+%zu)\n", edge_count, edge_count - initial_edges);
        printf("  Adaptations: %llu\n", (unsigned long long)adaptations);
        
        // Check for output
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            printf("  Output: %zu bytes\n", output_size);
        }
        
        initial_nodes = node_count;
        initial_edges = edge_count;
        printf("\n");
    }
    
    // Final statistics
    printf("==========================================\n");
    printf("Final Statistics\n");
    printf("==========================================\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    printf("\n");
    
    // Check for output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("Output available: %zu bytes\n", output_size);
        printf("\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

