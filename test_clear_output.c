#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    printf("=== Clear Output Test ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_clear.m");
    
    // Learn "hello"
    printf("Step 1: Learning 'hello'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    // Check output from learning
    uint8_t output1[256];
    size_t output1_len = melvin_m_universal_output_read(mfile, output1, sizeof(output1));
    printf("Output from learning: '");
    for (size_t i = 0; i < output1_len; i++) {
        printf("%c", output1[i]);
    }
    printf("' (%zu bytes)\n\n", output1_len);
    
    // CLEAR output buffer
    printf("Step 2: Clearing output buffer\n");
    melvin_m_universal_output_clear(mfile);
    
    // CLEAR input buffer
    melvin_m_universal_input_clear(mfile);
    
    // Now generate from 'h'
    printf("Step 3: Generating from 'h'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output2[256];
    size_t output2_len = melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    printf("Output from 'h': '");
    for (size_t i = 0; i < output2_len; i++) {
        printf("%c", output2[i]);
    }
    printf("' (%zu bytes)\n", output2_len);
    
    melvin_m_close(mfile);
    return 0;
}

