#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    size_t expected_len = strlen(expected);
    size_t errors = 0;
    size_t max_len = (actual_len > expected_len) ? actual_len : expected_len;
    
    for (size_t i = 0; i < max_len; i++) {
        if (i >= actual_len || i >= expected_len) {
            errors++;
        } else if (actual[i] != (uint8_t)expected[i]) {
            errors++;
        }
    }
    
    return (max_len > 0) ? ((float)errors / (float)max_len) : 1.0f;
}

int main() {
    printf("=== Detailed Error Rate Analysis ===\n\n");
    
    const char *test_file = "test_detailed_error.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    const char *input_prefix = "hello ";
    const char *expected_output = "world";
    
    printf("Training: '%s'\n", pattern);
    printf("Testing: '%s' -> expecting: '%s'\n\n", input_prefix, expected_output);
    
    printf("Iter | Nodes | Edges | Error%% | Output (full)\n");
    printf("-----|-------|-------|--------|------------------\n");
    
    for (int iter = 1; iter <= 500; iter++) {
        // Train
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 10 iterations
        if (iter % 10 == 0) {
            melvin_m_universal_input_write(mfile, (uint8_t*)input_prefix, strlen(input_prefix));
            melvin_m_process_input(mfile);
            
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            melvin_m_universal_output_read(mfile, output, output_len);
            output[output_len] = '\0';
            
            float error_rate = calculate_error_rate(expected_output, output, output_len);
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            printf("%4d | %5zu | %5zu | %5.1f%% | %.*s\n", 
                   iter, nodes, edges, error_rate * 100.0f, 
                   (int)(output_len < 40 ? output_len : 40), output);
            
            free(output);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
