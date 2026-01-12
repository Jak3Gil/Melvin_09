/*
 * DEBUG: Test with 3 iterations to see the loop
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    fprintf(stderr, "=== Testing with 3 iterations ===\n");
    
    remove("/tmp/debug_3iter.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_3iter.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    // Train 3 times
    for (int i = 0; i < 3; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    fprintf(stderr, "\nNodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test with "hello"
    fprintf(stderr, "\n=== Testing 'hello' ===\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    fprintf(stderr, "Test output (%zu bytes): '%s'\n", out_size, output);
    
    free(output);
    melvin_m_close(mfile);
    remove("/tmp/debug_3iter.m");
    
    return 0;
}
