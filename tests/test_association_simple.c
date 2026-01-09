#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_association.m";
    
    printf("=== Testing Association Learning ===\n\n");
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("Created new brain: %s\n", brain_file);
    
    // Step 1: Train with "hello world" 5 times
    printf("\n=== Training: 'hello world' ===\n");
    const char *training = "hello world";
    for (int i = 1; i <= 5; i++) {
        printf("Training %d/5... ", i);
        fflush(stdout);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training, strlen(training));
        melvin_m_save(mfile);
        printf("done (nodes: %zu, edges: %zu)\n", 
               melvin_m_get_node_count(mfile), 
               melvin_m_get_edge_count(mfile));
    }
    
    // Step 2: Test with just "hello"
    printf("\n=== Testing: 'hello' ===\n");
    const char *test_input = "hello";
    printf("Input: '%s'\n", test_input);
    
    // Clear any output from training iterations
    melvin_m_universal_output_clear(mfile);
    
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    // Get output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
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
            
            // Check if output contains 'w' (from "world")
            int found_w = 0;
            for (size_t i = 0; i < output_size; i++) {
                if (output[i] == 'w') {
                    found_w = 1;
                    break;
                }
            }
            
            if (found_w) {
                printf("\n✓ SUCCESS: System learned association! Output contains 'w' (from 'world')\n");
            } else {
                printf("\n✗ FAILED: Output does not contain 'w'. Association not learned.\n");
            }
            
            free(output);
        }
    } else {
        printf("No output generated.\n");
    }
    
    // Final stats
    printf("\n=== Final Stats ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    melvin_m_close(mfile);
    return 0;
}

