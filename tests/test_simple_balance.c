#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Testing Growth/Decay Balance ===\n\n");
    
    const char *test_file = "test_balance.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    
    // Train on simple sequence
    const char *pattern = "abc";
    
    printf("Training on: '%s'\n\n", pattern);
    printf("Iter | Nodes | Edges | Output (input='a')\n");
    printf("-----|-------|-------|-------------------\n");
    
    for (int iter = 1; iter <= 100; iter++) {
        // Train
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 10 iterations
        if (iter % 10 == 0) {
            melvin_m_universal_input_write(mfile, (uint8_t*)"a", 1);
            melvin_m_process_input(mfile);
            
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            melvin_m_universal_output_read(mfile, output, output_len);
            output[output_len] = '\0';
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            printf("%4d | %5zu | %5zu | %s\n", iter, nodes, edges, output);
            
            free(output);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
