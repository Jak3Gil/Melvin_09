/* Test: Cumulative Pattern Learning
 * Adds multiple patterns to the same .m file sequentially
 * Verifies each pattern works after adding new ones
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void train_and_test(MelvinMFile *mfile, const char *name, 
                    const char *train_data, const char *test_input,
                    int iterations) {
    printf("\n=== Pattern: %s ===\n", name);
    
    size_t before_nodes = melvin_m_get_node_count(mfile);
    size_t before_edges = melvin_m_get_edge_count(mfile);
    
    // Train
    for (int i = 0; i < iterations; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)train_data, strlen(train_data));
        melvin_m_save(mfile);
    }
    
    size_t after_nodes = melvin_m_get_node_count(mfile);
    size_t after_edges = melvin_m_get_edge_count(mfile);
    
    printf("Trained: '%s' x%d\n", train_data, iterations);
    printf("Graph: %zu→%zu nodes (+%zu), %zu→%zu edges (+%zu)\n",
           before_nodes, after_nodes, after_nodes - before_nodes,
           before_edges, after_edges, after_edges - before_edges);
    
    // Test
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            output[output_size] = '\0';
            printf("Test: '%s' → '%s'\n", test_input, output);
            free(output);
        }
    } else {
        printf("Test: '%s' → (no output)\n", test_input);
    }
}

void test_all_patterns(MelvinMFile *mfile, const char **patterns, int count) {
    printf("\n=== Testing All %d Patterns ===\n", count);
    for (int i = 0; i < count; i++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)patterns[i], strlen(patterns[i]));
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (output_size > 0) {
            uint8_t *output = malloc(output_size + 1);
            if (output) {
                melvin_m_universal_output_read(mfile, output, output_size);
                output[output_size] = '\0';
                printf("  '%s' → '%s'\n", patterns[i], output);
                free(output);
            }
        } else {
            printf("  '%s' → (no output)\n", patterns[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    const char *brain_file = "test_cumulative.m";
    
    printf("========================================\n");
    printf("CUMULATIVE PATTERN TEST\n");
    printf("Adding patterns sequentially to same brain\n");
    printf("========================================\n");
    
    // Remove old brain
    remove(brain_file);
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain\n");
        return 1;
    }
    
    printf("\nCreated: %s\n", brain_file);
    
    // Add patterns one by one with MORE training iterations
    train_and_test(mfile, "Pattern 1", "hello world", "hello", 10);
    train_and_test(mfile, "Pattern 2", "hello there", "hello", 10);
    train_and_test(mfile, "Pattern 3", "cat meow", "cat", 10);
    train_and_test(mfile, "Pattern 4", "dog bark", "dog", 10);
    train_and_test(mfile, "Pattern 5", "sun shine", "sun", 10);
    
    // Test all patterns again
    const char *test_inputs[] = {"hello", "cat", "dog", "sun"};
    test_all_patterns(mfile, test_inputs, 4);
    
    // Show final stats
    printf("\n=== Final Graph Stats ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Average degree: %.2f\n", 
           melvin_m_get_node_count(mfile) > 0 ? 
           (double)melvin_m_get_edge_count(mfile) / melvin_m_get_node_count(mfile) : 0.0);
    
    // Cleanup
    melvin_m_close(mfile);
    printf("\nBrain saved: %s\n", brain_file);
    
    return 0;
}
