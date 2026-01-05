#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Hierarchy Debug Test ===\n\n");
    
    const char *test_file = "test_hierarchy_debug.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Training on '%s' 100 times...\n", pattern);
    for (int i = 0; i < 100; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    printf("Now testing with input 'hel'...\n");
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
    
    // Check if hierarchy nodes exist
    printf("Checking for hierarchy nodes...\n");
    printf("(Hierarchy nodes have abstraction_level > 0 and payload_size > 1)\n\n");
    
    // We need to access the internal graph structure
    // Since we can't access it directly, let's check if hierarchies were created
    // by looking at node count growth
    
    printf("Analysis:\n");
    printf("- If hierarchies exist: Graph should have multi-byte payload nodes\n");
    printf("- If find_active_hierarchy() works: Should find 'hello' when checking 'hel'\n");
    printf("- If hierarchy guidance works: Should output 'lo'\n\n");
    
    printf("Actual output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("'\n");
    
    if (out_len == 2 && output[0] == 'l' && output[1] == 'o') {
        printf("✓ SUCCESS! Hierarchy guidance is working!\n");
    } else {
        printf("✗ FAILED! Hierarchy guidance NOT working\n");
        printf("\nPossible issues:\n");
        printf("1. Hierarchies not being created (check wave_compute_hierarchy_probability)\n");
        printf("2. find_active_hierarchy() not finding them (check matching logic)\n");
        printf("3. Hierarchy guidance not being followed (check edge selection)\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

