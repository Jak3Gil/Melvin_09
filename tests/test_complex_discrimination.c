/* Test: Complex Pattern Discrimination with Adaptive Context
 * 
 * Tests if adaptive context can distinguish:
 * - "the cat" vs "the dog" (shared prefix)
 * - "cat sat" vs "cat ran" (shared prefix)
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== Adaptive Context: Complex Pattern Discrimination ===\n\n");
    
    remove("test_discrimination.m");
    MelvinMFile *mfile = melvin_m_create("test_discrimination.m");
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Train two patterns with shared prefix
    printf("Training patterns:\n");
    printf("  1. 'the cat sat'\n");
    printf("  2. 'the dog ran'\n\n");
    
    for (int i = 0; i < 100; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)"the cat sat", 11);
        melvin_m_universal_output_clear(mfile);
        
        melvin_in_port_process_device(mfile, 0, (uint8_t*)"the dog ran", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("After 100 iterations:\n\n");
    
    // Test 1: "the cat"
    melvin_in_port_process_device(mfile, 0, (uint8_t*)"the cat", 7);
    size_t len1 = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = malloc(len1 + 1);
    melvin_m_universal_output_read(mfile, out1, len1);
    out1[len1] = '\0';
    printf("Input: 'the cat' → Output: '%.*s' (expected: 'sat')\n", 
           (int)(len1 > 20 ? 20 : len1), out1);
    free(out1);
    melvin_m_universal_output_clear(mfile);
    
    // Test 2: "the dog"
    melvin_in_port_process_device(mfile, 0, (uint8_t*)"the dog", 7);
    size_t len2 = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = malloc(len2 + 1);
    melvin_m_universal_output_read(mfile, out2, len2);
    out2[len2] = '\0';
    printf("Input: 'the dog' → Output: '%.*s' (expected: 'ran')\n",
           (int)(len2 > 20 ? 20 : len2), out2);
    free(out2);
    melvin_m_universal_output_clear(mfile);
    
    // Stats
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nGraph: %zu nodes, %zu edges\n", nodes, edges);
    
    printf("\n=== Analysis ===\n");
    printf("If outputs are different: Adaptive context CAN discriminate ✅\n");
    printf("If outputs are same: Context still too weak ⚠️\n");
    
    melvin_m_close(mfile);
    return 0;
}
