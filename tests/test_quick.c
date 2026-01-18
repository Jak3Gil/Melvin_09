#include <stdio.h>
#include <string.h>
#include "src/melvin.h"

int main() {
    printf("=== Quick Organic System Test ===\n\n");
    
    // Create brain
    MelvinMFile *brain = melvin_m_create("test_quick.m");
    if (!brain) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    printf("✓ Brain created\n");
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(brain), 
           melvin_m_get_edge_count(brain));
    
    // Train on ONE simple pattern
    const char *pattern = "hi";
    printf("\nTraining on: '%s'\n", pattern);
    
    melvin_m_universal_input_write(brain, (const uint8_t*)pattern, strlen(pattern));
    
    printf("Processing...\n");
    fflush(stdout);
    
    int result = melvin_m_process_input(brain);
    
    printf("Process result: %d\n", result);
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(brain), 
           melvin_m_get_edge_count(brain));
    
    // Check output
    size_t out_size = melvin_m_universal_output_size(brain);
    printf("Output size: %zu\n", out_size);
    
    if (out_size > 0) {
        uint8_t out_buf[256];
        size_t read = melvin_m_universal_output_read(brain, out_buf, sizeof(out_buf));
        printf("Output (%zu bytes): '", read);
        for (size_t j = 0; j < read && j < 20; j++) {
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
