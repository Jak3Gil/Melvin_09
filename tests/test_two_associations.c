/* Test two associations in a single brain file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Two Associations Test ===\n\n");
    
    const char *test_file = "test_two_assoc.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    // Train two simple associations
    printf("Training 'cat meow' 5 times...\n");
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training 'dog bark' 5 times...\n");
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"dog bark", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    printf("\n=== Testing Associations ===\n\n");
    
    // Test cat -> meow
    printf("Test 1: 'cat' -> ");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        output[read] = '\0';
        
        printf("'");
        for (size_t i = 0; i < read && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf(".");
            }
        }
        printf("'");
        
        int match = (read >= 4 && output[0] == ' ' && output[1] == 'm' && 
                     output[2] == 'e' && output[3] == 'o' && output[4] == 'w');
        printf("%s\n", match ? " ✅" : " ❌");
        
        free(output);
    } else {
        printf("(empty) ❌\n");
    }
    
    // Test dog -> bark
    printf("Test 2: 'dog' -> ");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"dog", 3);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        output[read] = '\0';
        
        printf("'");
        for (size_t i = 0; i < read && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf(".");
            }
        }
        printf("'");
        
        int match = (read >= 5 && output[0] == ' ' && output[1] == 'b' && 
                     output[2] == 'a' && output[3] == 'r' && output[4] == 'k');
        printf("%s\n", match ? " ✅" : " ❌");
        
        free(output);
    } else {
        printf("(empty) ❌\n");
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
