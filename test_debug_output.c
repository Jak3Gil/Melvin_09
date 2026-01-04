#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main() {
    MelvinMFile *mfile = melvin_m_create("debug_test.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create\n");
        return 1;
    }
    
    const char *input = "hello";
    melvin_m_universal_input_write(mfile, (const uint8_t*)input, strlen(input));
    
    printf("Before process: nodes=%zu edges=%zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    int result = melvin_m_process_input(mfile);
    
    printf("After process: nodes=%zu edges=%zu result=%d\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile), result);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu\n", output_size);
    
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        printf("Read %zu bytes: ", read);
        for (size_t i = 0; i < read; i++) {
            printf("%c", output[i]);
        }
        printf("\n");
        free(output);
    }
    
    melvin_m_close(mfile);
    return 0;
}
