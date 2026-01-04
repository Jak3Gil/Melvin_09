#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== 1000 Iteration Test ===\n\n");
    
    const char *test_file = "test_1000.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    for (int i = 1; i <= 1000; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i % 100 == 0) {
            printf("Iteration %d: %zu nodes, %zu edges\n", 
                   i,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\nFinal: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test output
    printf("Test: Input 'hel' → Expected 'lo'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hel", 3);
    melvin_m_process_input(mfile);
    
    uint8_t output[256];
    size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("Output: '");
    for (size_t i = 0; i < out_len; i++) {
        printf("%c", output[i]);
    }
    printf("' (%zu bytes)\n", out_len);
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
