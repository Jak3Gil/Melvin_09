#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// Minimal test to understand what's happening
int main() {
    printf("=== Detailed Trace ===\n\n");
    
    // Create fresh file
    MelvinMFile *mfile = melvin_m_create("test_detailed.m");
    if (!mfile) {
        printf("Failed to create file\n");
        return 1;
    }
    
    // Learn "hello" once
    printf("Learning: 'hello'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Now generate from just 'h'
    printf("Input: 'h' (1 byte)\n");
    printf("Expected: System should follow h→e→l→l→o path\n");
    printf("Expected output: 'ello' or similar\n\n");
    
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    uint8_t output[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Actual output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n\n", output_len);
    
    // Hypothesis: Maybe the wave state is choosing 'l' node instead of 'h' node?
    // Or maybe 'h' node is choosing 'l' instead of 'e'?
    
    printf("Analysis:\n");
    if (output_len >= 2 && output[0] == 'l' && output[1] == 'l') {
        printf("- Output starts with 'll'\n");
        printf("- This suggests either:\n");
        printf("  1. Wave chose 'l' node instead of 'h' node as start\n");
        printf("  2. 'h' node chose 'l' edge instead of 'e' edge\n");
        printf("  3. Context-aware selection is not working\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

