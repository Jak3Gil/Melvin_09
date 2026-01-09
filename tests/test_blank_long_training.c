#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    const char *brain_file = (argc > 1) ? argv[1] : "test_blank_long.m";
    
    printf("=== LONG TRAINING TEST: Blank Node Creation ===\n");
    printf("Testing with extended training to build high variability\n\n");
    
    // Remove old brain
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    printf("=== PHASE 1: Variable Position Training (1000 iterations) ===\n");
    printf("Training patterns where middle position varies:\n");
    
    // Patterns with HIGHLY variable middle position
    const char *animal_patterns[] = {
        "a cat runs",
        "a dog runs",
        "a fox runs",
        "a pig runs",
        "a cow runs",
        "a hen runs",
        "a rat runs",
        "a bat runs",
        "a owl runs",
        "a ant runs",
    };
    
    int num_animals = sizeof(animal_patterns) / sizeof(animal_patterns[0]);
    
    for (int p = 0; p < num_animals; p++) {
        printf("  '%s'\n", animal_patterns[p]);
    }
    
    printf("\nTraining 1000 iterations (100 per pattern)...\n");
    for (int iter = 0; iter < 100; iter++) {
        for (int p = 0; p < num_animals; p++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)animal_patterns[p], strlen(animal_patterns[p]));
        }
        
        if ((iter + 1) % 20 == 0) {
            printf("  Iteration %d: %zu nodes, %zu edges\n", 
                   (iter + 1) * num_animals, 
                   melvin_m_get_node_count(mfile), 
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    melvin_m_save(mfile);
    
    printf("\n=== PHASE 2: Arithmetic Training (1000 iterations) ===\n");
    printf("Training patterns where digit positions vary:\n");
    
    const char *arithmetic[] = {
        "0+0=0",
        "1+1=2",
        "2+2=4",
        "3+3=6",
        "4+4=8",
        "5+5=10",
        "6+6=12",
        "7+7=14",
        "8+8=16",
        "9+9=18",
        "1+2=3",
        "2+3=5",
        "3+4=7",
        "4+5=9",
        "5+6=11",
        "6+7=13",
        "7+8=15",
        "8+9=17",
    };
    
    int num_arith = sizeof(arithmetic) / sizeof(arithmetic[0]);
    
    for (int p = 0; p < num_arith; p++) {
        printf("  '%s'\n", arithmetic[p]);
    }
    
    printf("\nTraining 1000 iterations (55 per pattern)...\n");
    for (int iter = 0; iter < 55; iter++) {
        for (int p = 0; p < num_arith; p++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)arithmetic[p], strlen(arithmetic[p]));
        }
        
        if ((iter + 1) % 10 == 0) {
            printf("  Iteration %d: %zu nodes, %zu edges\n", 
                   (iter + 1) * num_arith, 
                   melvin_m_get_node_count(mfile), 
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    melvin_m_save(mfile);
    
    printf("\n=== PHASE 3: Testing Pattern Completion ===\n\n");
    
    // Test animal patterns
    printf("Animal Pattern Tests:\n");
    const char *animal_tests[] = {"a cat", "a dog", "a fox", "a pig"};
    for (size_t t = 0; t < sizeof(animal_tests) / sizeof(animal_tests[0]); t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)animal_tests[t], strlen(animal_tests[t]));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("  '%s' → ", animal_tests[t]);
        
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("'");
            for (size_t i = 0; i < output_size && i < 20; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
            }
            printf("'\n");
            free(output);
        } else {
            printf("(no output)\n");
        }
    }
    
    printf("\nArithmetic Pattern Tests:\n");
    const char *arith_tests[] = {"2+2=", "5+5=", "7+8=", "3+4="};
    for (size_t t = 0; t < sizeof(arith_tests) / sizeof(arith_tests[0]); t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)arith_tests[t], strlen(arith_tests[t]));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("  '%s' → ", arith_tests[t]);
        
        if (output_size > 0) {
            uint8_t *output = malloc(output_size);
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("'");
            for (size_t i = 0; i < output_size && i < 20; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
            }
            printf("'\n");
            free(output);
        } else {
            printf("(no output)\n");
        }
    }
    
    printf("\n=== FINAL STATISTICS ===\n");
    printf("Total nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Total edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Training iterations: 2000\n");
    printf("Unique patterns: %d\n", num_animals + num_arith);
    
    printf("\n=== BLANK NODE ANALYSIS ===\n");
    printf("Expected blank positions:\n");
    printf("  - 'a X runs' → X should be blank (10 different animals)\n");
    printf("  - 'X+Y=Z' → X, Y, Z should be blanks (18 different combinations)\n");
    printf("\nBlank nodes should be created where diversity exceeds graph average.\n");
    printf("Check if nodes like ' ' (space after 'a') have high variability.\n");
    
    melvin_m_close(mfile);
    return 0;
}
