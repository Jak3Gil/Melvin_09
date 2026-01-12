/*
 * DEBUG: More detailed trace of generation
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    fprintf(stderr, "╔════════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║         DETAILED GENERATION TRACE                          ║\n");
    fprintf(stderr, "╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_trace2.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_trace2.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    // Train ONCE to keep graph simple
    fprintf(stderr, "\n=== Training 'hello world' ONCE ===\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
    
    // Check what output was generated DURING training
    size_t train_out = melvin_m_universal_output_size(mfile);
    fprintf(stderr, "Output during training: %zu bytes\n", train_out);
    if (train_out > 0) {
        uint8_t *out = malloc(train_out + 1);
        melvin_m_universal_output_read(mfile, out, train_out);
        out[train_out] = '\0';
        fprintf(stderr, "Training output: '%s'\n", out);
        free(out);
    }
    
    fprintf(stderr, "\nNodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Clear output
    melvin_m_universal_output_clear(mfile);
    
    // Now test with "hello"
    fprintf(stderr, "\n=== Testing 'hello' ===\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    fprintf(stderr, "Test output (%zu bytes): '%s'\n", out_size, output);
    
    // Character by character analysis
    fprintf(stderr, "\nCharacter-by-character:\n");
    for (size_t i = 0; i < out_size; i++) {
        fprintf(stderr, "  [%zu] '%c' (0x%02x)\n", i, 
               (output[i] >= 32 && output[i] < 127) ? output[i] : '?',
               output[i]);
    }
    
    free(output);
    melvin_m_close(mfile);
    remove("/tmp/debug_trace2.m");
    
    return 0;
}
