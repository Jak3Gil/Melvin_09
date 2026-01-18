#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

int main(void) {
    const char *test_file = "test_stop_weight.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) return 1;
    
    const char *pattern = "cat meow";
    
    // Test with different iteration counts
    for (int iterations = 1; iterations <= 10; iterations++) {
        // Recreate brain for each test
        melvin_m_close(mfile);
        unlink(test_file);
        mfile = melvin_m_create(test_file);
        
        printf("=== %d iterations ===\n", iterations);
        
        // Train
        for (int i = 0; i < iterations; i++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        // Test generation
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
        melvin_m_process_input(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("  Output size: %zu\n", output_size);
        
        if (output_size == 0 && iterations > 5) {
            printf("  ⚠️ EMPTY OUTPUT at iteration %d!\n", iterations);
            break;
        }
    }
    
    melvin_m_close(mfile);
    unlink(test_file);
    return 0;
}
