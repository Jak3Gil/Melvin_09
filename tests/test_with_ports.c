/* Test: Using Port Pipeline for Input
 * 
 * Demonstrates proper use of melvin_in_port for input handling
 * This is the minimal change to use the full pipeline
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== Test Using Port Pipeline ===\n\n");
    
    // Create .m file
    const char *test_file = "test_ports.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    
    printf("Training with port pipeline:\n");
    printf("Pattern: '%s'\n\n", pattern);
    
    // Train using PORT PIPELINE (not direct API)
    for (int i = 0; i < 100; i++) {
        // Use melvin_in_port_process_device instead of direct write
        // Port ID 0 = standard input
        melvin_in_port_process_device(mfile, 0, 
                                       (uint8_t*)pattern, 
                                       strlen(pattern));
        
        // Clear output after each iteration
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training complete (100 iterations)\n\n");
    
    // Test: Generate output
    printf("Testing generation:\n");
    const char *test_input = "hello ";
    
    // Input through port pipeline
    melvin_in_port_process_device(mfile, 0,
                                   (uint8_t*)test_input,
                                   strlen(test_input));
    
    // Read output (can still use direct API for reading)
    size_t output_len = melvin_m_universal_output_size(mfile);
    if (output_len > 0) {
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        printf("Input: '%s'\n", test_input);
        printf("Output: '%s'\n", output);
        printf("Expected: 'world'\n");
        
        free(output);
    } else {
        printf("No output generated\n");
    }
    
    // Stats
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nGraph: %zu nodes, %zu edges\n", nodes, edges);
    
    melvin_m_close(mfile);
    
    printf("\n=== Key Difference ===\n");
    printf("OLD: melvin_m_universal_input_write() + melvin_m_process_input()\n");
    printf("NEW: melvin_in_port_process_device() [does both + port tracking]\n");
    
    return 0;
}
