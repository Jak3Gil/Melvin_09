/*
 * DEBUG: Understanding Generation Flow
 * 
 * Why does "hello" → " worldldlor" instead of " world"?
 * 
 * Brain analog: Patterns should have natural endings
 * - Refractory period prevents re-activation
 * - Context should guide towards completion, not looping
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING GENERATION LOOP                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_gen.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_gen.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Train with clear termination
    printf("\n=== Training 'hello world' with clear end ===\n");
    
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After training:\n");
    printf("  Nodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test generation
    printf("\n=== Testing 'hello' ===\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    printf("Output (%zu bytes): '", out_size);
    for (size_t i = 0; i < out_size && i < 50; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("'\n");
    
    // Analyze the output
    printf("\n=== Analysis ===\n");
    printf("Expected: ' world' (6 bytes)\n");
    printf("Got: %zu bytes\n", out_size);
    
    // Check for patterns
    int has_repeating = 0;
    for (size_t i = 0; i + 1 < out_size; i++) {
        if (output[i] == output[i+1]) {
            // Consecutive same char
        }
        // Check for 2-char repeating pattern
        if (i + 3 < out_size && 
            output[i] == output[i+2] && 
            output[i+1] == output[i+3]) {
            has_repeating = 1;
            printf("  Found repeating pattern at position %zu: '%c%c'\n",
                   i, output[i], output[i+1]);
        }
    }
    
    if (has_repeating) {
        printf("\n⚠ LOOPING DETECTED - cycle detection not working properly\n");
    } else if (out_size > 10) {
        printf("\n⚠ OUTPUT TOO LONG - stopping conditions too weak\n");
    } else if (out_size <= 6 && memcmp(output, " world", 6) == 0) {
        printf("\n✓ PERFECT OUTPUT!\n");
    }
    
    free(output);
    melvin_m_close(mfile);
    remove("/tmp/debug_gen.m");
    
    return 0;
}
