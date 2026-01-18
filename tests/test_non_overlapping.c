/* Test associations with minimal overlap */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    printf("=== Non-Overlapping Associations Test ===\n\n");
    
    const char *test_file = "test_non_overlap.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("❌ Failed to create brain\n");
        return 1;
    }
    
    // Patterns with minimal character overlap
    const char *patterns[] = {
        "cat meow",
        "dog bark", 
        "fish swim",
        "lion roar",
    };
    
    size_t num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Training %zu patterns (10 iterations each)...\n\n", num_patterns);
    
    for (size_t i = 0; i < num_patterns; i++) {
        printf("Training: '%s'\n", patterns[i]);
        for (int iter = 0; iter < 10; iter++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(mfile));
    
    printf("\n=== Testing Associations ===\n\n");
    
    const char *tests[][2] = {
        {"cat", " meow"},
        {"dog", " bark"},
        {"fish", " swim"},
        {"lion", " roar"},
    };
    
    int passed = 0;
    for (size_t i = 0; i < num_patterns; i++) {
        printf("Test %zu: '%s' -> ", i + 1, tests[i][0]);
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)tests[i][0], strlen(tests[i][0]));
        melvin_m_process_input(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size + 1);
            size_t read = melvin_m_universal_output_read(mfile, output, output_size);
            output[read] = '\0';
            
            printf("'");
            for (size_t j = 0; j < read && j < 20; j++) {
                if (output[j] >= 32 && output[j] < 127) {
                    printf("%c", output[j]);
                } else {
                    printf(".");
                }
            }
            printf("'");
            
            // Check if starts with expected
            const char *expected = tests[i][1];
            int match = 1;
            for (size_t j = 0; j < strlen(expected) && j < read; j++) {
                if (output[j] != expected[j]) {
                    match = 0;
                    break;
                }
            }
            
            if (match) {
                printf(" ✅");
                passed++;
            } else {
                printf(" ❌ (expected '%s')", expected);
            }
            
            free(output);
        } else {
            printf("(empty) ❌");
        }
        
        printf("\n");
    }
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%zu (%.1f%%)\n", passed, num_patterns,
           num_patterns > 0 ? (100.0f * passed / num_patterns) : 0.0f);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return (passed == num_patterns) ? 0 : 1;
}
