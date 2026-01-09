#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_different_inputs.m";
    
    printf("=== Testing Different Input Patterns ===\n\n");
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("Created new brain: %s\n", brain_file);
    
    // Train with "hello world" 5 times
    printf("\n=== Training: 'hello world' (5 times) ===\n");
    const char *training = "hello world";
    for (int i = 1; i <= 5; i++) {
        printf("Training %d/5... ", i);
        fflush(stdout);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training, strlen(training));
        melvin_m_save(mfile);
        printf("done\n");
    }
    
    printf("\n=== Testing Different Inputs ===\n\n");
    
    // Test cases
    struct {
        const char *input;
        const char *description;
    } test_cases[] = {
        {"hello", "Input: 'hello' (no space)"},
        {"hello ", "Input: 'hello ' (with space)"},
        {"hell", "Input: 'hell' (partial)"},
        {"hel", "Input: 'hel' (shorter)"},
        {"h", "Input: 'h' (single char)"},
        {"he", "Input: 'he' (two chars)"},
        {"helloworld", "Input: 'helloworld' (no space)"},
        {"world", "Input: 'world' (reverse)"},
        {"o", "Input: 'o' (last char of hello)"},
        {" w", "Input: ' w' (space + w)"},
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int t = 0; t < num_tests; t++) {
        printf("Test %d: %s\n", t + 1, test_cases[t].description);
        printf("  Input: '%s'\n", test_cases[t].input);
        
        // Clear previous output
        melvin_m_universal_output_clear(mfile);
        
        // Process input
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_cases[t].input, strlen(test_cases[t].input));
        
        // Get output
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            if (output) {
                melvin_m_universal_output_read(mfile, output, output_size);
                printf("  Output (%zu bytes): ", output_size);
                
                // Show first 60 chars
                for (size_t i = 0; i < output_size && i < 60; i++) {
                    if (output[i] >= 32 && output[i] < 127) {
                        printf("%c", output[i]);
                    } else {
                        printf("\\x%02x", output[i]);
                    }
                }
                if (output_size > 60) {
                    printf("...");
                }
                printf("\n");
                
                // Check for 'w' (from "world")
                int found_w = 0;
                for (size_t i = 0; i < output_size; i++) {
                    if (output[i] == 'w') {
                        found_w = 1;
                        break;
                    }
                }
                
                // Check for loops (repeating patterns)
                int has_loop = 0;
                if (output_size >= 6) {
                    // Check for "held" pattern
                    for (size_t i = 0; i + 3 < output_size; i++) {
                        if (output[i] == 'h' && output[i+1] == 'e' && 
                            output[i+2] == 'l' && output[i+3] == 'd') {
                            has_loop = 1;
                            break;
                        }
                    }
                }
                
                printf("  Contains 'w': %s | Has 'held' loop: %s\n", 
                       found_w ? "YES" : "NO",
                       has_loop ? "YES" : "NO");
                
                free(output);
            }
        } else {
            printf("  No output generated.\n");
        }
        printf("\n");
    }
    
    // Final stats
    printf("=== Final Stats ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    return 0;
}
