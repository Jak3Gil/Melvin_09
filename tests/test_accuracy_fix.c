/*
 * Test accuracy after hierarchy activation fixes
 * Trains on patterns multiple times, then tests generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("=== Accuracy Test After Hierarchy Activation Fix ===\n\n");
    
    const char *mfile_path = "test_accuracy_fix.m";
    remove(mfile_path);
    
    MelvinMFile *mfile = melvin_m_create(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to create .m file\n");
        return 1;
    }
    
    // Test 1: Simple pattern "hello"
    printf("=== Test 1: Training 'hello' (20 iterations) ===\n");
    const char *pattern1 = "hello";
    for (int i = 0; i < 20; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern1, strlen(pattern1));
        melvin_m_process_input(mfile);
    }
    
    printf("Graph stats: %zu nodes, %zu edges\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test generation
    melvin_m_universal_input_write(mfile, (const uint8_t*)pattern1, strlen(pattern1));
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t output[1024];
    if (output_size > 0) {
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Input: '%s'\n", pattern1);
        printf("Output (%zu bytes): ", read_size);
        for (size_t i = 0; i < read_size && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("\\x%02x", output[i]);
            }
        }
        printf("\n");
        
        // Check if output is reasonable
        int has_reasonable_output = 0;
        for (size_t i = 0; i < read_size; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                has_reasonable_output = 1;
                break;
            }
        }
        printf("Has reasonable output: %s\n\n", has_reasonable_output ? "YES" : "NO");
    } else {
        printf("No output generated!\n\n");
    }
    
    // Test 2: Association "hello" -> "world"
    printf("=== Test 2: Training associations (10 iterations) ===\n");
    const char *patterns[] = {"hello", "world", "cat", "dog", "apple"};
    size_t pattern_count = sizeof(patterns) / sizeof(patterns[0]);
    
    for (int iter = 0; iter < 10; iter++) {
        for (size_t p = 0; p < pattern_count; p++) {
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_process_input(mfile);
        }
    }
    
    printf("Graph stats: %zu nodes, %zu edges\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test generation for "hello"
    printf("\nTesting input 'hello':\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Output (%zu bytes): ", read_size);
        for (size_t i = 0; i < read_size && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("\\x%02x", output[i]);
            }
        }
        printf("\n");
    } else {
        printf("No output generated!\n");
    }
    
    // Test 3: Multi-character input
    printf("\n=== Test 3: Multi-character input 'hel' ===\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hel", 3);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Input: 'hel'\n");
        printf("Output (%zu bytes): ", read_size);
        for (size_t i = 0; i < read_size && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("\\x%02x", output[i]);
            }
        }
        printf("\n");
        
        // Check if output contains 'l' or 'o' (continuation of "hel")
        int has_continuation = 0;
        for (size_t i = 0; i < read_size; i++) {
            if (output[i] == 'l' || output[i] == 'o') {
                has_continuation = 1;
                break;
            }
        }
        printf("Has continuation ('l' or 'o'): %s\n", has_continuation ? "YES" : "NO");
    } else {
        printf("No output generated!\n");
    }
    
    melvin_m_close(mfile);
    remove(mfile_path);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
