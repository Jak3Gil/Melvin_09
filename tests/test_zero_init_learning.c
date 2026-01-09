/* Test: Zero Initialization Learning Curve
 * 
 * Tests how zero-initialized weights learn over many iterations
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== Zero Init Learning Curve ===\n\n");
    
    remove("test_zero_learning.m");
    MelvinMFile *mfile = melvin_m_create("test_zero_learning.m");
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    printf("Training: 'hello world'\n");
    printf("Testing:  'hello ' → expect 'world'\n\n");
    
    // Test at different iteration counts
    int test_points[] = {10, 50, 100, 200, 500, 1000};
    int num_tests = 6;
    
    for (int t = 0; t < num_tests; t++) {
        int target = test_points[t];
        int current = (t == 0) ? 0 : test_points[t-1];
        
        // Train more iterations
        for (int i = current; i < target; i++) {
            melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello world", 11);
            melvin_m_universal_output_clear(mfile);
        }
        
        // Test
        melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello ", 6);
        size_t len = melvin_m_universal_output_size(mfile);
        uint8_t *out = malloc(len + 1);
        melvin_m_universal_output_read(mfile, out, len);
        out[len] = '\0';
        
        // Compute error
        const char *expected = "world";
        int errors = 0;
        size_t exp_len = strlen(expected);
        size_t cmp_len = (len < exp_len) ? len : exp_len;
        for (size_t i = 0; i < cmp_len; i++) {
            if (out[i] != expected[i]) errors++;
        }
        if (len != exp_len) errors += abs((int)len - (int)exp_len);
        float error_rate = (exp_len > 0) ? (100.0f * errors / exp_len) : 0.0f;
        
        printf("Iter %4d: '%.*s' (%.1f%% error)\n", 
               target, (int)(len > 20 ? 20 : len), out, error_rate);
        
        free(out);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nFinal graph: %zu nodes, %zu edges\n", nodes, edges);
    
    melvin_m_close(mfile);
    return 0;
}
