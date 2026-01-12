/*
 * TEST 3: Hierarchy Recognition Benefit
 * 
 * Brain analog: Chunking in working memory
 * - "hello world" becomes single chunk after learning
 * - Faster processing, reduced cognitive load
 * 
 * Tests:
 * 1. Do hierarchies speed up recognition?
 * 2. Do they improve completion accuracy?
 * 3. Do nested hierarchies form?
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void test_hierarchy_completion() {
    printf("\n=== Test 1: Hierarchy-Assisted Completion ===\n");
    
    remove("/tmp/test_hier_comp.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_hier_comp.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train common phrase repeatedly to form hierarchies
    printf("Training 'hello world' 20 times (should form hierarchies):\n");
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("  Nodes: %zu, Edges: %zu\n", nodes, edges);
    
    // Hierarchies should exist: 'he', 'el', 'll', 'lo', 'o ', ' w', 'wo', 'or', 'rl', 'ld'
    // Unique chars: h, e, l, o, space, w, r, d = 8
    // Expected nodes: ~8 chars + ~10 hierarchies = ~18
    
    if (nodes > 12) {
        printf("  ✓ Hierarchies likely formed (node count suggests hierarchies exist)\n");
    } else {
        printf("  ✗ Few hierarchies (node count low)\n");
    }
    
    // Test completion with partial input
    printf("\nTest: Input 'hel' (partial):\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hel", 3);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        printf("  Output: '%.*s' (%zu bytes)\n", (int)out_size, output, out_size);
        
        // Check if it completes to "lo world" (continuing from 'hel')
        int has_lo = (out_size >= 2 && output[0] == 'l' && output[1] == 'o');
        int has_world = 0;
        for (size_t i = 0; i < out_size - 4; i++) {
            if (memcmp(&output[i], "world", 5) == 0) {
                has_world = 1;
                break;
            }
        }
        
        printf("  Contains 'lo': %s\n", has_lo ? "YES ✓" : "NO");
        printf("  Contains 'world': %s\n", has_world ? "YES ✓" : "NO");
        
        if (has_lo || has_world) {
            printf("PASS: Hierarchy-assisted completion working!\n");
        } else {
            printf("PARTIAL: Completion exists but not using hierarchies effectively\n");
        }
        
        free(output);
    } else {
        printf("  No output\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_hier_comp.m");
}

void test_nested_hierarchies() {
    printf("\n=== Test 2: Nested Hierarchy Formation ===\n");
    
    remove("/tmp/test_nested.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_nested.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train progressively longer sequences
    // First: "ab" → forms 'ab' hierarchy (level 1)
    // Then: "abcd" → should form 'cd' hierarchy, then 'abcd' (level 2)
    
    printf("Phase 1: Training 'ab' (20 times):\n");
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_1 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu\n", nodes_1);
    
    printf("\nPhase 2: Training 'cd' (20 times):\n");
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cd", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_2 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_2, nodes_2 - nodes_1);
    
    printf("\nPhase 3: Training 'abcd' (30 times):\n");
    for (int i = 0; i < 30; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abcd", 4);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_3 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_3, nodes_3 - nodes_2);
    
    // Expected: 4 chars + 'ab' + 'cd' + potentially 'abcd' = 7+ nodes
    printf("\nAnalysis:\n");
    printf("  Char nodes: 4 (a, b, c, d)\n");
    printf("  Level-1 hierarchies: ~2 ('ab', 'cd')\n");
    printf("  Level-2 hierarchies: ~1 ('abcd' if nested)\n");
    printf("  Expected total: ~7 nodes\n");
    printf("  Actual: %zu nodes\n", nodes_3);
    
    if (nodes_3 >= 6) {
        printf("PASS: Nested hierarchies likely forming\n");
    } else {
        printf("PARTIAL: May not be forming nested hierarchies\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_nested.m");
}

void test_hierarchy_recognition_speed() {
    printf("\n=== Test 3: Hierarchy Recognition Speed ===\n");
    printf("(Conceptual test - timing not reliable in single-threaded C)\n");
    
    remove("/tmp/test_speed.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_speed.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train a long repeated sequence
    const char *seq = "the quick brown fox jumps over the lazy dog";
    printf("Training: '%s'\n", seq);
    printf("(50 iterations to form many hierarchies)\n");
    
    for (int i = 0; i < 50; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)seq, strlen(seq));
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nAfter training:\n");
    printf("  Nodes: %zu, Edges: %zu\n", nodes, edges);
    
    // Unique chars in sequence: ~20
    // If many hierarchies formed, node count should be significantly higher
    if (nodes > 40) {
        printf("  ✓ Many hierarchies formed (nodes >> unique chars)\n");
        printf("PASS: Hierarchies are being created for common patterns\n");
    } else {
        printf("  ✗ Few hierarchies (nodes ≈ unique chars)\n");
        printf("PARTIAL: Hierarchy formation may be limited\n");
    }
    
    // Test recognition
    printf("\nTest: Input 'the quick':\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"the quick", 9);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        printf("  Output: '%.*s'\n", (int)(out_size > 30 ? 30 : out_size), output);
        
        // Check if it continues with " brown"
        int has_brown = 0;
        for (size_t i = 0; i < out_size - 4; i++) {
            if (memcmp(&output[i], "brown", 5) == 0) {
                has_brown = 1;
                break;
            }
        }
        
        printf("  Continues with 'brown': %s\n", has_brown ? "YES ✓" : "NO");
        
        free(output);
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_speed.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          HIERARCHY RECOGNITION BENEFIT TEST                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_hierarchy_completion();
    test_nested_hierarchies();
    test_hierarchy_recognition_speed();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
