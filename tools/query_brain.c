/*
 * Query Brain Tool
 * 
 * Tests a trained brain file with various queries
 * 
 * Usage: ./query_brain <brain.m> <query1> [query2] ...
 */

#include "../src/melvin.h"
#include "../src/melvin_in_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain.m> <query1> [query2] ...\n", argv[0]);
        fprintf(stderr, "\nExample:\n");
        fprintf(stderr, "  %s trained.m \"Paris is\" \"The cat\" \"Water is\"\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              MELVIN BRAIN QUERY TOOL                       ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Brain: %s\n", brain_path);
    printf("\n");
    
    // Open brain (use create which opens existing if present)
    MelvinMFile *mfile = melvin_m_create(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error opening brain file: %s\n", brain_path);
        return 1;
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    printf("Brain Statistics:\n");
    printf("  Nodes: %zu\n", nodes);
    printf("  Edges: %zu\n", edges);
    printf("  Avg degree: %.2f\n", nodes > 0 ? (double)edges / (double)nodes : 0.0);
    printf("\n");
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("                        QUERIES                             \n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    // Process each query
    for (int i = 2; i < argc; i++) {
        const char *query = argv[i];
        size_t query_len = strlen(query);
        
        printf("Query %d: \"%s\"\n", i - 1, query);
        
        // Clear previous output
        melvin_m_universal_output_clear(mfile);
        
        // Feed query
        int result = melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)query, query_len);
        if (result < 0) {
            printf("  Error processing query\n\n");
            continue;
        }
        
        // Get output
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t output[1024];
            size_t read_size = (output_size < sizeof(output)) ? output_size : sizeof(output) - 1;
            melvin_m_universal_output_read(mfile, output, read_size);
            output[read_size] = '\0';
            
            printf("  Output: \"%s\"\n", output);
            printf("  Length: %zu bytes\n", output_size);
        } else {
            printf("  Output: (no output)\n");
        }
        
        printf("\n");
    }
    
    printf("═══════════════════════════════════════════════════════════\n");
    printf("\n");
    
    melvin_m_close(mfile);
    
    return 0;
}
