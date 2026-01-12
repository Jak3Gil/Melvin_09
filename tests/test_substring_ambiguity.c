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

void test_input(MelvinMFile *mfile, const char *test_input, const char *description) {
    printf("\n%s\n", description);
    printf("Input: '%s'\n", test_input);
    
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
    printf("TEST: SUBSTRING AMBIGUITY\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    remove("test_substring.m");
    MelvinMFile *mfile = melvin_m_create("test_substring.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("\nTraining with:\n");
    printf("  - 'the cat'\n");
    printf("  - 'the cathedral'\n");
    printf("  - 'cat nap'\n");
    printf("(5 times each)\n");
    
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the cat", 7);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the cathedral", 13);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cat nap", 7);
    }
    melvin_m_save(mfile);
    printf("\nTraining complete. Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    test_input(mfile, "the", "Ambiguous 'the' (could be 'cat' or 'cathedral')");
    test_input(mfile, "cat", "Ambiguous 'cat' (could be ' ' from 'the cat' or ' nap' from 'cat nap')");
    test_input(mfile, "the c", "Partial 'the c' (still ambiguous)");
    test_input(mfile, "the ca", "Partial 'the ca' (still ambiguous)");
    test_input(mfile, "the cat", "Complete 'the cat'");
    test_input(mfile, "the cath", "Partial 'the cath' (should lead to 'cathedral')");
    
    melvin_m_close(mfile);
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("TEST COMPLETE\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    
    return 0;
}
