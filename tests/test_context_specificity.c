/*
 * Context Specificity Test
 * Tests if context matching is specific enough when there are many patterns
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Context Specificity Test ===\n");
    printf("Testing if context matching is specific enough\n\n");
    
    const char *test_file = "test_context_specificity.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // Test: Learn two similar but different patterns
    // "hello world" and "hello there"
    // Query "hello" should distinguish between them based on context
    
    printf("Learning 'hello world' (10 times)...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)"hello world", 11);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Learning 'hello there' (10 times)...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)"hello there", 11);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph size: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test 1: Query "hello" - should output either " world" or " there"
    printf("Test 1: Query 'hello' (should output ' world' or ' there')\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_size + 1);
    if (output && output_size > 0) {
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        if (read > output_size) read = output_size;
        if (read > 0 && read <= output_size) {
            output[read] = '\0';
        } else {
            read = 0;
            output[0] = '\0';
        }
        
        printf("Output: '%.*s' (%zu bytes)\n", (int)read, output, read);
        
        // Check if it matches either expected pattern
        int matches_world = (read >= 6 && memcmp(output, " world", 6) == 0);
        int matches_there = (read >= 6 && memcmp(output, " there", 6) == 0);
        
        if (matches_world || matches_there) {
            printf("✅ PASSED: Output matches one of the learned patterns\n");
        } else {
            printf("⚠️  Output doesn't match expected patterns\n");
        }
        
        free(output);
    }
    melvin_m_universal_output_clear(mfile);
    
    // Test 2: Learn more patterns and see if accuracy degrades
    printf("\nTest 2: Learning more patterns to test scalability...\n");
    
    const char *patterns[] = {
        "test testing",
        "cat meow",
        "dog bark",
        "red apple",
        "blue sky"
    };
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 10; j++) {
            melvin_m_universal_input_write(mfile, 
                (const uint8_t*)patterns[i], 
                strlen(patterns[i]));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("Graph size after more patterns: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test "hello" again - should still work
    printf("Test 3: Query 'hello' again (after learning more patterns)\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    output = malloc(output_size + 1);
    if (output && output_size > 0) {
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        if (read > output_size) read = output_size;
        if (read > 0 && read <= output_size) {
            output[read] = '\0';
        } else {
            read = 0;
            output[0] = '\0';
        }
        
        printf("Output: '%.*s' (%zu bytes)\n", (int)read, output, read);
        
        int matches_world = (read >= 6 && memcmp(output, " world", 6) == 0);
        int matches_there = (read >= 6 && memcmp(output, " there", 6) == 0);
        
        if (matches_world || matches_there) {
            printf("✅ PASSED: Still accurate after learning more patterns\n");
        } else {
            printf("⚠️  Accuracy degraded after learning more patterns\n");
        }
        
        free(output);
    }
    melvin_m_universal_output_clear(mfile);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
