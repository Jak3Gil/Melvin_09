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
    printf("TEST: PARTIAL MATCHES (Shared Prefixes)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_partial.m");
    MelvinMFile *mfile = melvin_m_create("test_partial.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("\nTraining with: 'hello world' and 'hello friend' (5 times each)\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello friend", 12);
    }
    melvin_m_save(mfile);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    printf("\n--- Testing Shared Prefix 'hello' ---\n");
    printf("(Should output either 'world' or 'friend' since both are valid)\n");
    test_input(mfile, "hello");
    
    printf("\n--- Testing Partial Prefixes ---\n");
    test_input(mfile, "hel");
    test_input(mfile, "hell");
    test_input(mfile, "hello ");
    
    printf("\n--- Testing Novel Suffix ---\n");
    test_input(mfile, "hello stranger");
    
    melvin_m_close(mfile);
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    return 0;
}
