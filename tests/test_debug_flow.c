#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain.m> <input.txt>\n", argv[0]);
        return 1;
    }
    
    MelvinMFile *mfile = melvin_m_open(argv[1]);
    if (!mfile) {
        fprintf(stderr, "Failed to open brain file\n");
        return 1;
    }
    
    // Read input file
    FILE *f = fopen(argv[2], "r");
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
    
    printf("=== BEFORE PROCESSING ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Process input
    melvin_m_universal_input_write(mfile, (uint8_t*)input, size);
    melvin_m_process_input(mfile);
    
    printf("\n=== AFTER PROCESSING ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Check output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        melvin_m_universal_output_read(mfile, output, output_size);
        printf("Output (%zu bytes): ", output_size);
        for (size_t i = 0; i < output_size && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("\\x%02x", output[i]);
            }
        }
        printf("\n");
        free(output);
    } else {
        printf("No output generated\n");
    }
    
    melvin_m_close(mfile);
    free(input);
    return 0;
}

