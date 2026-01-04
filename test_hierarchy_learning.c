#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// Calculate exact match rate
float calculate_match_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    size_t expected_len = strlen(expected);
    if (expected_len == 0 || actual_len == 0) return 0.0f;
    
    size_t matches = 0;
    size_t min_len = (actual_len < expected_len) ? actual_len : expected_len;
    
    for (size_t i = 0; i < min_len; i++) {
        if (actual[i] == (uint8_t)expected[i]) {
            matches++;
        }
    }
    
    // Perfect match requires correct length AND correct bytes
    if (actual_len == expected_len && matches == expected_len) {
        return 1.0f;
    }
    
    // Partial credit for correct bytes
    return (float)matches / (float)expected_len;
}

typedef struct {
    const char *pattern;
    const char *input_prefix;
    const char *expected_completion;
} TestCase;

int main() {
    printf("=== Hierarchy Learning Test: Multiple Examples ===\n\n");
    printf("Testing if hierarchies guide output generation correctly\n");
    printf("Fix: find_active_hierarchy() now checks INPUT + OUTPUT combined\n\n");
    
    const char *test_file = "test_hierarchy_multi.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Test cases: different patterns with different structures
    TestCase tests[] = {
        {"hello", "hel", "lo"},
        {"world", "wor", "ld"},
        {"testing", "test", "ing"},
        {"learn", "lea", "rn"},
        {"pattern", "patt", "ern"},
        {"complete", "comp", "lete"},
        {"system", "sys", "tem"},
        {"neural", "neu", "ral"}
    };
    int num_tests = 8;
    
    printf("=== Phase 1: Training (200 repetitions per pattern) ===\n\n");
    
    for (int epoch = 0; epoch < 200; epoch++) {
        for (int t = 0; t < num_tests; t++) {
            melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].pattern, strlen(tests[t].pattern));
            melvin_m_process_input(mfile);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        if (epoch % 50 == 49) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            printf("  Epoch %3d: %zu nodes, %zu edges\n", epoch + 1, nodes, edges);
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    printf("\nFinal graph: %zu nodes, %zu edges\n\n", final_nodes, final_edges);
    
    printf("=== Phase 2: Testing Pattern Completion ===\n\n");
    printf("%-15s %-10s %-15s %-15s %-10s\n", "Pattern", "Input", "Expected", "Actual", "Match");
    printf("--------------------------------------------------------------------------------\n");
    
    int perfect_matches = 0;
    int good_matches = 0;
    float total_match_rate = 0.0f;
    
    for (int t = 0; t < num_tests; t++) {
        const char *pattern = tests[t].pattern;
        const char *input = tests[t].input_prefix;
        const char *expected = tests[t].expected_completion;
        
        // Generate output
        melvin_m_universal_input_write(mfile, (uint8_t*)input, strlen(input));
        melvin_m_process_input(mfile);
        
        uint8_t output[256];
        size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        // Calculate match rate
        float match_rate = calculate_match_rate(expected, output, out_len);
        total_match_rate += match_rate;
        
        if (match_rate == 1.0f) perfect_matches++;
        if (match_rate >= 0.8f) good_matches++;
        
        // Print result
        printf("%-15s %-10s %-15s ", pattern, input, expected);
        printf("'");
        for (size_t i = 0; i < out_len && i < 15; i++) {
            printf("%c", output[i]);
        }
        printf("'");
        for (size_t i = out_len; i < 15; i++) printf(" ");
        
        if (match_rate == 1.0f) {
            printf(" ✓ 100%%\n");
        } else if (match_rate >= 0.8f) {
            printf(" ~ %.0f%%\n", match_rate * 100.0f);
        } else {
            printf(" ✗ %.0f%%\n", match_rate * 100.0f);
        }
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("--------------------------------------------------------------------------------\n");
    float avg_match = total_match_rate / num_tests;
    printf("Results: %d/%d perfect (100%%), %d/%d good (≥80%%), avg: %.1f%%\n\n",
           perfect_matches, num_tests, good_matches, num_tests, avg_match * 100.0f);
    
    printf("=== Phase 3: Learning Curve (Error Rate Over Time) ===\n\n");
    printf("Testing if error rate decreases with more training\n\n");
    printf("%-10s %-15s %-15s %-15s\n", "Iteration", "Avg Match Rate", "Perfect Count", "Status");
    printf("--------------------------------------------------------------------------------\n");
    
    // Reset and test learning curve
    melvin_m_close(mfile);
    remove(test_file);
    mfile = melvin_m_create(test_file);
    
    for (int iter = 0; iter <= 10; iter++) {
        // Test current performance
        float iter_match_rate = 0.0f;
        int iter_perfect = 0;
        
        for (int t = 0; t < num_tests; t++) {
            melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].input_prefix, strlen(tests[t].input_prefix));
            melvin_m_process_input(mfile);
            
            uint8_t output[256];
            size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
            
            float match = calculate_match_rate(tests[t].expected_completion, output, out_len);
            iter_match_rate += match;
            if (match == 1.0f) iter_perfect++;
            
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        iter_match_rate /= num_tests;
        
        printf("%-10d %-15.1f%% %-15d ", iter * 20, iter_match_rate * 100.0f, iter_perfect);
        if (iter_match_rate >= 0.9f) {
            printf("✓ Excellent\n");
        } else if (iter_match_rate >= 0.7f) {
            printf("~ Good\n");
        } else if (iter_match_rate >= 0.5f) {
            printf("○ Fair\n");
        } else {
            printf("✗ Poor\n");
        }
        
        // Train more
        if (iter < 10) {
            for (int train = 0; train < 20; train++) {
                for (int t = 0; t < num_tests; t++) {
                    melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].pattern, strlen(tests[t].pattern));
                    melvin_m_process_input(mfile);
                    melvin_m_universal_input_clear(mfile);
                    melvin_m_universal_output_clear(mfile);
                }
            }
        }
    }
    
    printf("\n=== Phase 4: Detailed Analysis ===\n\n");
    
    // Test each pattern in detail
    for (int t = 0; t < 3; t++) {  // Just show first 3 for detail
        printf("Pattern: '%s'\n", tests[t].pattern);
        printf("  Input:    '%s'\n", tests[t].input_prefix);
        printf("  Expected: '%s'\n", tests[t].expected_completion);
        
        melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].input_prefix, strlen(tests[t].input_prefix));
        melvin_m_process_input(mfile);
        
        uint8_t output[256];
        size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        printf("  Actual:   '");
        for (size_t i = 0; i < out_len; i++) {
            printf("%c", output[i]);
        }
        printf("' (%zu bytes)\n", out_len);
        
        float match = calculate_match_rate(tests[t].expected_completion, output, out_len);
        printf("  Match:    %.1f%%\n", match * 100.0f);
        
        if (match == 1.0f) {
            printf("  ✓ PERFECT! Hierarchy guidance working!\n");
        } else if (match >= 0.8f) {
            printf("  ~ Good match, minor errors\n");
        } else {
            printf("  ✗ Poor match, hierarchy not guiding correctly\n");
        }
        printf("\n");
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("=== Summary ===\n\n");
    printf("Graph statistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Total training: 220 iterations per pattern\n\n");
    
    printf("Expected behavior with fix:\n");
    printf("  ✓ find_active_hierarchy() checks INPUT + OUTPUT\n");
    printf("  ✓ When input='hel', output='', full='hel' matches 'hello'\n");
    printf("  ✓ Hierarchy guides: position 3='l', position 4='o'\n");
    printf("  ✓ Output should be 'lo' (correct completion)\n");
    printf("  ✓ Error rate should DECREASE over training\n");
    printf("  ✓ Perfect matches should INCREASE over training\n\n");
    
    printf("If match rate > 80%%: ✓ Fix is working!\n");
    printf("If match rate < 50%%: ✗ Still broken, needs more investigation\n\n");
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("Test complete!\n");
    return 0;
}

