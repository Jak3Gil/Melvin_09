/*
 * DEBUG: Trace exactly what edges exist and where loops form
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Trace generation output with edge info
void trace_generation(MelvinMFile *mfile, const char *input) {
    fprintf(stderr, "\n=== TRACING GENERATION FOR '%s' ===\n", input);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)input, strlen(input));
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    fprintf(stderr, "Output (%zu bytes): '", out_size);
    for (size_t i = 0; i < out_size && i < 30; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            fprintf(stderr, "%c", output[i]);
        } else {
            fprintf(stderr, "\\x%02x", output[i]);
        }
    }
    fprintf(stderr, "'\n");
    
    free(output);
}

int main() {
    fprintf(stderr, "╔════════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║         TRACING GENERATION PATH                            ║\n");
    fprintf(stderr, "╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_trace.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_trace.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    // Train with 5 iterations
    fprintf(stderr, "\n=== Training 'hello world' 5 times ===\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    fprintf(stderr, "Nodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test
    trace_generation(mfile, "hello");
    
    // Also test single chars to see connections
    fprintf(stderr, "\n=== Testing individual chars ===\n");
    trace_generation(mfile, "d");  // What comes after 'd'?
    trace_generation(mfile, "l");  // What comes after 'l'?
    trace_generation(mfile, "o");  // What comes after 'o'?
    
    melvin_m_close(mfile);
    remove("/tmp/debug_trace.m");
    
    return 0;
}
