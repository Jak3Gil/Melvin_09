#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/melvin.h"

int main() {
    printf("=== Debug Generation Test ===\n\n");
    
    // Create system
    MelvinFile *mfile = melvin_m_create();
    if (!mfile) {
        fprintf(stderr, "Failed to create Melvin system\n");
        return 1;
    }
    
    // Train on simple pattern
    printf("Training: 'hello world'\n");
    for (int i = 0; i < 100; i++) {
        const char *pattern = "hello world";
        melvin_m_process_input(mfile, (const uint8_t*)pattern, strlen(pattern));
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           mfile->graph->node_count, mfile->graph->edge_count);
    
    // Test with debug output enabled
    printf("Test: 'hello w' (should continue with 'orld')\n");
    printf("Expected: 'orld'\n\n");
    
    const char *input = "hello w";
    melvin_m_process_input(mfile, (const uint8_t*)input, strlen(input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size, 0);
            output[output_size] = '\0';
            printf("Actual:   '%s'\n", output);
            free(output);
        }
    }
    
    melvin_m_destroy(mfile);
    return 0;
}
