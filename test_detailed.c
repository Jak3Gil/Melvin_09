#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== DETAILED TEST: MULTI-BYTE OUTPUT ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_detailed.m");
    
    // Train with simple repeating pattern
    printf("Training: 'abc' repeated 100 times...\n");
    for (int i = 0; i < 100; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abc", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test with 'a'
    printf("Test 1: Input 'a'\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"a", 1);
    
    size_t read1 = melvin_m_universal_output_size(mfile);
    uint8_t output1[256];
    melvin_m_universal_output_read(mfile, output1, sizeof(output1));
    
    printf("Output: %zu bytes\n", read1);
    if (read1 > 0) {
        printf("Content: ");
        for (size_t i = 0; i < read1 && i < 20; i++) {
            printf("%02x ", output1[i]);
        }
        printf("\n");
    }
    printf("\n");
    
    // Clear and test with 'ab'
    melvin_m_universal_output_clear(mfile);
    printf("Test 2: Input 'ab'\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
    
    size_t read2 = melvin_m_universal_output_size(mfile);
    uint8_t output2[256];
    melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    
    printf("Output: %zu bytes\n", read2);
    if (read2 > 0) {
        printf("Content: ");
        for (size_t i = 0; i < read2 && i < 20; i++) {
            printf("%02x ", output2[i]);
        }
        printf("\n");
    }
    
    if (read1 > 1 || read2 > 1) {
        printf("\n✅ SUCCESS! Multi-byte output achieved!\n");
        printf("   Test 1: %zu bytes\n", read1);
        printf("   Test 2: %zu bytes\n", read2);
    } else {
        printf("\n⚠️  Still 1 byte in both tests\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

