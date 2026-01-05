/*
 * Debug Test for Self-Modification
 * Prints detailed context information to understand edge disambiguation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("=========================================\n");
    printf("MELVIN SELF-MODIFICATION DEBUG TEST\n");
    printf("=========================================\n\n");
    
    remove("debug_self_mod.m");
    
    MelvinMFile *brain = melvin_m_create("debug_self_mod.m");
    if (!brain) {
        fprintf(stderr, "ERROR: Failed to create brain\n");
        return 1;
    }
    
    // Simple training - just two similar sequences
    printf("Training on 'hello world' and 'hello there'...\n\n");
    
    for (int iter = 0; iter < 30; iter++) {
        melvin_m_universal_input_write(brain, (const uint8_t *)"hello world", 11);
        melvin_m_process_input(brain);
        melvin_m_universal_output_clear(brain);  // Clear output after training
        
        melvin_m_universal_input_write(brain, (const uint8_t *)"hello there", 11);
        melvin_m_process_input(brain);
        melvin_m_universal_output_clear(brain);  // Clear output after training
    }
    
    printf("Nodes: %zu, Edges: %zu\n\n", 
           melvin_m_get_node_count(brain),
           melvin_m_get_edge_count(brain));
    
    // Test output
    printf("Testing: 'hello' -> expecting ' world' or ' there'\n");
    
    melvin_m_universal_input_write(brain, (const uint8_t *)"hello", 5);
    melvin_m_process_input(brain);
    
    size_t output_size = melvin_m_universal_output_size(brain);
    uint8_t output[256] = {0};
    if (output_size > 0) {
        melvin_m_universal_output_read(brain, output, sizeof(output) - 1);
    }
    
    printf("Output: '");
    for (size_t i = 0; i < output_size && i < 50; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf(".");
        }
    }
    printf("' (len=%zu)\n", output_size);
    
    melvin_m_close(brain);
    
    printf("\n✅ Debug test complete!\n");
    return 0;
}

