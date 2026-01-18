#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main() {
    printf("=== Hierarchy Formation Test ===\n\n");
    
    const char *test_file = "test_hierarchy.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train a simple pattern multiple times to see hierarchy formation
    const char *pattern = "cat meow";
    int iterations = 10;
    
    printf("Training '%s' %d times...\n", pattern, iterations);
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    
    for (int i = 0; i < iterations; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
        
        size_t nodes = melvin_m_get_node_count(mfile);
        size_t edges = melvin_m_get_edge_count(mfile);
        
        if (i == 0 || nodes != prev_nodes || edges != prev_edges) {
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
    
    // Check if we can generate output
    printf("\n=== Testing Generation ===\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        output[read] = '\0';
        
        printf("Input 'cat' -> '");
        for (size_t i = 0; i < read && i < 20; i++) {
            printf("%c", output[i] >= 32 ? output[i] : '?');
        }
        printf("'\n");
        free(output);
    } else {
        printf("Input 'cat' -> (empty)\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
