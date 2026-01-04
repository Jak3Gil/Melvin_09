#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m> <input.txt>\n", argv[0]);
        return 1;
    }
    
    const char *brain_file = argv[1];
    const char *input_file = (argc > 2) ? argv[2] : "test_all_input1.txt";
    
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        mfile = melvin_m_create(brain_file);
    }
    if (!mfile) {
        fprintf(stderr, "Failed to open/create brain file\n");
        return 1;
    }
    
    // Read input file
    FILE *f = fopen(input_file, "r");
    if (!f) {
        fprintf(stderr, "Failed to open input file\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *input = malloc(size + 1);
    fread(input, 1, size, f);
    input[size] = '\0';
    fclose(f);
    
    printf("=== Pipeline Test: Feeding input %d times ===\n\n", 10);
    
    for (int i = 1; i <= 10; i++) {
        printf("--- Input %d ---\n", i);
        
        // Write input
        melvin_m_universal_input_write(mfile, (uint8_t*)input, size);
        
        // Process
        melvin_m_process_input(mfile);
        
        // Get stats
        size_t nodes = melvin_m_get_node_count(mfile);
        size_t edges = melvin_m_get_edge_count(mfile);
        
        // Read output
        size_t output_size = melvin_m_universal_output_size(mfile);
        
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            printf("  Output (%zu bytes): \"", output_size);
            for (size_t j = 0; j < output_size && j < 100; j++) {
                if (output[j] >= 32 && output[j] < 127) {
                    putchar(output[j]);
                } else {
                    printf("\\x%02x", output[j]);
                }
            }
            printf("\"\n");
            
            free(output);
        } else {
            printf("  Output: (none - pure thinking mode)\n");
        }
        
        printf("  Graph: %zu nodes, %zu edges\n", nodes, edges);
        printf("\n");
        
        // Save after each input
        melvin_m_save(mfile);
    }
    
    printf("=== Final State ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    free(input);
    return 0;
}

