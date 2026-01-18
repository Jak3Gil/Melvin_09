/* Test simple association learning */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Simple Association Test ===\n\n");
    
    const char *test_file = "test_simple_assoc.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create mfile\n");
        return 1;
    }
    
    // Train on "cat meow" multiple times
    const char *pattern = "cat meow";
    printf("Training on pattern '%s' 10 times...\n", pattern);
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test generation from "cat"
    printf("\nTesting generation from 'cat':\n");
    
    // Write input
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    
    // Read output
    size_t output_size = melvin_m_universal_output_size(mfile);
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
    
    printf("Input: 'cat'\n");
    printf("Output: '");
    for (size_t i = 0; i < output_size && i < 20; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("?");
        }
    }
    printf("'\n");
    printf("Expected: ' meow'\n");
    
    // Check if it predicted ' meow'
    const char *expected = " meow";
    int success = 1;
    if (output_size < strlen(expected)) {
        success = 0;
    } else {
        for (size_t i = 0; i < strlen(expected); i++) {
            if (output[i] != expected[i]) {
                success = 0;
                break;
            }
        }
    }
    
    printf("\nResult: %s\n", success ? "✅ PASS" : "❌ FAIL");
    
    free(output);
    melvin_m_close(mfile);
    unlink(test_file);
    
    return success ? 0 : 1;
}
