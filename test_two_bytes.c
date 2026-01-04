#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Two Bytes Test ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_two.m");
    
    // Learn "hello"
    printf("Learning 'Xhello'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"Xhello", 6);  // X as port_id
    melvin_m_process_input(mfile);
    melvin_m_universal_output_clear(mfile);
    melvin_m_universal_input_clear(mfile);
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Generate from 'h'
    printf("Generating from 'Xh' (X=port_id, h=data)\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"Xh", 2);
    melvin_m_process_input(mfile);
    
    uint8_t output[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    printf("Output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", output_len);
    printf("Expected: 'ello' or similar\n");
    
    melvin_m_close(mfile);
    return 0;
}

