#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_association_multi.m";
    
    printf("=== Testing Multiple Association Learning ===\n\n");
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("Created new brain: %s\n", brain_file);
    
    // Step 1: Train with "cat meow" 5 times
    printf("\n=== Training: 'cat meow' ===\n");
    const char *training1 = "cat meow";
    for (int i = 1; i <= 5; i++) {
        printf("Training %d/5... ", i);
        fflush(stdout);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training1, strlen(training1));
        melvin_m_save(mfile);
        printf("done (nodes: %zu, edges: %zu)\n", 
               melvin_m_get_node_count(mfile), 
               melvin_m_get_edge_count(mfile));
    }
    
    // Step 2: Train with "dog bark" 5 times
    printf("\n=== Training: 'dog bark' ===\n");
    const char *training2 = "dog bark";
    for (int i = 1; i <= 5; i++) {
        printf("Training %d/5... ", i);
        fflush(stdout);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training2, strlen(training2));
        melvin_m_save(mfile);
        printf("done (nodes: %zu, edges: %zu)\n", 
               melvin_m_get_node_count(mfile), 
               melvin_m_get_edge_count(mfile));
    }
    
    // Step 3: Test with just "cat"
    printf("\n=== Test 1: 'cat' ===\n");
    const char *test1 = "cat";
    printf("Input: '%s'\n", test1);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test1, strlen(test1));
    
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
            
            // Check if output contains 'm' (from "meow")
            int found_m = 0;
            for (size_t i = 0; i < output_size; i++) {
                if (output[i] == 'm') {
                    found_m = 1;
                    break;
                }
            }
            
            if (found_m) {
                printf("✓ Test 1 PASSED: Output contains 'm' (from 'meow')\n");
            } else {
                printf("✗ Test 1 FAILED: Output does not contain 'm'\n");
            }
            
            free(output);
        }
    } else {
        printf("No output generated.\n");
    }
    
    // Step 4: Test with just "dog"
    printf("\n=== Test 2: 'dog' ===\n");
    const char *test2 = "dog";
    printf("Input: '%s'\n", test2);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test2, strlen(test2));
    
    output_size = melvin_m_universal_output_size(mfile);
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
            
            // Check if output contains 'b' (from "bark")
            int found_b = 0;
            for (size_t i = 0; i < output_size; i++) {
                if (output[i] == 'b') {
                    found_b = 1;
                    break;
                }
            }
            
            if (found_b) {
                printf("✓ Test 2 PASSED: Output contains 'b' (from 'bark')\n");
            } else {
                printf("✗ Test 2 FAILED: Output does not contain 'b'\n");
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
