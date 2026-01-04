#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Brain Mechanism Debug Test ===\n\n");
    
    // Create fresh .m file
    const char *test_file = "test_brain_debug.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    printf("Learning 'hello' 10 times...\n");
    for (int i = 0; i < 10; i++) {
        const char *input = "hello";
        melvin_m_universal_input_write(mfile, (uint8_t*)input, strlen(input));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("Graph: %zu nodes, %zu edges\n\n", nodes, edges);
    
    printf("Test: Input 'hello' again\n");
    const char *test = "hello";
    melvin_m_universal_input_write(mfile, (uint8_t*)test, strlen(test));
    melvin_m_process_input(mfile);
    
    uint8_t output[256];
    size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Input:  '%s'\n", test);
    printf("Output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n\n", out_len);
    
    // Now test partial input
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    printf("Test: Input 'hel' (partial)\n");
    const char *test2 = "hel";
    melvin_m_universal_input_write(mfile, (uint8_t*)test2, strlen(test2));
    melvin_m_process_input(mfile);
    
    uint8_t output2[256];
    size_t out_len2 = melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    
    printf("Input:  '%s'\n", test2);
    printf("Output: '");
    for (size_t i = 0; i < out_len2; i++) {
        printf("%c", output2[i]);
    }
    printf("' (%zu bytes)\n\n", out_len2);
    
    // Test single character
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    printf("Test: Input 'h' (single char)\n");
    const char *test3 = "h";
    melvin_m_universal_input_write(mfile, (uint8_t*)test3, strlen(test3));
    melvin_m_process_input(mfile);
    
    uint8_t output3[256];
    size_t out_len3 = melvin_m_universal_output_read(mfile, output3, sizeof(output3));
    
    printf("Input:  '%s'\n", test3);
    printf("Output: '");
    for (size_t i = 0; i < out_len3; i++) {
        printf("%c", output3[i]);
    }
    printf("' (%zu bytes)\n\n", out_len3);
    
    printf("Key insight: The brain-like mechanism uses activated context.\n");
    printf("If output is empty, wave propagation may not be activating nodes.\n");
    printf("If output is 'lll', the system is following strong l->l edges.\n");
    printf("The context (activated nodes) should guide which edges win.\n");
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

