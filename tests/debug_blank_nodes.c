/*
 * DEBUG: Understanding Blank Node Creation
 * 
 * Brain analog: Pattern separation in dentate gyrus
 * - When same input leads to different outputs based on context
 * - Brain creates sparse, orthogonal representations
 * 
 * Current requirement: node->incoming_count >= 3 && node->outgoing_count >= 3
 * This might be too restrictive for fast learning!
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING BLANK NODE CREATION                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    remove("/tmp/debug_blank.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/debug_blank.m");
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    printf("\n=== Training patterns with shared prefix ===\n");
    printf("Pattern 1: 'the cat sat'\n");
    printf("Pattern 2: 'the dog ran'\n");
    printf("Pattern 3: 'the bird flew'\n");
    printf("\nThe 't', 'h', 'e', ' ' nodes should have high variability\n");
    printf("because they lead to 'c', 'd', and 'b'\n\n");
    
    // Train each pattern multiple times
    for (int iter = 1; iter <= 20; iter++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the cat sat", 11);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the dog ran", 11);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the bird flew", 13);
        melvin_m_universal_output_clear(mfile);
        
        if (iter == 1 || iter == 5 || iter == 10 || iter == 20) {
            printf("After %d iterations:\n", iter);
            printf("  Nodes: %zu, Edges: %zu\n",
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\n=== Testing Discrimination ===\n");
    
    // Test each prefix
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the c", 5);
    size_t len1 = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = malloc(len1 + 1);
    melvin_m_universal_output_read(mfile, out1, len1);
    out1[len1] = '\0';
    printf("'the c' -> '%.*s'\n", (int)(len1 > 20 ? 20 : len1), out1);
    free(out1);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the d", 5);
    size_t len2 = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = malloc(len2 + 1);
    melvin_m_universal_output_read(mfile, out2, len2);
    out2[len2] = '\0';
    printf("'the d' -> '%.*s'\n", (int)(len2 > 20 ? 20 : len2), out2);
    free(out2);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the b", 5);
    size_t len3 = melvin_m_universal_output_size(mfile);
    uint8_t *out3 = malloc(len3 + 1);
    melvin_m_universal_output_read(mfile, out3, len3);
    out3[len3] = '\0';
    printf("'the b' -> '%.*s'\n", (int)(len3 > 20 ? 20 : len3), out3);
    free(out3);
    
    printf("\n=== Analysis ===\n");
    printf("If outputs are different, context-based discrimination is working\n");
    printf("(via ContextTags, not necessarily blank nodes)\n");
    
    melvin_m_close(mfile);
    remove("/tmp/debug_blank.m");
    
    return 0;
}
