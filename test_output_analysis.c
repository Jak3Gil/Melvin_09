#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file> [brain_file]\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *brain_file = (argc > 2) ? argv[2] : "test_brain.m";
    
    // Open brain
    MelvinMFile *mfile = melvin_m_open(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to open brain: %s\n", brain_file);
        return 1;
    }
    
    // Read input file
    FILE *f = fopen(input_file, "r");
    if (!f) {
        fprintf(stderr, "Failed to open input: %s\n", input_file);
        melvin_m_close(mfile);
        return 1;
    }
    
    char line[1024];
    int input_count = 0;
    
    printf("=== Testing Node Edge Collection and Output Generation ===\n\n");
    
    while (fgets(line, sizeof(line), f)) {
        // Remove newline
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        if (len == 0) continue;
        
        input_count++;
        printf("--- Input #%d: '%s' ---\n", input_count, line);
        
        // Get node/edge counts before
        size_t nodes_before = melvin_m_get_node_count(mfile);
        size_t edges_before = melvin_m_get_edge_count(mfile);
        
        // Process input
        melvin_in_port_handle_text_file(mfile, 1, line, len);
        
        // Get node/edge counts after
        size_t nodes_after = melvin_m_get_node_count(mfile);
        size_t edges_after = melvin_m_get_edge_count(mfile);
        
        printf("  Nodes: %zu -> %zu (+%zu)\n", nodes_before, nodes_after, nodes_after - nodes_before);
        printf("  Edges: %zu -> %zu (+%zu)\n", edges_before, edges_after, edges_after - edges_before);
        
        // Get output
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            if (output) {
                melvin_m_universal_output_read(mfile, output, output_size);
                printf("  Output (%zu bytes): ", output_size);
                for (size_t i = 0; i < output_size && i < 100; i++) {
                    if (output[i] >= 32 && output[i] < 127) {
                        printf("%c", output[i]);
                    } else {
                        printf("\\x%02x", output[i]);
                    }
                }
                printf("\n");
                
                // Analyze output
                if (output_size > 0) {
                    printf("  Analysis: ");
                    if (strstr((char*)output, line) != NULL) {
                        printf("Output contains input (echo/learning)\n");
                    } else if (output_size >= len && memcmp(output, line, len) == 0) {
                        printf("Output starts with input (continuation)\n");
                    } else {
                        printf("Output is different from input (generation)\n");
                    }
                }
                free(output);
            }
        } else {
            printf("  Output: (none)\n");
        }
        
        printf("\n");
    }
    
    fclose(f);
    melvin_m_close(mfile);
    
    printf("=== Summary ===\n");
    printf("Processed %d inputs\n", input_count);
    
    return 0;
}
