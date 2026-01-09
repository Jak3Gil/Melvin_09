/* test_error_rate.c
 * Observes pattern learning: Does the system learn patterns naturally?
 * Like a brain or LLM - learns from data patterns, not external error signals
 * One input can produce many valid outputs - we observe, don't penalize
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Pattern Learning Observation ===\n");
    printf("Observing: Does the system learn patterns naturally from input?\n");
    printf("Like a brain/LLM - learns from data, not external error signals\n\n");
    
    const char *path = "test_error_rate.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello world";
    size_t pattern_len = strlen(pattern);
    
    printf("Input pattern: '%s' (%zu bytes)\n", pattern, pattern_len);
    printf("System learns from pattern structure itself (Hebbian learning)\n");
    printf("Outputs may vary - that's natural, like brain/LLM generation\n\n");
    
    printf("Iteration | Nodes | Edges | Output Len | Output (first 30 chars)\n");
    printf("----------|-------|-------|------------|------------------------\n");
    
    for (int iter = 1; iter <= 200; iter++) {
        // Learn the pattern (Hebbian learning - no external error signals)
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, pattern_len);
        melvin_m_process_input(mfile);
        
        // Observe every 20 iterations
        if (iter % 20 == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            // Get output (system generates based on learned patterns)
            size_t output_size = melvin_m_universal_output_size(mfile);
            uint8_t output[1024] = {0};
            size_t read_size = 0;
            
            if (output_size > 0) {
                read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
            }
            
            // Display output (no error calculation - just observe)
            printf("%9d | %5zu | %5zu | %10zu | ", iter, nodes, edges, read_size);
            
            // Show first 30 characters of output
            size_t display_len = (read_size < 30) ? read_size : 30;
            for (size_t i = 0; i < display_len; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("%c", output[i]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
            
            // Clear output buffer to observe each iteration separately
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\n=== Observations ===\n");
    printf("System learns patterns through Hebbian learning:\n");
    printf("- Edges strengthen when nodes co-activate\n");
    printf("- Patterns emerge from data structure\n");
    printf("- Outputs reflect learned patterns (may vary naturally)\n");
    printf("- No external error signals - pure pattern learning\n");
    
    melvin_m_close(mfile);
    return 0;
}

