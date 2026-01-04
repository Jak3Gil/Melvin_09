#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    const char *brain_file = argc >= 2 ? argv[1] : "demo_brain.m";
    
    printf("=== Melvin: Watching Actual Outputs ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to create brain\n");
        return 1;
    }
    
    // Sequence of inputs
    const char *inputs[] = {
        "hello",
        "hello", 
        "hello",
        "world",
        "world",
        "hello world",
        NULL
    };
    
    for (int i = 0; inputs[i]; i++) {
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("Input %d: '%s'\n", i+1, inputs[i]);
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        
        // Feed input
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)inputs[i], strlen(inputs[i]));
        
        // Check output size
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("Output size: %zu bytes\n", output_size);
        
        if (output_size > 0) {
            // Read actual output bytes
            uint8_t *output = malloc(output_size + 1);
            if (output) {
                size_t read = melvin_m_universal_output_read(mfile, output, output_size);
                output[read] = '\0';
                
                printf("Output bytes: ");
                for (size_t j = 0; j < read; j++) {
                    if (output[j] >= 32 && output[j] < 127) {
                        printf("%c", output[j]);
                    } else {
                        printf("\\x%02x", output[j]);
                    }
                }
                printf("\n");
                
                printf("Output (as string): \"%s\"\n", output);
                free(output);
            }
        } else {
            printf("Output: (empty - patterns not mature enough yet)\n");
        }
        
        printf("Nodes: %zu, Edges: %zu, Adaptations: %lu\n", 
               melvin_m_get_node_count(mfile),
               melvin_m_get_edge_count(mfile),
               (unsigned long)melvin_m_get_adaptation_count(mfile));
        printf("\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}
