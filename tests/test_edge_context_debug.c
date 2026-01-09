#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

/*
 * Debug test: Check what edge contexts are being stored
 */

int main() {
    printf("=== Edge Context Debug Test ===\n\n");
    
    const char *test_file = "test_edge_debug.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Train on two simple patterns
    const char *pattern1 = "hello world";
    const char *pattern2 = "hello friend";
    
    printf("Training on:\n");
    printf("  1. '%s'\n", pattern1);
    printf("  2. '%s'\n\n", pattern2);
    
    // Train 10 times each
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern1, strlen(pattern1));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern2, strlen(pattern2));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training complete.\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n\n", melvin_m_get_edge_count(mfile));
    
    // Now test different inputs to see what happens
    struct {
        const char *input;
        const char *description;
    } tests[] = {
        {"hello ", "After 'hello ' (ambiguous - could be world or friend)"},
        {"hello w", "After 'hello w' (should continue with 'orld')"},
        {"hello f", "After 'hello f' (should continue with 'riend')"},
    };
    
    for (int t = 0; t < 3; t++) {
        printf("Test: %s\n", tests[t].description);
        printf("  Input: '%s'\n", tests[t].input);
        
        melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].input, strlen(tests[t].input));
        melvin_m_process_input(mfile);
        
        size_t output_len = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        printf("  Output: '%.*s'\n\n", (int)output_len, output);
        
        free(output);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
