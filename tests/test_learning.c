#include <stdio.h>
#include <string.h>
#include "src/melvin.h"

int main() {
    printf("=== Testing Organic Learning ===\n\n");
    
    // Create brain
    MelvinMFile *brain = melvin_m_create("test_learning.m");
    if (!brain) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    printf("✓ Brain created\n\n");
    
    // Train on pattern multiple times
    const char *pattern = "hello";
    
    for (int iter = 1; iter <= 5; iter++) {
        printf("=== Iteration %d ===\n", iter);
        printf("Training: '%s'\n", pattern);
        
        melvin_m_universal_input_write(brain, (const uint8_t*)pattern, strlen(pattern));
        int result = melvin_m_process_input(brain);
        
        printf("Process result: %d\n", result);
        printf("Nodes: %zu, Edges: %zu\n", 
               melvin_m_get_node_count(brain), 
               melvin_m_get_edge_count(brain));
        
        // Check output
        size_t out_size = melvin_m_universal_output_size(brain);
        if (out_size > 0) {
            uint8_t out_buf[256];
            size_t read = melvin_m_universal_output_read(brain, out_buf, sizeof(out_buf));
            printf("Output (%zu bytes): '", read);
            for (size_t j = 0; j < read && j < 50; j++) {
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
        printf("\n");
    }
    
    // Test generation
    printf("=== Generation Test ===\n");
    const char *prompt = "hel";
    printf("Prompt: '%s'\n", prompt);
    
    melvin_m_universal_input_write(brain, (const uint8_t*)prompt, strlen(prompt));
    int result = melvin_m_process_input(brain);
    
    printf("Result: %d\n", result);
    
    size_t out_size = melvin_m_universal_output_size(brain);
    if (out_size > 0) {
        uint8_t out_buf[256];
        size_t read = melvin_m_universal_output_read(brain, out_buf, sizeof(out_buf));
        printf("Output (%zu bytes): '", read);
        for (size_t j = 0; j < read && j < 50; j++) {
            if (out_buf[j] >= 32 && out_buf[j] < 127) {
                printf("%c", out_buf[j]);
            } else {
                printf("<%02x>", out_buf[j]);
            }
        }
        printf("'\n");
    }
    
    // Cleanup
    melvin_m_close(brain);
    printf("\n✓ Test completed!\n");
    
    return 0;
}
