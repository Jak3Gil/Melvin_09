#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

// Add a simple test that manually checks the graph structure
int main() {
    printf("=== DEBUGGING 1-BYTE OUTPUT ISSUE ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_debug.m");
    
    // Train with simple pattern
    printf("Training: 'ab' repeated 200 times...\n");
    for (int i = 0; i < 200; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test
    printf("Test: Input 'a'\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"a", 1);
    
    size_t read = melvin_m_universal_output_size(mfile);
    uint8_t output[256];
    melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output: %zu bytes\n", read);
    if (read > 0) {
        printf("Bytes: ");
        for (size_t i = 0; i < read && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("'%c' ", output[i]);
            } else {
                printf("0x%02x ", output[i]);
            }
        }
        printf("\n");
    }
    
    printf("\nExpected: After training 'ab' 200 times, input 'a' should output 'b'\n");
    printf("If output is only 1 byte, the autoregressive loop is stopping after first sample.\n");
    
    melvin_m_close(mfile);
    return 0;
}
