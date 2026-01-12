#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_association(const char *brain_file, const char *training_phrase, const char *test_input, char expected_char) {
    printf("\n=== Testing: '%s' → '%s' ===\n", test_input, training_phrase);
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return;
    }
    
    // Train
    printf("Training with '%s'... ", training_phrase);
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training_phrase, strlen(training_phrase));
        melvin_m_save(mfile);
    }
    printf("done (nodes: %zu, edges: %zu)\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Test
    printf("Testing with '%s'... ", test_input);
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("Output (%zu bytes): '", output_size);
            for (size_t i = 0; i < output_size && i < 50; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("%c", output[i]);
                } else {
                    printf("\\x%02x", output[i]);
                }
            }
            printf("'\n");
            
            // Check if output contains expected character
            int found = 0;
            for (size_t i = 0; i < output_size; i++) {
                if (output[i] == expected_char) {
                    found = 1;
                    break;
                }
            }
            
            if (found) {
                printf("✓ PASSED: Output contains '%c'\n", expected_char);
            } else {
                printf("✗ FAILED: Output does not contain '%c'\n", expected_char);
            }
            
            free(output);
        }
    } else {
        printf("No output generated.\n");
    }
    
    melvin_m_close(mfile);
}

int main() {
    printf("=== Testing Association Learning (Separate Brains) ===\n");
    
    test_association("test_assoc_1.m", "hello world", "hello", 'w');
    test_association("test_assoc_2.m", "cat meow", "cat", 'm');
    test_association("test_assoc_3.m", "dog bark", "dog", 'b');
    test_association("test_assoc_4.m", "sun shine", "sun", 's');
    
    printf("\n=== All Tests Complete ===\n");
    
    return 0;
}
