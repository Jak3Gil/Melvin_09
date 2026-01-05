#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Testing: Does More Data Solve It? ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_learning.m");
    
    // Test with 1, 5, 10, 20 repetitions
    int repetitions[] = {1, 5, 10, 20};
    
    for (int r = 0; r < 4; r++) {
        // Fresh file for each test
        if (r > 0) {
            melvin_m_close(mfile);
            remove("test_learning.m");
            mfile = melvin_m_create("test_learning.m");
        }
        
        int reps = repetitions[r];
        printf("Test %d: Learning 'hello' %d time(s)\n", r+1, reps);
        
        for (int i = 0; i < reps; i++) {
            melvin_m_universal_input_write(mfile, (const uint8_t*)"Xhello", 6);
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
            melvin_m_universal_input_clear(mfile);
        }
        
        printf("  Graph: %zu nodes, %zu edges\n", 
               melvin_m_get_node_count(mfile), 
               melvin_m_get_edge_count(mfile));
        
        // Generate from 'h'
        melvin_m_universal_input_write(mfile, (const uint8_t*)"Xh", 2);
        melvin_m_process_input(mfile);
        
        uint8_t output[256];
        size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        printf("  Output: '");
        for (size_t i = 0; i < output_len; i++) {
            printf("%c", output[i]);
        }
        printf("' (%zu bytes)\n", output_len);
        
        // Check if it's correct
        int correct = (output_len == 4 && 
                      output[0] == 'e' && 
                      output[1] == 'l' && 
                      output[2] == 'l' && 
                      output[3] == 'o');
        printf("  Result: %s\n\n", correct ? "✓ CORRECT" : "✗ WRONG");
    }
    
    melvin_m_close(mfile);
    return 0;
}

