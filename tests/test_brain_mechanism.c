#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Brain-Like Mechanism Test ===\n\n");
    
    // Create fresh .m file
    const char *test_file = "test_brain.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    printf("Phase 1: Learning 'hello' pattern\n");
    printf("-----------------------------------\n");
    
    // Learn "hello" multiple times to build strong connections
    for (int i = 0; i < 50; i++) {
        const char *input = "hello";
        melvin_m_universal_input_write(mfile, (uint8_t*)input, strlen(input));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i % 10 == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            printf("  Iteration %d: %zu nodes, %zu edges\n", i, nodes, edges);
        }
    }
    
    printf("\nPhase 2: Testing output generation\n");
    printf("-----------------------------------\n");
    
    // Test 1: Input "h" -> should output "ello" based on activated context
    printf("\nTest 1: Input 'h'\n");
    const char *test1 = "h";
    melvin_m_universal_input_write(mfile, (uint8_t*)test1, strlen(test1));
    melvin_m_process_input(mfile);
    
    uint8_t output1[256];
    size_t out_len1 = melvin_m_universal_output_read(mfile, output1, sizeof(output1));
    
    printf("  Input:  '%s'\n", test1);
    printf("  Output: '");
    for (size_t i = 0; i < out_len1; i++) {
        printf("%c", output1[i]);
    }
    printf("' (%zu bytes)\n", out_len1);
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Test 2: Input "he" -> should output "llo"
    printf("\nTest 2: Input 'he'\n");
    const char *test2 = "he";
    melvin_m_universal_input_write(mfile, (uint8_t*)test2, strlen(test2));
    melvin_m_process_input(mfile);
    
    uint8_t output2[256];
    size_t out_len2 = melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    
    printf("  Input:  '%s'\n", test2);
    printf("  Output: '");
    for (size_t i = 0; i < out_len2; i++) {
        printf("%c", output2[i]);
    }
    printf("' (%zu bytes)\n", out_len2);
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Test 3: Input "hel" -> should output "lo"
    printf("\nTest 3: Input 'hel'\n");
    const char *test3 = "hel";
    melvin_m_universal_input_write(mfile, (uint8_t*)test3, strlen(test3));
    melvin_m_process_input(mfile);
    
    uint8_t output3[256];
    size_t out_len3 = melvin_m_universal_output_read(mfile, output3, sizeof(output3));
    
    printf("  Input:  '%s'\n", test3);
    printf("  Output: '");
    for (size_t i = 0; i < out_len3; i++) {
        printf("%c", output3[i]);
    }
    printf("' (%zu bytes)\n", out_len3);
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    printf("\nPhase 3: Learning 'world' pattern\n");
    printf("-----------------------------------\n");
    
    // Now learn "world" - test generalization
    for (int i = 0; i < 50; i++) {
        const char *input = "world";
        melvin_m_universal_input_write(mfile, (uint8_t*)input, strlen(input));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i % 10 == 0) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            printf("  Iteration %d: %zu nodes, %zu edges\n", i, nodes, edges);
        }
    }
    
    printf("\nPhase 4: Testing both patterns\n");
    printf("-----------------------------------\n");
    
    // Test 4: Input "h" -> should still output "ello"
    printf("\nTest 4: Input 'h' (after learning 'world')\n");
    melvin_m_universal_input_write(mfile, (uint8_t*)test1, strlen(test1));
    melvin_m_process_input(mfile);
    
    uint8_t output4[256];
    size_t out_len4 = melvin_m_universal_output_read(mfile, output4, sizeof(output4));
    
    printf("  Input:  '%s'\n", test1);
    printf("  Output: '");
    for (size_t i = 0; i < out_len4; i++) {
        printf("%c", output4[i]);
    }
    printf("' (%zu bytes)\n", out_len4);
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    // Test 5: Input "w" -> should output "orld"
    printf("\nTest 5: Input 'w'\n");
    const char *test5 = "w";
    melvin_m_universal_input_write(mfile, (uint8_t*)test5, strlen(test5));
    melvin_m_process_input(mfile);
    
    uint8_t output5[256];
    size_t out_len5 = melvin_m_universal_output_read(mfile, output5, sizeof(output5));
    
    printf("  Input:  '%s'\n", test5);
    printf("  Output: '");
    for (size_t i = 0; i < out_len5; i++) {
        printf("%c", output5[i]);
    }
    printf("' (%zu bytes)\n", out_len5);
    
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_output_clear(mfile);
    
    printf("\n=== Summary ===\n");
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    printf("Final graph: %zu nodes, %zu edges\n", final_nodes, final_edges);
    printf("\nBrain-like mechanism: Decisions based on activated context and graph connectivity\n");
    printf("No hardcoded sequence matching - patterns emerge from learned connections\n");
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

