#include <stdio.h>
#include <string.h>
#include "src/melvin.h"

int main() {
    printf("=== Testing Organic System ===\n\n");
    
    // Create brain
    MelvinMFile *brain = melvin_m_create("test_organic.m");
    if (!brain) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    printf("✓ Brain created\n");
    
    // Train on simple patterns
    const char *patterns[] = {
        "hello",
        "hello",
        "hello",
        "world",
        "world",
        "hello world"
    };
    
    printf("\n=== Training Phase ===\n");
    for (int i = 0; i < 6; i++) {
        printf("Training %d: '%s'\n", i+1, patterns[i]);
        
        // Write input
        melvin_m_universal_input_write(brain, (const uint8_t*)patterns[i], strlen(patterns[i]));
        
        // Process
        int result = melvin_m_process_input(brain);
        printf("  Process result: %d\n", result);
        
        // Check output
        size_t out_size = melvin_m_universal_output_size(brain);
        if (out_size > 0) {
            uint8_t out_buf[256];
            size_t read = melvin_m_universal_output_read(brain, out_buf, sizeof(out_buf));
            printf("  Output (%zu bytes): '", read);
            for (size_t j = 0; j < read && j < 20; j++) {
                if (out_buf[j] >= 32 && out_buf[j] < 127) {
                    printf("%c", out_buf[j]);
                } else {
                    printf("<%02x>", out_buf[j]);
                }
            }
            printf("'\n");
        }
        
        melvin_m_universal_input_clear(brain);
        melvin_m_universal_output_clear(brain);
    }
    
    printf("\n=== Generation Test ===\n");
    
    // Test generation with "hel" prompt
    const char *prompt = "hel";
    printf("Prompt: '%s'\n", prompt);
    
    melvin_m_universal_input_write(brain, (const uint8_t*)prompt, strlen(prompt));
    int result = melvin_m_process_input(brain);
    
    printf("Result: %d\n", result);
    
    size_t output_len = melvin_m_universal_output_size(brain);
    printf("Output length: %zu\n", output_len);
    
    if (output_len > 0) {
        uint8_t output[256];
        size_t read = melvin_m_universal_output_read(brain, output, sizeof(output));
        printf("Output: '");
        for (size_t i = 0; i < read && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("<%02x>", output[i]);
            }
        }
        printf("'\n");
    }
    
    // Get stats
    printf("\n=== Brain Stats ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(brain));
    printf("Edges: %zu\n", melvin_m_get_edge_count(brain));
    
    // Cleanup
    melvin_m_close(brain);
    printf("\n✓ Test completed successfully!\n");
    
    return 0;
}
