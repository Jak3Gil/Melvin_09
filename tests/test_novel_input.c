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
    printf("TEST: NOVEL INPUT (Never Seen Before)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_novel.m");
    MelvinMFile *mfile = melvin_m_create("test_novel.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("\nTraining with: 'hello world' (5 times)\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
    }
    melvin_m_save(mfile);
    printf("Training complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    printf("\n--- Testing Known Input ---\n");
    test_input(mfile, "hello");
    
    printf("\n--- Testing Novel Inputs (Never Seen) ---\n");
    test_input(mfile, "zebra");
    test_input(mfile, "xyz");
    test_input(mfile, "12345");
    test_input(mfile, "goodbye");
    
    melvin_m_close(mfile);
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    return 0;
}
