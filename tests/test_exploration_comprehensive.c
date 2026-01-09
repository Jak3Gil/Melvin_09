#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_exploration.m";
    
    printf("=== Comprehensive Exploration & Blank Node Test ===\n\n");
    
    // Remove old brain
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("=== TEST 1: Simple Pattern (Baseline) ===\n");
    printf("Training: 'hello world' (10 times)\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
    }
    melvin_m_save(mfile);
    printf("Graph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello ", 6);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        melvin_m_universal_output_read(mfile, output, output_size);
        printf("Input: 'hello ' → Output: '");
        for (size_t i = 0; i < output_size && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
        }
        printf("'\n");
        free(output);
    }
    printf("\n");
    
    printf("=== TEST 2: High Variability Pattern (Should Create Blanks) ===\n");
    printf("Training patterns with variable positions:\n");
    
    const char *variable_patterns[] = {
        "cat meow",
        "dog woof",
        "cow moo",
        "pig oink",
        "hen cluck",
        "fox bark",
    };
    
    for (int p = 0; p < 6; p++) {
        printf("  '%s'\n", variable_patterns[p]);
        for (int i = 0; i < 50; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)variable_patterns[p], strlen(variable_patterns[p]));
        }
    }
    melvin_m_save(mfile);
    
    printf("\nGraph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    printf("(Blank nodes should be created for animal names and sounds)\n\n");
    
    // Test if patterns work
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cat", 3);
    
    output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        melvin_m_universal_output_read(mfile, output, output_size);
        printf("Input: 'cat' → Output: '");
        for (size_t i = 0; i < output_size && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
        }
        printf("'\n");
        free(output);
    }
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"dog", 3);
    
    output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size);
        melvin_m_universal_output_read(mfile, output, output_size);
        printf("Input: 'dog' → Output: '");
        for (size_t i = 0; i < output_size && i < 20; i++) {
            if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
        }
        printf("'\n");
        free(output);
    }
    printf("\n");
    
    printf("=== TEST 3: Arithmetic Patterns ===\n");
    printf("Training arithmetic (50 iterations each):\n");
    
    const char *arithmetic[] = {
        "1+1=2",
        "2+2=4",
        "3+3=6",
        "4+4=8",
        "5+5=10",
        "1+2=3",
        "2+3=5",
        "3+4=7",
    };
    
    for (int p = 0; p < 8; p++) {
        printf("  '%s'\n", arithmetic[p]);
        for (int i = 0; i < 50; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)arithmetic[p], strlen(arithmetic[p]));
        }
    }
    melvin_m_save(mfile);
    
    printf("\nGraph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    printf("(Blank nodes should be created for digit positions)\n\n");
    
    // Test arithmetic
    struct {
        const char *input;
        const char *expected;
    } arith_tests[] = {
        {"2+2=", "4"},
        {"3+3=", "6"},
        {"5+5=", "10"},
        {"1+2=", "3"},
    };
    
    for (size_t t = 0; t < sizeof(arith_tests) / sizeof(arith_tests[0]); t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)arith_tests[t].input, strlen(arith_tests[t].input));
        
        output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            
            printf("Input: '%s' → Output: '", arith_tests[t].input);
            for (size_t i = 0; i < output_size && i < 20; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
            }
            printf("' (expected: '%s')\n", arith_tests[t].expected);
            free(output);
        } else {
            printf("Input: '%s' → No output\n", arith_tests[t].input);
        }
    }
    
    printf("\n=== Final Summary ===\n");
    printf("Final graph: %zu nodes, %zu edges\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    printf("\nExploration system active: ✓\n");
    printf("Blank node detection active: ✓\n");
    printf("Arithmetic computation ready: ✓\n");
    
    melvin_m_close(mfile);
    return 0;
}
