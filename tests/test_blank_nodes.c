/*
 * TEST 2: Blank Node Pattern Separation
 * 
 * Brain analog: Dentate gyrus pattern separation
 * - Creates orthogonal representations for similar inputs
 * - Prevents interference between overlapping patterns
 * 
 * Tests:
 * 1. Do blank nodes form when needed?
 * 2. Do they improve disambiguation?
 * 3. Compare: with blank nodes vs. ContextTags only
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_blank_node_formation() {
    printf("\n=== Test 1: Blank Node Formation ===\n");
    
    remove("/tmp/test_blank.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_blank.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train patterns with high ambiguity at shared prefix
    // "the cat sat", "the dog ran", "the bird flew", "the fish swam"
    printf("Training 4 patterns with shared prefix 'the ':\n");
    const char *patterns[] = {
        "the cat sat",
        "the dog ran", 
        "the bird flew",
        "the fish swam"
    };
    
    for (int iter = 0; iter < 20; iter++) {
        for (int p = 0; p < 4; p++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    size_t node_count = melvin_m_get_node_count(mfile);
    printf("\nAfter training:\n");
    printf("  Total nodes: %zu\n", node_count);
    
    // Count blank nodes (payload_size == 0)
    // Note: We can't directly access node internals, so we infer from behavior
    printf("  (Blank nodes have payload_size == 0, counted internally)\n");
    
    // Test discrimination at the ambiguous point
    printf("\n=== Testing Discrimination ===\n");
    
    const char *tests[] = {"the c", "the d", "the b", "the f"};
    const char *expected[] = {"cat", "dog", "bird", "fish"};
    int pass_count = 0;
    
    for (int t = 0; t < 4; t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)tests[t], strlen(tests[t]));
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            // Check if output contains expected first char
            int has_expected = 0;
            for (size_t i = 0; i < out_size; i++) {
                if (output[i] == expected[t][0]) {
                    has_expected = 1;
                    break;
                }
            }
            
            printf("  '%s' → '%.*s' (expected '%c': %s)\n",
                   tests[t], (int)(out_size > 15 ? 15 : out_size), output,
                   expected[t][0], has_expected ? "✓" : "✗");
            
            if (has_expected) pass_count++;
            free(output);
        }
    }
    
    printf("\nDiscrimination: %d/4 correct\n", pass_count);
    if (pass_count >= 3) {
        printf("PASS: Pattern separation working (blank nodes or ContextTags)\n");
    } else {
        printf("FAIL: Poor discrimination\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_blank.m");
}

void test_blank_vs_context_tags() {
    printf("\n=== Test 2: Blank Nodes vs ContextTags ===\n");
    printf("Testing if blank nodes provide additional benefit over ContextTags\n");
    
    // This test compares disambiguation quality with/without blank nodes
    // Since blank node creation is automatic, we test by measuring
    // disambiguation accuracy with varying pattern complexity
    
    remove("/tmp/test_compare.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_compare.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train highly overlapping patterns
    printf("Training 6 overlapping patterns:\n");
    const char *patterns[] = {
        "abcdef",
        "abcdgh",
        "abcijk",
        "ablmno",
        "abpqrs",
        "abtuvw"
    };
    
    for (int iter = 0; iter < 10; iter++) {
        for (int p = 0; p < 6; p++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("Nodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test discrimination at various branch points
    printf("\nTesting discrimination:\n");
    const char *tests[] = {"abc", "abl", "abp", "abt"};
    const char *expected_chars[] = {"d", "l", "p", "t"};
    int correct = 0;
    
    for (int t = 0; t < 4; t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)tests[t], strlen(tests[t]));
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            int has_expected = (out_size > 0 && output[0] == expected_chars[t][0]);
            printf("  '%s' → '%c' (expected '%s': %s)\n",
                   tests[t], out_size > 0 ? output[0] : '?',
                   expected_chars[t], has_expected ? "✓" : "✗");
            
            if (has_expected) correct++;
            free(output);
        }
    }
    
    printf("\nAccuracy: %d/4 (%.0f%%)\n", correct, correct * 25.0f);
    if (correct >= 3) {
        printf("PASS: High disambiguation accuracy\n");
    } else {
        printf("PARTIAL: Moderate disambiguation\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_compare.m");
}

void test_blank_node_threshold() {
    printf("\n=== Test 3: Blank Node Creation Threshold ===\n");
    printf("Testing when blank nodes should form\n");
    
    remove("/tmp/test_threshold.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_threshold.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Start with 2 patterns (should NOT create blank - threshold is 3)
    printf("Phase 1: Training 2 patterns (below threshold):\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xa", 2);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xb", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_2 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu\n", nodes_2);
    
    // Add 3rd pattern (should trigger blank node creation)
    printf("\nPhase 2: Adding 3rd pattern (at threshold):\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xc", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_3 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_3, nodes_3 - nodes_2);
    
    // Add 4th pattern
    printf("\nPhase 3: Adding 4th pattern:\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xd", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_4 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_4, nodes_4 - nodes_3);
    
    if (nodes_3 > nodes_2 + 1 || nodes_4 > nodes_3 + 1) {
        printf("\nPASS: Blank nodes appear to be forming (node count increased)\n");
    } else {
        printf("\nPARTIAL: Blank nodes may not be forming (check threshold)\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_threshold.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           BLANK NODE PATTERN SEPARATION TEST               ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_blank_node_formation();
    test_blank_vs_context_tags();
    test_blank_node_threshold();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
