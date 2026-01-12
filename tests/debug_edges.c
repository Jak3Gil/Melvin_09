/*
 * DEBUG: Trace actual edge connections after training
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING EDGE STRUCTURE                           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_edges.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_edges.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train with single pass to minimize complexity
    printf("\n=== Training 'hello world' ONCE ===\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
    melvin_m_universal_output_clear(mfile);
    
    printf("Nodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    printf("\n=== Expected edges from 'hello world' ===\n");
    printf("h→e, e→l, l→l, l→o, o→' ', ' '→w, w→o, o→r, r→l, l→d\n");
    printf("Note: Some of these share the same 'l' and 'o' nodes\n");
    
    printf("\n=== Where can loops form? ===\n");
    printf("If hierarchies create edges like 'ld'→l, then:\n");
    printf("  d → 'ld' → l → l → o → ... (back to earlier nodes)\n");
    printf("Or if 'l' has edge to 'd' (from 'ld') AND 'd' has edge to 'l'...\n");
    
    // Test generation
    printf("\n=== Testing 'hello' ===\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    printf("Output: '%s' (%zu bytes)\n", output, out_size);
    
    free(output);
    melvin_m_close(mfile);
    remove("/tmp/debug_edges.m");
    
    return 0;
}
