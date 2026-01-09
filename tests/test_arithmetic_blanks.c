#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_arithmetic.m";
    
    printf("=== Testing Arithmetic Pattern & Blank Node Creation ===\n\n");
    
    // Remove old brain to start fresh
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("Created new brain: %s\n\n", brain_file);
    
    // Train with multiple arithmetic patterns
    printf("=== Training Arithmetic Patterns ===\n");
    const char *patterns[] = {
        "2+2=4",
        "3+3=6",
        "4+4=8",
        "5+5=10",
        "1+1=2",
        "6+6=12",
        "7+7=14",
        "8+8=16",
        "9+9=18",
    };
    
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Training %d patterns (30 iterations each):\n", num_patterns);
    for (int p = 0; p < num_patterns; p++) {
        printf("  '%s'\n", patterns[p]);
        for (int i = 0; i < 30; i++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)patterns[p], strlen(patterns[p]));
        }
    }
    melvin_m_save(mfile);
    
    printf("\n=== After Training ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Check for blank nodes
    size_t blank_count = 0;
    // Note: We don't have direct API to count blanks, but we can infer from behavior
    
    printf("\n=== Testing Pattern Continuation ===\n\n");
    
    // Test various inputs
    struct {
        const char *input;
        const char *description;
    } test_cases[] = {
        {"2+2", "Simple: 2+2 (trained)"},
        {"3+3", "Simple: 3+3 (trained)"},
        {"5+5", "Simple: 5+5 (trained)"},
        {"1+2", "New: 1+2 (not trained)"},
        {"4+5", "New: 4+5 (not trained)"},
        {"7+3", "New: 7+3 (not trained)"},
        {"2+", "Partial: 2+ (incomplete)"},
        {"5", "Single: 5 (just digit)"},
        {"+", "Operator: + (just operator)"},
    };
    
    for (size_t t = 0; t < sizeof(test_cases) / sizeof(test_cases[0]); t++) {
        printf("Test %zu: %s\n", t + 1, test_cases[t].description);
        printf("  Input: '%s'\n", test_cases[t].input);
        
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_cases[t].input, strlen(test_cases[t].input));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            if (output) {
                melvin_m_universal_output_read(mfile, output, output_size);
                printf("  Output (%zu bytes): ", output_size);
                for (size_t i = 0; i < output_size && i < 40; i++) {
                    if (output[i] >= 32 && output[i] < 127) {
                        printf("%c", output[i]);
                    } else {
                        printf("\\x%02x", output[i]);
                    }
                }
                if (output_size > 40) printf("...");
                printf("\n");
                free(output);
            }
        } else {
            printf("  No output generated\n");
        }
        printf("\n");
    }
    
    printf("=== Final Stats ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("\nNote: Blank nodes would be created if high variability detected\n");
    printf("      (nodes with 3+ different incoming/outgoing connections)\n");
    
    melvin_m_close(mfile);
    return 0;
}
