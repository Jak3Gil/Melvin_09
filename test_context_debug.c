#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Context-Aware Edge Selection Debug ===\n\n");
    
    // Create fresh file
    MelvinMFile *mfile = melvin_m_create("test_context_debug.m");
    if (!mfile) {
        printf("Failed to create file\n");
        return 1;
    }
    
    // Learn "hello" once
    const char *input = "hello";
    printf("Learning: '%s'\n", input);
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
    melvin_m_process_input(mfile);
    
    // Check graph structure
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    printf("After learning: %zu nodes, %zu edges\n\n", node_count, edge_count);
    
    // Generate output starting from 'h'
    printf("Generating output starting from 'h'...\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    uint8_t output[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output (%zu bytes): '", output_len);
    for (size_t i = 0; i < output_len; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("'\n\n");
    
    // Try learning more times
    printf("Learning 'hello' 10 more times...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
        melvin_m_process_input(mfile);
    }
    
    node_count = melvin_m_get_node_count(mfile);
    edge_count = melvin_m_get_edge_count(mfile);
    printf("After 11 repetitions: %zu nodes, %zu edges\n\n", node_count, edge_count);
    
    // Generate again
    printf("Generating output again...\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output (%zu bytes): '", output_len);
    for (size_t i = 0; i < output_len; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("'\n");
    
    melvin_m_close(mfile);
    return 0;
}

