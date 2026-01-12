/*
 * TEST 4: Scalability
 * 
 * Brain analog: Long-term memory consolidation
 * - Can handle sequences of 100+ elements
 * - Hierarchical compression for efficiency
 * 
 * Tests:
 * 1. Long sequences (100+ chars)
 * 2. Multiple overlapping patterns
 * 3. Memory efficiency
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_long_sequence() {
    printf("\n=== Test 1: Long Sequence Learning ===\n");
    
    remove("/tmp/test_long.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_long.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train a 100+ character sequence
    const char *long_seq = 
        "In the beginning was the Word and the Word was with God and the Word was God "
        "He was in the beginning with God";
    
    size_t seq_len = strlen(long_seq);
    printf("Training sequence of %zu characters:\n", seq_len);
    printf("  First 50: '%.50s...'\n", long_seq);
    
    printf("\nTraining 10 iterations...\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)long_seq, seq_len);
        melvin_m_universal_output_clear(mfile);
        
        if (i == 0 || i == 4 || i == 9) {
            printf("  Iteration %d: Nodes=%zu, Edges=%zu\n",
                   i+1,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal stats:\n");
    printf("  Nodes: %zu\n", final_nodes);
    printf("  Edges: %zu\n", final_edges);
    printf("  Sequence length: %zu\n", seq_len);
    
    // Calculate compression ratio
    // If hierarchies are working, nodes << sequence length
    float compression = (float)seq_len / (float)final_nodes;
    printf("  Compression ratio: %.2fx\n", compression);
    
    if (compression > 2.0f) {
        printf("  ✓ Good compression (hierarchies working)\n");
    } else {
        printf("  ✗ Poor compression (minimal hierarchy formation)\n");
    }
    
    // Test partial recall
    printf("\nTest: Input 'In the beginning':\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"In the beginning", 16);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        printf("  Output (%zu bytes): '%.50s%s'\n", 
               out_size, output, out_size > 50 ? "..." : "");
        
        // Check if it continues correctly
        int has_was = 0;
        for (size_t i = 0; i < out_size - 2; i++) {
            if (memcmp(&output[i], "was", 3) == 0) {
                has_was = 1;
                break;
            }
        }
        
        printf("  Contains 'was': %s\n", has_was ? "YES ✓" : "NO");
        
        if (has_was && out_size > 10) {
            printf("PASS: Long sequence learned and recalled!\n");
        } else {
            printf("PARTIAL: Some learning but incomplete recall\n");
        }
        
        free(output);
    } else {
        printf("  No output\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_long.m");
}

void test_overlapping_patterns() {
    printf("\n=== Test 2: Multiple Overlapping Patterns ===\n");
    
    remove("/tmp/test_overlap.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_overlap.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train patterns that share subsequences
    const char *patterns[] = {
        "the cat sat on the mat",
        "the dog sat on the rug",
        "the bird sat on the branch",
        "the cat ran to the door",
        "the dog ran to the park"
    };
    int num_patterns = 5;
    
    printf("Training %d overlapping patterns:\n", num_patterns);
    for (int i = 0; i < num_patterns; i++) {
        printf("  %d. '%s'\n", i+1, patterns[i]);
    }
    
    printf("\nTraining 10 iterations each...\n");
    for (int iter = 0; iter < 10; iter++) {
        for (int p = 0; p < num_patterns; p++) {
            melvin_in_port_handle_buffer(mfile, 0, 
                                         (uint8_t*)patterns[p], 
                                         strlen(patterns[p]));
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    printf("\nAfter training:\n");
    printf("  Nodes: %zu\n", nodes);
    printf("  Edges: %zu\n", edges);
    
    // Test discrimination at branch points
    printf("\n=== Testing Discrimination ===\n");
    
    struct {
        const char *input;
        const char *expected_word;
    } tests[] = {
        {"the cat sat", "mat"},
        {"the dog sat", "rug"},
        {"the bird sat", "branch"},
        {"the cat ran", "door"},
        {"the dog ran", "park"}
    };
    
    int correct = 0;
    for (int t = 0; t < 5; t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, 
                                     (uint8_t*)tests[t].input, 
                                     strlen(tests[t].input));
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            // Check if output contains expected word
            int has_expected = 0;
            size_t exp_len = strlen(tests[t].expected_word);
            for (size_t i = 0; i < out_size - exp_len; i++) {
                if (memcmp(&output[i], tests[t].expected_word, exp_len) == 0) {
                    has_expected = 1;
                    break;
                }
            }
            
            printf("  '%s' → contains '%s': %s\n",
                   tests[t].input, tests[t].expected_word,
                   has_expected ? "YES ✓" : "NO ✗");
            
            if (has_expected) correct++;
            free(output);
        }
    }
    
    printf("\nAccuracy: %d/5 (%.0f%%)\n", correct, correct * 20.0f);
    if (correct >= 4) {
        printf("PASS: Excellent discrimination despite overlap!\n");
    } else if (correct >= 3) {
        printf("PARTIAL: Good discrimination\n");
    } else {
        printf("FAIL: Poor discrimination\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_overlap.m");
}

void test_memory_efficiency() {
    printf("\n=== Test 3: Memory Efficiency ===\n");
    
    remove("/tmp/test_memory.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_memory.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train many short patterns
    printf("Training 50 short patterns (3-5 chars each):\n");
    
    char pattern[6];
    for (int i = 0; i < 50; i++) {
        // Generate pattern like "p00", "p01", etc.
        snprintf(pattern, sizeof(pattern), "p%02d", i);
        
        for (int iter = 0; iter < 5; iter++) {
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern, strlen(pattern));
            melvin_m_universal_output_clear(mfile);
        }
        
        if (i == 9 || i == 24 || i == 49) {
            printf("  After %d patterns: Nodes=%zu, Edges=%zu\n",
                   i+1,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal stats:\n");
    printf("  Nodes: %zu\n", final_nodes);
    printf("  Edges: %zu\n", final_edges);
    printf("  Patterns trained: 50\n");
    
    // Expected: ~11 unique chars (p, 0-9) + hierarchies
    // If memory efficient, nodes should be << 50 * 3 = 150
    printf("  Naive storage: 150 nodes (50 patterns × 3 chars)\n");
    printf("  Actual: %zu nodes\n", final_nodes);
    
    float efficiency = 150.0f / (float)final_nodes;
    printf("  Efficiency: %.2fx better than naive\n", efficiency);
    
    if (efficiency > 2.0f) {
        printf("PASS: Good memory efficiency (reusing nodes)\n");
    } else if (efficiency > 1.5f) {
        printf("PARTIAL: Moderate efficiency\n");
    } else {
        printf("FAIL: Poor efficiency (not reusing nodes)\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_memory.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  SCALABILITY TEST                          ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_long_sequence();
    test_overlapping_patterns();
    test_memory_efficiency();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
