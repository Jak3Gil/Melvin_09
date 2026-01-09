#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Debug Edge Weights ===\n\n");
    
    const char *test_file = "test_debug.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    const char *pattern = "abcd";
    
    // Train for 100 iterations
    for (int iter = 1; iter <= 100; iter++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After 100 training iterations on 'abcd':\n");
    printf("Nodes: %zu, Edges: %zu\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Test
    melvin_m_universal_input_write(mfile, (uint8_t*)"ab", 2);
    melvin_m_process_input(mfile);
    
    size_t output_len = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_len + 1);
    melvin_m_universal_output_read(mfile, output, output_len);
    output[output_len] = '\0';
    
    printf("Input: 'ab'\n");
    printf("Output: '%s' (%zu bytes)\n", output, output_len);
    
    free(output);
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
