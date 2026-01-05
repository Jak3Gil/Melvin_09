#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// We need to access internal structures to debug
// This is a hack for debugging only
typedef struct Node Node;
typedef struct Edge Edge;

int main() {
    printf("=== Edge Selection Debug ===\n\n");
    
    // Create and learn
    MelvinMFile *mfile = melvin_m_create("test_edge_debug.m");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    printf("After learning 'hello':\n");
    printf("Nodes: %zu, Edges: %zu\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Try to generate
    printf("Generating from 'h'...\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    uint8_t output[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n\n", output_len);
    
    printf("Expected: 'ello' (following h→e→l→l→o)\n");
    printf("Actual: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output[i]);
    }
    printf("'\n\n");
    
    if (output_len > 0 && output[0] != 'e') {
        printf("ERROR: First byte should be 'e' (from h→e edge)\n");
        printf("But got '%c' instead\n", output[0]);
        printf("\nPossible causes:\n");
        printf("1. 'h' node has multiple outgoing edges (shouldn't happen)\n");
        printf("2. Context-aware selection is choosing wrong edge\n");
        printf("3. Edge weights are incorrect\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

