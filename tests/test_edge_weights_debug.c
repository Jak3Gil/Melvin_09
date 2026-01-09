/* Test: Debug Edge Weights After Training
 * 
 * Shows edge weights after training to understand why learning isn't working
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// No forward declarations needed - included in melvin.h

int main() {
    printf("=== Edge Weight Debug ===\n\n");
    
    remove("test_debug.m");
    MelvinMFile *mfile = melvin_m_create("test_debug.m");
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Train 3 iterations
    printf("Training 'hello world' for 3 iterations...\n\n");
    for (int i = 0; i < 3; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test
    printf("Testing 'hello ' → expect 'world'\n\n");
    melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello ", 6);
    size_t len = melvin_m_universal_output_size(mfile);
    uint8_t *out = malloc(len + 1);
    melvin_m_universal_output_read(mfile, out, len);
    out[len] = '\0';
    
    printf("Output: '%.*s'\n\n", (int)(len > 50 ? 50 : len), out);
    free(out);
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("Graph: %zu nodes, %zu edges\n", nodes, edges);
    
    melvin_m_close(mfile);
    return 0;
}
