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
    printf("TEST: MIXED PATTERNS (Words from Different Inputs)\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_mixed.m");
    MelvinMFile *mfile = melvin_m_create("test_mixed.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("\nTraining with:\n");
    printf("  - 'apple pie'\n");
    printf("  - 'apple juice'\n");
    printf("  - 'orange juice'\n");
    printf("(5 times each)\n");
    
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"apple pie", 9);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"apple juice", 11);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"orange juice", 12);
    }
    melvin_m_save(mfile);
    printf("\nTraining complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    printf("\n--- Testing Shared Word 'apple' ---\n");
    printf("(Trained: 'apple pie' and 'apple juice')\n");
    test_input(mfile, "apple");
    
    printf("\n--- Testing Shared Word 'juice' ---\n");
    printf("(Trained: 'apple juice' and 'orange juice')\n");
    test_input(mfile, "juice");
    
    printf("\n--- Testing Novel Combination 'orange pie' ---\n");
    printf("('orange' from one pattern, 'pie' from another)\n");
    test_input(mfile, "orange pie");
    
    printf("\n--- Testing Partial Words ---\n");
    test_input(mfile, "app");
    test_input(mfile, "ora");
    test_input(mfile, "jui");
    
    melvin_m_close(mfile);
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    return 0;
}
