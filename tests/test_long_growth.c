/*
 * Long-term growth test for Melvin
 * - Processes many inputs to observe growth patterns
 * - Tracks outputs and statistics over time
 * - Shows how the graph evolves with self-destruct system
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain_file.m> <input1> [input2] ... [inputN]\n", argv[0]);
        return 1;
    }
    
    const char *brain_file = argv[1];
    
    printf("==========================================\n");
    printf("Melvin Long-Term Growth Test\n");
    printf("==========================================\n");
    printf("Brain file: %s\n\n", brain_file);
    
    // Load or create brain
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        mfile = melvin_m_create(brain_file);
        if (!mfile) {
            fprintf(stderr, "Failed to create brain file\n");
            return 1;
        }
        printf("Created new brain file\n");
    } else {
        printf("Loaded existing brain file\n");
    }
    
    size_t initial_nodes = melvin_m_get_node_count(mfile);
    size_t initial_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nInitial state:\n");
    printf("  Nodes: %zu\n", initial_nodes);
    printf("  Edges: %zu\n", initial_edges);
    if (initial_nodes > 0) {
        printf("  Edge-to-node ratio: %.2f:1\n", (float)initial_edges / (float)initial_nodes);
    }
    
    printf("\n==========================================\n");
    printf("Processing %d inputs...\n", argc - 2);
    printf("==========================================\n\n");
    
    // Process each input
    for (int i = 2; i < argc; i++) {
        const char *input_file = argv[i];
        size_t nodes_before = melvin_m_get_node_count(mfile);
        size_t edges_before = melvin_m_get_edge_count(mfile);
        
        printf("--- Input %d: %s ---\n", i - 1, input_file);
        
        // Process input
        uint8_t port_id = 1;
        if (melvin_in_port_handle_text_file(mfile, port_id, input_file) != 0) {
            fprintf(stderr, "Failed to process input file: %s\n", input_file);
            continue;
        }
        
        // Save after each input
        if (melvin_m_save(mfile) != 0) {
            fprintf(stderr, "Warning: Failed to save brain file\n");
        }
        
        size_t nodes_after = melvin_m_get_node_count(mfile);
        size_t edges_after = melvin_m_get_edge_count(mfile);
        
        printf("  Nodes: %zu (+%zu)\n", nodes_after, nodes_after - nodes_before);
        printf("  Edges: %zu (+%zu)\n", edges_after, edges_after - edges_before);
        
        if (nodes_after > 0) {
            printf("  Edge-to-node ratio: %.2f:1\n", (float)edges_after / (float)nodes_after);
        }
        
        // Check for output
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            printf("  Output: %zu bytes\n", output_size);
            
            // Read and show first 100 bytes of output
            uint8_t output_buffer[200];
            size_t read_size = melvin_m_universal_output_read(mfile, output_buffer, sizeof(output_buffer));
            if (read_size > 0) {
                printf("  Output preview: ");
                size_t preview_len = (read_size < 100) ? read_size : 100;
                for (size_t j = 0; j < preview_len; j++) {
                    uint8_t byte = output_buffer[j];
                    if (byte >= 32 && byte < 127) {
                        printf("%c", byte);
                    } else {
                        printf("\\x%02x", byte);
                    }
                }
                if (read_size > 100) {
                    printf("...");
                }
                printf("\n");
            }
        } else {
            printf("  Output: None\n");
        }
        
        printf("\n");
    }
    
    printf("==========================================\n");
    printf("Final Statistics\n");
    printf("==========================================\n");
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    printf("  Nodes: %zu\n", final_nodes);
    printf("  Edges: %zu\n", final_edges);
    if (final_nodes > 0) {
        printf("  Edge-to-node ratio: %.2f:1\n", (float)final_edges / (float)final_nodes);
    }
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    // Check output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("\nFinal Output:\n");
        printf("  Size: %zu bytes\n", output_size);
        
        uint8_t output_buffer[500];
        size_t read_size = melvin_m_universal_output_read(mfile, output_buffer, sizeof(output_buffer));
        if (read_size > 0) {
            printf("  Content: ");
            for (size_t i = 0; i < read_size && i < 200; i++) {
                uint8_t byte = output_buffer[i];
                if (byte >= 32 && byte < 127) {
                    printf("%c", byte);
                } else {
                    printf("\\x%02x", byte);
                }
            }
            if (read_size > 200) {
                printf("...");
            }
            printf("\n");
        }
    } else {
        printf("\nNo output generated\n");
    }
    
    // Cleanup
    melvin_m_close(mfile);
    
    return 0;
}

