#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Detailed Hierarchy Debug ===\n\n");
    
    // Create Melvin instance
    MelvinMFile *mfile = melvin_m_create("test_hierarchy_detailed.m");
    if (!mfile) {
        printf("Failed to create Melvin instance\n");
        return 1;
    }
    
    // Train on "hello" many times
    const char *pattern = "hello";
    printf("Training on '%s' 200 times...\n", pattern);
    
    for (int i = 0; i < 200; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if ((i + 1) % 50 == 0) {
            printf("  After %d iterations: %zu nodes, %zu edges\n", 
                   i + 1,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\n=== Testing Output Generation ===\n\n");
    
    // Test 1: Input "hel" → Expected "lo"
    const char *test_input = "hel";
    printf("Test Input: '%s'\n", test_input);
    printf("Expected Output: 'lo'\n\n");
    
    // Write input and process
    melvin_m_universal_input_write(mfile, (const uint8_t*)test_input, strlen(test_input));
    melvin_m_process_input(mfile);
    
    // Generate output
    uint8_t output_buffer[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output_buffer, sizeof(output_buffer));
    
    printf("Actual Output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output_buffer[i]);
    }
    printf("' (%zu bytes)\n\n", output_len);
    
    // Check if correct
    const char *expected = "lo";
    int correct = (output_len == strlen(expected) && 
                   memcmp(output_buffer, expected, output_len) == 0);
    
    if (correct) {
        printf("✓ SUCCESS! Output matches expected\n");
    } else {
        printf("✗ FAILED! Output doesn't match\n");
        printf("\nDiagnostics:\n");
        printf("- Output length: %zu (expected: %zu)\n", output_len, strlen(expected));
        printf("- First byte: '%c' (expected: '%c')\n", 
               output_len > 0 ? output_buffer[0] : '?',
               expected[0]);
        if (output_len > 1) {
            printf("- Second byte: '%c' (expected: '%c')\n", output_buffer[1], expected[1]);
        } else {
            printf("- Second byte: (none) (expected: '%c')\n", expected[1]);
        }
    }
    
    // Test 2: Input "h" → Expected "ello"
    printf("\n=== Test 2 ===\n");
    const char *test_input2 = "h";
    printf("Test Input: '%s'\n", test_input2);
    printf("Expected Output: 'ello'\n\n");
    
    melvin_m_universal_input_write(mfile, (const uint8_t*)test_input2, strlen(test_input2));
    melvin_m_process_input(mfile);
    
    output_len = melvin_m_universal_output_read(mfile, output_buffer, sizeof(output_buffer));
    
    printf("Actual Output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output_buffer[i]);
    }
    printf("' (%zu bytes)\n\n", output_len);
    
    const char *expected2 = "ello";
    correct = (output_len == strlen(expected2) && 
               memcmp(output_buffer, expected2, output_len) == 0);
    
    if (correct) {
        printf("✓ SUCCESS!\n");
    } else {
        printf("✗ FAILED!\n");
    }
    
    // Test 3: Input "he" → Expected "llo"
    printf("\n=== Test 3 ===\n");
    const char *test_input3 = "he";
    printf("Test Input: '%s'\n", test_input3);
    printf("Expected Output: 'llo'\n\n");
    
    melvin_m_universal_input_write(mfile, (const uint8_t*)test_input3, strlen(test_input3));
    melvin_m_process_input(mfile);
    
    output_len = melvin_m_universal_output_read(mfile, output_buffer, sizeof(output_buffer));
    
    printf("Actual Output: '");
    for (size_t i = 0; i < output_len; i++) {
        printf("%c", output_buffer[i]);
    }
    printf("' (%zu bytes)\n\n", output_len);
    
    const char *expected3 = "llo";
    correct = (output_len == strlen(expected3) && 
               memcmp(output_buffer, expected3, output_len) == 0);
    
    if (correct) {
        printf("✓ SUCCESS!\n");
    } else {
        printf("✗ FAILED!\n");
    }
    
    // Cleanup
    melvin_m_close(mfile);
    remove("test_hierarchy_detailed.m");
    
    return 0;
}

