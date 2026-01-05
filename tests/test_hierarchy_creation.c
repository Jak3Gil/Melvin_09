#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Hierarchy Creation Debug ===\n\n");
    printf("Testing if hierarchies are actually being created\n\n");
    
    const char *test_file = "test_hierarchy_create.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Pattern: '%s'\n", pattern);
    printf("Initial: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    printf("Training iterations:\n");
    for (int i = 1; i <= 200; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i % 20 == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            printf("  Iteration %3d: %3zu nodes, %3zu edges", i, nodes, edges);
            
            // Check if we're growing beyond just the 5 byte nodes
            if (nodes > 5) {
                printf(" (hierarchies likely forming!)");
            }
            printf("\n");
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal: %zu nodes, %zu edges\n\n", final_nodes, final_edges);
    
    printf("Analysis:\n");
    printf("- 'hello' has 5 unique bytes: h, e, l, o\n");
    printf("- Minimum nodes: 5 (just the bytes)\n");
    printf("- Actual nodes: %zu\n", final_nodes);
    printf("- Extra nodes: %zu\n", final_nodes > 5 ? final_nodes - 5 : 0);
    
    if (final_nodes > 5) {
        printf("✓ Extra nodes created - hierarchies are forming!\n");
        printf("  Possible hierarchies: 'he', 'el', 'll', 'lo', 'hel', 'ell', 'llo', 'hello'\n");
    } else {
        printf("✗ No extra nodes - hierarchies NOT forming!\n");
        printf("  Problem: wave_compute_hierarchy_probability() not reaching threshold\n");
        printf("  Threshold: 0.6 (60%%)\n");
        printf("  Signals needed: dominance, repetition, maturity, compression\n");
    }
    
    printf("\n=== Testing Output ===\n\n");
    
    printf("Input: 'hel'\n");
    melvin_m_universal_input_write(mfile, (uint8_t*)"hel", 3);
    melvin_m_process_input(mfile);
    
    uint8_t output[256];
    size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", out_len);
    printf("Expected: 'lo'\n\n");
    
    if (out_len == 2 && output[0] == 'l' && output[1] == 'o') {
        printf("✓ PERFECT! System is working!\n");
    } else if (out_len > 0 && (output[0] == 'l' || output[0] == 'o')) {
        printf("~ Partial success - got some correct bytes\n");
    } else {
        printf("✗ FAILED! Wrong output\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

