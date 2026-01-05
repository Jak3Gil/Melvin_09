#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== TESTING CONVERGENCE LOGIC ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_conv.m");
    
    // Train with very simple pattern many times to ensure strong edges
    printf("Training: 'ab' repeated 500 times...\n");
    for (int i = 0; i < 500; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test with 'a' - should output 'b'
    printf("Test: Input 'a'\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"a", 1);
    
    size_t read = melvin_m_universal_output_size(mfile);
    uint8_t output[256];
    melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output: %zu bytes\n", read);
    if (read > 0) {
        printf("Bytes: ");
        for (size_t i = 0; i < read && i < 10; i++) {
            printf("'%c' (0x%02x) ", (output[i] >= 32 && output[i] < 127) ? output[i] : '.', output[i]);
        }
        printf("\n");
    }
    
    if (read > 1) {
        printf("\n✅ SUCCESS! Multi-byte output: %zu bytes\n", read);
    } else {
        printf("\n⚠️  Only %zu byte(s) - loop stopping after first sample\n", read);
    }
    
    melvin_m_close(mfile);
    return 0;
}

