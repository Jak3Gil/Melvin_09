#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// Add some debug output to trace what's happening
int main() {
    printf("=== Trace Generation Debug ===\n\n");
    
    // Create fresh file
    MelvinMFile *mfile = melvin_m_create("test_trace.m");
    if (!mfile) {
        printf("Failed to create file\n");
        return 1;
    }
    
    // Learn "hello" once
    const char *input = "hello";
    printf("Step 1: Learning '%s'\n", input);
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
    melvin_m_process_input(mfile);
    
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    printf("Graph: %zu nodes, %zu edges\n\n", node_count, edge_count);
    
    // Test 1: Generate from 'h' (should produce "ello")
    printf("Step 2: Generate from 'h' (expecting 'ello')\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    uint8_t output1[256];
    size_t output1_len = melvin_m_universal_output_read(mfile, output1, sizeof(output1));
    printf("Output: '");
    for (size_t i = 0; i < output1_len; i++) {
        printf("%c", output1[i]);
    }
    printf("' (%zu bytes)\n\n", output1_len);
    
    // Test 2: Generate from 'he' (should produce "llo")
    printf("Step 3: Generate from 'he' (expecting 'llo')\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"he", 2);
    uint8_t output2[256];
    size_t output2_len = melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    printf("Output: '");
    for (size_t i = 0; i < output2_len; i++) {
        printf("%c", output2[i]);
    }
    printf("' (%zu bytes)\n\n", output2_len);
    
    // Test 3: Generate from 'hel' (should produce "lo")
    printf("Step 4: Generate from 'hel' (expecting 'lo')\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hel", 3);
    uint8_t output3[256];
    size_t output3_len = melvin_m_universal_output_read(mfile, output3, sizeof(output3));
    printf("Output: '");
    for (size_t i = 0; i < output3_len; i++) {
        printf("%c", output3[i]);
    }
    printf("' (%zu bytes)\n\n", output3_len);
    
    // Test 4: Generate from empty (should use wave state)
    printf("Step 5: Generate from empty input\n");
    melvin_m_universal_input_clear(mfile);
    uint8_t output4[256];
    size_t output4_len = melvin_m_universal_output_read(mfile, output4, sizeof(output4));
    printf("Output: '");
    for (size_t i = 0; i < output4_len; i++) {
        printf("%c", output4[i]);
    }
    printf("' (%zu bytes)\n", output4_len);
    
    melvin_m_close(mfile);
    return 0;
}

