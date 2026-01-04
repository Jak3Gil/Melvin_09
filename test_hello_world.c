#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_hello_world_brain.m";
    
    printf("=== Testing Hello World Association Learning ===\n\n");
    
    // Create or load brain
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        printf("Creating new brain: %s\n", brain_file);
        mfile = melvin_m_create(brain_file);
        if (!mfile) {
            fprintf(stderr, "Failed to create brain file\n");
            return 1;
        }
    } else {
        printf("Loaded existing brain: %s\n", brain_file);
    }
    
    // Step 1: Train with "hello world" multiple times
    printf("\n=== Step 1: Training ===\n");
    const char *training_input = "hello world";
    printf("Training input: '%s'\n", training_input);
    
    for (int i = 1; i <= 10; i++) {
        printf("Training iteration %d... ", i);
        fflush(stdout);
        
        // Process input
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training_input, strlen(training_input));
        
        // Check output size
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            printf("(output: %zu bytes) ", output_size);
        }
        
        // Save after each iteration
        melvin_m_save(mfile);
        printf("done\n");
    }
    
    // Show graph stats
    printf("\nGraph stats: %zu nodes, %zu edges\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Step 2: Test with just "hello"
    printf("\n=== Step 2: Testing Association ===\n");
    const char *test_input = "hello";
    printf("Test input: '%s'\n", test_input);
    printf("Expected: Should output 'world' (or at least start with 'w')\n\n");
    
    // Clear any previous output
    melvin_m_universal_output_clear(mfile);
    
    // Process test input
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    // Check output
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            size_t read = melvin_m_universal_output_read(mfile, output, output_size);
            printf("Output bytes: ");
            for (size_t i = 0; i < read && i < 20; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("'%c' ", output[i]);
                } else {
                    printf("0x%02x ", output[i]);
                }
            }
            printf("\n");
            
            // Check if output contains 'w' (first letter of 'world')
            int found_w = 0;
            for (size_t i = 0; i < read; i++) {
                if (output[i] == 'w') {
                    found_w = 1;
                    break;
                }
            }
            
            if (found_w) {
                printf("\n✅ SUCCESS: Output contains 'w' (first letter of 'world')!\n");
            } else {
                printf("\n⚠️  Output does not contain 'w'. Let's see what we got:\n");
                printf("Output as string: ");
                for (size_t i = 0; i < read && i < 50; i++) {
                    if (output[i] >= 32 && output[i] < 127) {
                        printf("%c", output[i]);
                    } else {
                        printf(".");
                    }
                }
                printf("\n");
            }
            
            free(output);
        }
    } else {
        printf("\n⚠️  No output generated. This might mean:\n");
        printf("   - Patterns are not mature enough (need more training)\n");
        printf("   - Output readiness threshold not met\n");
        printf("   - System is in 'thinking mode' (internal processing only)\n");
    }
    
    // Final save
    melvin_m_save(mfile);
    melvin_m_close(mfile);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}

