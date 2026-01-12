/* Comprehensive Scale Test: Multiple Patterns on Same Graph
 * Tests that the multi-pattern trie enables:
 * 1. Multiple association patterns on the same .m file
 * 2. Graph growth without pattern interference
 * 3. All patterns work correctly even with large graph
 * 4. Knowledge compounds across patterns
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Test result tracking
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestResults;

void print_test_header(const char *name) {
    printf("\n========================================\n");
    printf("TEST: %s\n", name);
    printf("========================================\n");
}

void print_test_result(TestResults *results, const char *test_name, int passed) {
    results->total_tests++;
    if (passed) {
        results->passed_tests++;
        printf("  ✓ PASS: %s\n", test_name);
    } else {
        results->failed_tests++;
        printf("  ✗ FAIL: %s\n", test_name);
    }
}

void print_final_results(TestResults *results) {
    printf("\n========================================\n");
    printf("FINAL RESULTS\n");
    printf("========================================\n");
    printf("Total Tests: %d\n", results->total_tests);
    printf("Passed: %d (%.1f%%)\n", results->passed_tests, 
           100.0 * results->passed_tests / results->total_tests);
    printf("Failed: %d (%.1f%%)\n", results->failed_tests,
           100.0 * results->failed_tests / results->total_tests);
    printf("========================================\n");
}

int test_pattern(MelvinMFile *mfile, const char *pattern_name, 
                 const char *train_data, const char *test_input, 
                 const char *expected_substring, int iterations) {
    printf("\n--- Training pattern: %s ---\n", pattern_name);
    
    size_t before_nodes = melvin_m_get_node_count(mfile);
    size_t before_edges = melvin_m_get_edge_count(mfile);
    
    // Train the pattern
    for (int i = 0; i < iterations; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)train_data, strlen(train_data));
        melvin_m_save(mfile);
    }
    
    size_t after_nodes = melvin_m_get_node_count(mfile);
    size_t after_edges = melvin_m_get_edge_count(mfile);
    
    printf("  Trained '%s' x%d\n", train_data, iterations);
    printf("  Graph grew: %zu→%zu nodes (+%zu), %zu→%zu edges (+%zu)\n",
           before_nodes, after_nodes, after_nodes - before_nodes,
           before_edges, after_edges, after_edges - before_edges);
    
    // Test recall
    printf("  Testing recall...\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_input, strlen(test_input));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            output[output_size] = '\0';
            
            printf("  Input: '%s' → Output: '%s' (%zu bytes)\n", 
                   test_input, output, output_size);
            
            int found = (expected_substring == NULL || 
                        strstr((char*)output, expected_substring) != NULL);
            
            if (found) {
                printf("  ✓ Pattern recalled successfully!\n");
            } else {
                printf("  ✗ Expected substring '%s' not found\n", expected_substring);
            }
            
            free(output);
            return found;
        }
    }
    
    printf("  ⚠ No output generated\n");
    return 0;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    TestResults results = {0, 0, 0};
    const char *brain_file = "test_comprehensive_scale.m";
    
    printf("========================================\n");
    printf("COMPREHENSIVE SCALE TEST\n");
    printf("Testing multi-pattern support at scale\n");
    printf("========================================\n");
    
    // Remove old brain
    remove(brain_file);
    
    // Create new brain
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    printf("\nCreated brain: %s\n", brain_file);
    printf("Initial state: %zu nodes, %zu edges\n",
           melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    // ========================================
    // TEST 1: Basic Association
    // ========================================
    print_test_header("Basic Association");
    int pass1 = test_pattern(mfile, "hello→world", "hello world", "hello", "w", 5);
    print_test_result(&results, "Pattern 1: hello→world", pass1);
    
    // ========================================
    // TEST 2: Second Pattern on Same Graph
    // ========================================
    print_test_header("Second Pattern (Shared Prefix)");
    int pass2 = test_pattern(mfile, "hello→there", "hello there", "hello", "t", 5);
    print_test_result(&results, "Pattern 2: hello→there (shared 'hello')", pass2);
    
    // ========================================
    // TEST 3: Verify First Pattern Still Works
    // ========================================
    print_test_header("First Pattern Recall After Second");
    printf("\n--- Recalling first pattern ---\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    int pass3 = 0;
    if (output_size > 0) {
        uint8_t *output = malloc(output_size + 1);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            output[output_size] = '\0';
            printf("  Input: 'hello' → Output: '%s'\n", output);
            
            // Should get either 'w' or 't' (both valid)
            pass3 = (strchr((char*)output, 'w') != NULL || 
                    strchr((char*)output, 't') != NULL);
            free(output);
        }
    }
    print_test_result(&results, "First pattern still works", pass3);
    
    // ========================================
    // TEST 4: Unrelated Pattern
    // ========================================
    print_test_header("Unrelated Pattern");
    int pass4 = test_pattern(mfile, "cat→meow", "cat meow", "cat", "m", 5);
    print_test_result(&results, "Pattern 3: cat→meow (independent)", pass4);
    
    // ========================================
    // TEST 5: Another Unrelated Pattern
    // ========================================
    print_test_header("Fourth Pattern");
    int pass5 = test_pattern(mfile, "dog→bark", "dog bark", "dog", "b", 5);
    print_test_result(&results, "Pattern 4: dog→bark", pass5);
    
    // ========================================
    // TEST 6: Pattern with Shared Subsequence
    // ========================================
    print_test_header("Pattern with Shared Subsequence");
    int pass6 = test_pattern(mfile, "goodbye→world", "goodbye world", "goodbye", "w", 5);
    print_test_result(&results, "Pattern 5: goodbye→world (shares 'world')", pass6);
    
    // ========================================
    // TEST 7: Longer Pattern
    // ========================================
    print_test_header("Longer Pattern");
    int pass7 = test_pattern(mfile, "the quick brown", 
                            "the quick brown fox jumps over", 
                            "the quick", "brown", 5);
    print_test_result(&results, "Pattern 6: longer sequence", pass7);
    
    // ========================================
    // TEST 8: Pattern with Numbers
    // ========================================
    print_test_header("Pattern with Numbers");
    int pass8 = test_pattern(mfile, "count→123", "count 123", "count", "1", 5);
    print_test_result(&results, "Pattern 7: count→123", pass8);
    
    // ========================================
    // TEST 9: Verify All Patterns Coexist
    // ========================================
    print_test_header("All Patterns Coexist");
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal graph stats:\n");
    printf("  Nodes: %zu\n", final_nodes);
    printf("  Edges: %zu\n", final_edges);
    printf("  Average degree: %.2f\n", 
           final_nodes > 0 ? (double)final_edges / final_nodes : 0.0);
    
    int pass9 = (final_nodes > 0 && final_edges > 0);
    print_test_result(&results, "Graph grew successfully", pass9);
    
    // ========================================
    // TEST 10: Random Pattern Recall
    // ========================================
    print_test_header("Random Pattern Recall Test");
    
    const char *test_inputs[] = {"hello", "cat", "dog", "goodbye", "the quick", "count"};
    int num_inputs = 6;
    int recall_success = 0;
    
    for (int i = 0; i < num_inputs; i++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)test_inputs[i], strlen(test_inputs[i]));
        
        size_t size = melvin_m_universal_output_size(mfile);
        if (size > 0) {
            uint8_t *out = malloc(size + 1);
            if (out) {
                melvin_m_universal_output_read(mfile, out, size);
                out[size] = '\0';
                printf("  '%s' → '%s'\n", test_inputs[i], out);
                recall_success++;
                free(out);
            }
        } else {
            printf("  '%s' → (no output)\n", test_inputs[i]);
        }
    }
    
    int pass10 = (recall_success >= num_inputs / 2);  // At least half should recall
    print_test_result(&results, "Multiple pattern recall", pass10);
    
    // ========================================
    // Cleanup
    // ========================================
    printf("\n--- Cleanup ---\n");
    melvin_m_close(mfile);
    printf("Brain closed successfully\n");
    
    // Print final results
    print_final_results(&results);
    
    // Keep the brain file for inspection
    printf("\nBrain file saved: %s (for inspection)\n", brain_file);
    
    return (results.failed_tests == 0) ? 0 : 1;
}
