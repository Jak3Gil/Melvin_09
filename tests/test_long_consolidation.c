#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Long-Term Consolidation Test ===\n\n");
    
    // Create Melvin instance
    MFile *mfile = melvin_m_create("test_long_consolidation.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create MFile\n");
        return 1;
    }
    
    // Train on "hello" for 10,000 iterations
    const char *input = "hello";
    printf("Training on '%s' for 10,000 iterations...\n\n", input);
    
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    
    for (int i = 1; i <= 10000; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t *)input, strlen(input));
        melvin_m_process_input(mfile);
        
        if (i % 1000 == 0 || i == 100 || i == 500) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            char status[20] = "";
            if (nodes < prev_nodes) {
                snprintf(status, sizeof(status), "✓ CONSOLIDATING");
            } else if (nodes == prev_nodes) {
                snprintf(status, sizeof(status), "[STABLE]");
            }
            
            printf("  Iteration %5d: %3zu nodes, %3zu edges %s\n", i, nodes, edges, status);
            
            prev_nodes = nodes;
            prev_edges = edges;
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal: %zu nodes, %zu edges\n\n", final_nodes, final_edges);
    
    // Test output
    printf("Testing output after 10,000 iterations:\n");
    const char *test = "hel";
    melvin_m_universal_input_write(mfile, (const uint8_t *)test, strlen(test));
    melvin_m_process_input(mfile);
    
    uint8_t output[100];
    size_t out_len = melvin_m_universal_output_read(mfile, output, 100);
    
    printf("  Input: '%s' -> Output: '", test);
    for (size_t i = 0; i < out_len && i < 20; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", out_len);
    
    // Check if output is correct
    int correct = (out_len >= 2 && output[0] == 'l' && output[1] == 'o');
    printf("\n%s Output %s\n", correct ? "✓" : "✗", correct ? "CORRECT" : "INCORRECT");
    
    melvin_m_close(mfile);
    
    return correct ? 0 : 1;
}

