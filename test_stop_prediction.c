#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    const char *brain_file = "test_stop_brain.m";
    
    printf("=== Testing Wave Propagation Stop Prediction ===\n\n");
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    // Train with "hello world" 5 times
    printf("Training: 'hello world' x5\n");
    const char *training = "hello world";
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training, strlen(training));
        melvin_m_save(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Test with just "hello"
    printf("Test input: 'hello'\n");
    printf("Expected: Should output 'world' and STOP (not loop)\n\n");
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            size_t read = melvin_m_universal_output_read(mfile, output, output_size);
            printf("Output: \"");
            for (size_t i = 0; i < read && i < 100; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("%c", output[i]);
                } else {
                    printf("\\x%02x", output[i]);
                }
            }
            printf("\"\n");
            
            // Check for 'w' (first letter of world)
            int found_w = 0;
            for (size_t i = 0; i < read; i++) {
                if (output[i] == 'w') {
                    found_w = 1;
                    break;
                }
            }
            
            if (found_w) {
                printf("\n✅ SUCCESS: Output contains 'w' (learned association!)\n");
            }
            
            // Check if output is reasonable length (not looping)
            if (read > 0 && read < 50) {
                printf("✅ SUCCESS: Output stopped at reasonable length (%zu bytes)\n", read);
            } else if (read >= 50) {
                printf("⚠️  WARNING: Output is very long (%zu bytes) - may be looping\n", read);
            }
            
            free(output);
        }
    } else {
        printf("⚠️  No output generated\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

