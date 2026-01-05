#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Hierarchy Consolidation Debug Test ===\n\n");
    
    // Create Melvin instance
    MFile *mfile = melvin_m_create("test_consolidation_debug.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create MFile\n");
        return 1;
    }
    
    // Train on "hello" 1000 times
    const char *input = "hello";
    printf("Training on '%s' 1000 times...\n\n", input);
    
    for (int i = 0; i < 1000; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t *)input, strlen(input));
        melvin_m_process_input(mfile);
    }
    
    // Get node and edge counts
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    
    printf("Final: %zu nodes, %zu edges\n\n", node_count, edge_count);
    
    // List all hierarchies with their usage stats
    printf("Hierarchies (abstraction_level > 0):\n");
    printf("%-20s %-6s %-12s %-12s %-10s\n", 
           "Pattern", "Level", "Activations", "InWeight", "Usage Ratio");
    printf("%-20s %-6s %-12s %-12s %-10s\n", 
           "-------", "-----", "-----------", "--------", "-----------");
    
    // Access graph internals (we need to expose this or use a debug API)
    // For now, let's just test output to see if it's working
    
    printf("\nTesting output generation:\n");
    
    // Test 1: "hel" should output "lo"
    const char *test1 = "hel";
    melvin_m_universal_input_write(mfile, (const uint8_t *)test1, strlen(test1));
    melvin_m_process_input(mfile);
    
    uint8_t output1[100];
    size_t out_len1 = melvin_m_universal_output_read(mfile, output1, 100);
    
    printf("  Input: '%s' -> Output: '", test1);
    for (size_t i = 0; i < out_len1 && i < 20; i++) {
        printf("%c", output1[i]);
    }
    printf("' (%zu bytes)\n", out_len1);
    
    // Test 2: "hell" should output "o"
    const char *test2 = "hell";
    melvin_m_universal_input_write(mfile, (const uint8_t *)test2, strlen(test2));
    melvin_m_process_input(mfile);
    
    uint8_t output2[100];
    size_t out_len2 = melvin_m_universal_output_read(mfile, output2, 100);
    
    printf("  Input: '%s' -> Output: '", test2);
    for (size_t i = 0; i < out_len2 && i < 20; i++) {
        printf("%c", output2[i]);
    }
    printf("' (%zu bytes)\n", out_len2);
    
    // Test 3: "he" should output "llo"
    const char *test3 = "he";
    melvin_m_universal_input_write(mfile, (const uint8_t *)test3, strlen(test3));
    melvin_m_process_input(mfile);
    
    uint8_t output3[100];
    size_t out_len3 = melvin_m_universal_output_read(mfile, output3, 100);
    
    printf("  Input: '%s' -> Output: '", test3);
    for (size_t i = 0; i < out_len3 && i < 20; i++) {
        printf("%c", output3[i]);
    }
    printf("' (%zu bytes)\n", out_len3);
    
    melvin_m_close(mfile);
    
    return 0;
}

