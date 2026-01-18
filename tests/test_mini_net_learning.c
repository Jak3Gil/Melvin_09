/* Test mini-net learning on simple pattern */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Mini-Net Learning Test ===\n\n");
    
    const char *test_file = "test_mininet.m";
    unlink(test_file);
    
    // Create a simple pattern: "AB"
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create mfile\n");
        return 1;
    }
    
    // Train on "AB" multiple times
    const char *pattern = "AB";
    printf("Training on pattern 'AB' 10 times...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test generation from "A"
    printf("Testing generation from 'A':\n");
    
    // Write input
    melvin_m_universal_input_write(mfile, (const uint8_t*)"A", 1);
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
    
    printf("Input: 'A'\n");
    printf("Output: '");
    for (size_t i = 0; i < output_size && i < 10; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("?");
        }
    }
    printf("'\n");
    
    // Check if it predicted 'B'
    int success = (output_size > 0 && output[0] == 'B');
    printf("\nResult: %s\n", success ? "✅ PASS" : "❌ FAIL");
    
    free(output);
    melvin_m_close(mfile);
    unlink(test_file);
    
    return success ? 0 : 1;
}
