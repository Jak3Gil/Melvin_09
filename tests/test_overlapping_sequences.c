#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_output(const char *label, uint8_t *output, size_t size) {
    printf("%s (%zu bytes): '", label, size);
    for (size_t i = 0; i < size && i < 100; i++) {
        if (output[i] >= 32 && output[i] < 127) {
            printf("%c", output[i]);
        } else {
            printf("\\x%02x", output[i]);
        }
    }
    printf("'\n");
}

void test_input(MelvinMFile *mfile, const char *test_input) {
    printf("\nInput: '%s'\n", test_input);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            print_output("Output", output, output_size);
            free(output);
        }
    } else {
        printf("Output: (no output generated)\n");
    }
}

int main() {
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("TEST: OVERLAPPING SEQUENCES\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_overlap.m");
    MelvinMFile *mfile = melvin_m_create("test_overlap.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("\nTraining with overlapping sequences:\n");
    printf("  - 'abcd'\n");
    printf("  - 'bcde'\n");
    printf("  - 'cdef'\n");
    printf("(5 times each)\n");
    
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abcd", 4);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"bcde", 4);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cdef", 4);
    }
    melvin_m_save(mfile);
    printf("\nTraining complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    printf("\n--- Testing Single Characters ---\n");
    test_input(mfile, "a");
    test_input(mfile, "b");
    test_input(mfile, "c");
    test_input(mfile, "d");
    test_input(mfile, "e");
    test_input(mfile, "f");
    
    printf("\n--- Testing Two-Character Sequences ---\n");
    test_input(mfile, "ab");
    test_input(mfile, "bc");
    test_input(mfile, "cd");
    test_input(mfile, "de");
    
    printf("\n--- Testing Three-Character Sequences ---\n");
    test_input(mfile, "abc");
    test_input(mfile, "bcd");
    test_input(mfile, "cde");
    
    melvin_m_close(mfile);
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    return 0;
}
