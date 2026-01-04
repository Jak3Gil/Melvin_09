/*
 * Debug output generation to understand why non-printable bytes are selected
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain_file.m> <input_file>\n", argv[0]);
        return 1;
    }
    
    const char *brain_file = argv[1];
    const char *input_file = argv[2];
    
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        mfile = melvin_m_create(brain_file);
        if (!mfile) {
            fprintf(stderr, "Failed to create brain file\n");
            return 1;
        }
    }
    
    // Process input
    uint8_t port_id = 1;
    melvin_in_port_handle_text_file(mfile, port_id, input_file);
    melvin_m_save(mfile);
    
    // Get output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("Output: ");
            for (size_t i = 0; i < output_size; i++) {
                printf("0x%02x ", output[i]);
            }
            printf("\n");
            free(output);
        }
    }
    
    melvin_m_close(mfile);
    return 0;
}

