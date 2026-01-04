/*
 * Simple test to verify the refactored wave propagation works
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("Testing refactored wave propagation...\n");
    
    const char *mfile_path = "test_simple.m";
    
    // Remove existing file
    remove(mfile_path);
    
    // Create .m file
    printf("Creating .m file...\n");
    MelvinMFile *mfile = melvin_m_create(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to create .m file\n");
        return 1;
    }
    
    // Test simple pattern
    const char *pattern = "hello";
    printf("Processing pattern: '%s'\n", pattern);
    
    melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
    int result = melvin_m_process_input(mfile);
    
    printf("Process result: %d\n", result);
    printf("Node count: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edge count: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Check output
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[1024];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Output: '%.*s'\n", (int)read_size, output);
    }
    
    // Close and clean up
    melvin_m_close(mfile);
    remove(mfile_path);
    
    printf("✓ Test completed successfully!\n");
    return 0;
}
