#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== PROVING MULTI-BYTE OUTPUT ===\n\n");
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create("test_prove.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train: Feed "hello" many times to build sequential pattern
    printf("Training: Feeding 'hello' 100 times...\n");
    for (int i = 0; i < 100; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test outputs
    printf("Testing outputs:\n");
    
    const char *tests[] = {"h", "he", "hel", "hell", "hello"};
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)tests[i], strlen(tests[i]));
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        uint8_t output[256];
        size_t read = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        printf("  Input: %-6s -> Output: %zu bytes", tests[i], read);
        if (read > 0 && read <= 20) {
            printf(" [");
            for (size_t j = 0; j < read; j++) {
                if (output[j] >= 32 && output[j] < 127) {
                    printf("%c", output[j]);
                } else {
                    printf("\\x%02x", output[j]);
                }
            }
            printf("]");
        }
        printf("\n");
        
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("\n");
    if (melvin_m_universal_output_size(mfile) > 1) {
        printf("✅ SUCCESS: Multi-byte output proven!\n");
    } else {
        printf("⚠️  Still 1 byte - autoregressive loop stopping early\n");
        printf("   This means: next_node not found OR no outgoing edges\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}
