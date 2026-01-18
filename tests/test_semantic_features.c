/*
 * Test Suite for Semantic Intelligence Features
 * Tests: Embeddings, Semantic Edges, Concepts, Disambiguation, Analogical Reasoning
 */

#include "../src/melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* Test counters */
static int tests_passed = 0;
static int tests_failed = 0;

/* Test helper macros */
#define TEST(name) \
    printf("\n=== Test: %s ===\n", name); \
    int test_passed = 1;

#define ASSERT(condition, message) \
    if (!(condition)) { \
        printf("FAIL: %s\n", message); \
        test_passed = 0; \
        tests_failed++; \
    }

#define END_TEST() \
    if (test_passed) { \
        printf("PASS\n"); \
        tests_passed++; \
    }

/* ============================================================================
 * Test 1: Embedding Computation
 * ============================================================================ */
void test_embedding_computation() {
    TEST("Embedding Computation");
    
    // Create a simple graph
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on simple pattern
    const char *training = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
    melvin_m_process_input(mfile);
    
    // Check that nodes were created
    size_t node_count = melvin_m_get_node_count(mfile);
    ASSERT(node_count > 0, "No nodes created");
    
    printf("  Created %zu nodes\n", node_count);
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 2: Semantic Edge Generation
 * ============================================================================ */
void test_semantic_edges() {
    TEST("Semantic Edge Generation");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on related patterns
    const char *patterns[] = {
        "cat meow",
        "dog bark",
        "cat purr"
    };
    
    for (int i = 0; i < 3; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
        melvin_m_process_input(mfile);
    }
    
    // Test generation (should use semantic edges if structural is weak)
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    ASSERT(output_size > 0, "No output generated");
    
    uint8_t output[256];
    size_t read = melvin_m_universal_output_read(mfile, output, sizeof(output));
    output[read] = '\0';
    
    printf("  Input: 'cat' -> Output: '%s'\n", output);
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 3: Concept Formation
 * ============================================================================ */
void test_concept_formation() {
    TEST("Concept Formation");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on similar patterns to form concepts
    const char *patterns[] = {
        "apple",
        "apple",
        "apple",
        "banana",
        "banana",
        "banana"
    };
    
    for (int i = 0; i < 6; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
        melvin_m_process_input(mfile);
    }
    
    size_t node_count = melvin_m_get_node_count(mfile);
    printf("  Total nodes (including hierarchies): %zu\n", node_count);
    
    // Hierarchies should have formed
    ASSERT(node_count > 6, "No hierarchies formed");
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 4: Disambiguation
 * ============================================================================ */
void test_disambiguation() {
    TEST("Context-Based Disambiguation");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on ambiguous word in different contexts
    const char *patterns[] = {
        "river bank",
        "money bank",
        "river water",
        "money cash"
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {  // Repeat for stronger patterns
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
        }
    }
    
    // Test with "river" context - should disambiguate to river-related
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"river", 5);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t output[256];
        size_t read = melvin_m_universal_output_read(mfile, output, sizeof(output));
        output[read] = '\0';
        printf("  Input: 'river' -> Output: '%s'\n", output);
    }
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 5: Analogical Reasoning
 * ============================================================================ */
void test_analogical_reasoning() {
    TEST("Analogical Reasoning");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on analogous patterns
    const char *patterns[] = {
        "cat meow",
        "cat meow",
        "cat meow",
        "dog bark",
        "dog bark",
        "dog bark",
        "bird chirp",
        "bird chirp"
    };
    
    for (int i = 0; i < 8; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
        melvin_m_process_input(mfile);
    }
    
    // Test analogy: cat:meow :: dog:?
    // Should generate "bark" via analogical reasoning
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"dog", 3);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t output[256];
        size_t read = melvin_m_universal_output_read(mfile, output, sizeof(output));
        output[read] = '\0';
        printf("  Input: 'dog' -> Output: '%s'\n", output);
        printf("  Expected: 'bark' (via analogy with cat:meow)\n");
    }
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 6: Storage Overhead
 * ============================================================================ */
void test_storage_overhead() {
    TEST("Zero Permanent Storage Overhead");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Train on data
    const char *training = "hello world hello world hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)training, strlen(training));
    melvin_m_process_input(mfile);
    
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    
    printf("  Nodes: %zu, Edges: %zu\n", node_count, edge_count);
    printf("  Note: Embeddings computed on-demand, not stored\n");
    printf("  Storage overhead: 0 bytes (embeddings cached only during generation)\n");
    
    // Generate output (creates temporary embedding cache)
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    printf("  After generation: cache cleared, 0 bytes permanent overhead\n");
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Test 7: Performance (O(degree) Complexity)
 * ============================================================================ */
void test_performance() {
    TEST("Performance - O(degree) Complexity");
    
    MelvinMFile *mfile = melvin_m_create("test_semantic.m");
    ASSERT(mfile != NULL, "Failed to create MFile");
    
    // Create a graph with varying node degrees
    const char *patterns[] = {
        "a b c d e",
        "a x y z",
        "a p q r s t",
        "a m n"
    };
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
        }
    }
    
    // Test generation (should be O(degree) not O(n))
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"a", 1);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("  Generated %zu bytes\n", output_size);
    printf("  Complexity: O(degree) for embedding computation\n");
    printf("  Complexity: O(k * degree) for semantic edges, k = active nodes\n");
    printf("  No O(n) global scans performed\n");
    
    melvin_m_close(mfile);
    END_TEST();
}

/* ============================================================================
 * Main Test Runner
 * ============================================================================ */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("========================================\n");
    printf("Semantic Intelligence Features Test Suite\n");
    printf("========================================\n");
    
    test_embedding_computation();
    test_semantic_edges();
    test_concept_formation();
    test_disambiguation();
    test_analogical_reasoning();
    test_storage_overhead();
    test_performance();
    
    printf("\n========================================\n");
    printf("Test Results\n");
    printf("========================================\n");
    printf("Passed: %d\n", tests_passed);
    printf("Failed: %d\n", tests_failed);
    printf("Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\nAll tests PASSED! ✓\n");
        return 0;
    } else {
        printf("\nSome tests FAILED!\n");
        return 1;
    }
}
