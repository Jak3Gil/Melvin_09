/*
 * DEBUG: Understanding Fast Learning
 * 
 * Tests learning at 1, 2, 3, 5, 10 iterations to understand:
 * 1. When does association actually form?
 * 2. Are hierarchies forming?
 * 3. Are blank nodes forming?
 * 4. How does context matching work?
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_at_iteration_count(int iterations) {
    char filename[64];
    snprintf(filename, sizeof(filename), "/tmp/debug_iter_%d.m", iterations);
    remove(filename);
    
    MelvinMFile *mfile = melvin_m_create(filename);
    if (!mfile) {
        printf("Failed to create brain\n");
        return;
    }
    
    printf("\n========================================\n");
    printf("Testing with %d iteration(s)\n", iterations);
    printf("========================================\n");
    
    // Train "hello world"
    const char *training = "hello world";
    for (int i = 0; i < iterations; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)training, strlen(training));
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After training:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    // Test with just "hello"
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        // Check for 'w' in output
        int has_w = 0;
        for (size_t i = 0; i < out_size; i++) {
            if (output[i] == 'w') has_w = 1;
        }
        
        printf("  Input 'hello' -> Output '%.*s' (%zu bytes)\n", 
               (int)(out_size > 30 ? 30 : out_size), output, out_size);
        printf("  Contains 'w': %s\n", has_w ? "YES ✓" : "NO ✗");
        
        free(output);
    } else {
        printf("  Input 'hello' -> No output generated\n");
    }
    
    melvin_m_close(mfile);
    remove(filename);
}

void test_discrimination_at_iterations(int iterations) {
    char filename[64];
    snprintf(filename, sizeof(filename), "/tmp/debug_discrim_%d.m", iterations);
    remove(filename);
    
    MelvinMFile *mfile = melvin_m_create(filename);
    if (!mfile) return;
    
    printf("\n========================================\n");
    printf("Discrimination test: %d iteration(s)\n", iterations);
    printf("========================================\n");
    
    // Train two patterns
    for (int i = 0; i < iterations; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cat meow", 8);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"dog bark", 8);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Trained: 'cat meow' and 'dog bark'\n");
    printf("  Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // Test "cat"
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cat", 3);
    size_t len1 = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = malloc(len1 + 1);
    melvin_m_universal_output_read(mfile, out1, len1);
    out1[len1] = '\0';
    
    // Test "dog"
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"dog", 3);
    size_t len2 = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = malloc(len2 + 1);
    melvin_m_universal_output_read(mfile, out2, len2);
    out2[len2] = '\0';
    
    int has_m = 0, has_b = 0;
    for (size_t i = 0; i < len1; i++) if (out1[i] == 'm') has_m = 1;
    for (size_t i = 0; i < len2; i++) if (out2[i] == 'b') has_b = 1;
    
    printf("  'cat' -> '%.*s' (contains 'm': %s)\n", 
           (int)(len1 > 20 ? 20 : len1), out1, has_m ? "YES ✓" : "NO");
    printf("  'dog' -> '%.*s' (contains 'b': %s)\n",
           (int)(len2 > 20 ? 20 : len2), out2, has_b ? "YES ✓" : "NO");
    printf("  Outputs different: %s\n", 
           (len1 != len2 || memcmp(out1, out2, len1) != 0) ? "YES ✓" : "NO ✗");
    
    free(out1);
    free(out2);
    melvin_m_close(mfile);
    remove(filename);
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         DEBUGGING FAST LEARNING (2-5 iterations)          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    printf("\n=== PART 1: Simple Association Learning ===\n");
    test_at_iteration_count(1);
    test_at_iteration_count(2);
    test_at_iteration_count(3);
    test_at_iteration_count(5);
    test_at_iteration_count(10);
    
    printf("\n\n=== PART 2: Discrimination Learning ===\n");
    test_discrimination_at_iterations(1);
    test_discrimination_at_iterations(2);
    test_discrimination_at_iterations(3);
    test_discrimination_at_iterations(5);
    test_discrimination_at_iterations(10);
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    DEBUG COMPLETE                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
