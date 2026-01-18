#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main(void) {
    printf("=== Parallel Activation Debug Test ===\n\n");
    
    const char *test_file = "test_parallel.m";
    unlink(test_file);
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    // Train on "cat meow" multiple times
    const char *pattern = "cat meow";
    printf("Training on pattern '%s' 10 times...\n", pattern);
    
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph stats after training:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    // Test generation from "cat"
    printf("\nTesting generation from 'cat':\n");
    
    // Write input
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    
    // Read output
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu\n", output_size);
    
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        if (!output) {
            printf("Failed to allocate output buffer\n");
            melvin_m_close(mfile);
            unlink(test_file);
            return 1;
        }
        
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        if (read > output_size) read = output_size;
        output_size = read;
        
        printf("Output bytes: ");
        for (size_t i = 0; i < output_size; i++) {
            printf("0x%02x ", output[i]);
        }
        printf("\n");
        
        printf("Output text: '");
        for (size_t i = 0; i < output_size; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf(".");
            }
        }
        printf("'\n");
        
        // Check if output matches expected
        const char *expected = " meow";
        if (output_size == strlen(expected) && 
            memcmp(output, expected, output_size) == 0) {
            printf("\n✅ PASS: Output matches expected '%s'\n", expected);
        } else {
            printf("\n❌ FAIL: Expected '%s', got different output\n", expected);
        }
        
        free(output);
    } else {
        printf("❌ FAIL: Empty output\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return output_size > 0 ? 0 : 1;
}
