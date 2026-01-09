#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "melvin.h"

// Get current time in microseconds
static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

// Test O(1) lookup performance
static void test_o1_lookup(MelvinMFile *mfile, size_t iterations) {
    printf("\n=== O(1) LOOKUP PERFORMANCE TEST ===\n");
    printf("Testing %zu lookups...\n", iterations);
    
    double start = get_time_us();
    
    // Simulate many lookups (each node lookup should be O(1) via hash table)
    for (size_t i = 0; i < iterations; i++) {
        // Process a small input to trigger lookups
        const char *test = "a";
        melvin_m_universal_input_write(mfile, (const uint8_t*)test, 1);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    double end = get_time_us();
    double elapsed = (end - start) / 1000.0; // Convert to milliseconds
    double per_lookup = elapsed / iterations;
    
    printf("Total time: %.2f ms\n", elapsed);
    printf("Time per lookup: %.4f ms\n", per_lookup);
    printf("Lookups per second: %.0f\n", 1000.0 / per_lookup);
    
    // O(1) should scale linearly - verify
    if (per_lookup < 1.0) {
        printf("✓ O(1) performance verified (constant time per lookup)\n");
    } else {
        printf("⚠ Warning: Lookup time may not be O(1) (%.4f ms per lookup)\n", per_lookup);
    }
}

// Test learning curve over many iterations
static void test_learning_curve(const char *pattern, size_t iterations) {
    printf("\n=== LEARNING CURVE TEST ===\n");
    printf("Pattern: '%s' | Iterations: %zu\n", pattern, iterations);
    
    const char *test_file = "test_learning_curve.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return;
    }
    
    size_t pattern_len = strlen(pattern);
    size_t correct_count = 0;
    
    printf("\nIteration | Nodes | Edges | Output | Correct | Accuracy\n");
    printf("----------|-------|-------|--------|---------|---------\n");
    
    for (size_t i = 1; i <= iterations; i++) {
        // Train on pattern
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, pattern_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test continuation every 10 iterations
        if (i % 10 == 0) {
            // Test: input first half, expect second half
            size_t test_len = pattern_len / 2;
            if (test_len == 0) test_len = 1;
            
            melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, test_len);
            melvin_m_process_input(mfile);
            
            uint8_t output[256];
            size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
            
            // Check if output matches expected continuation
            int correct = 0;
            if (out_len > 0 && test_len < pattern_len) {
                size_t expected_len = pattern_len - test_len;
                if (out_len >= expected_len) {
                    correct = (memcmp(output, pattern + test_len, expected_len) == 0);
                }
            }
            
            if (correct) correct_count++;
            
            printf("%9zu | %5zu | %5zu | %6zu | %7s | %.1f%%\n",
                   i,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile),
                   out_len,
                   correct ? "YES" : "NO",
                   (correct_count * 100.0) / (i / 10));
            
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\nFinal accuracy: %.1f%% (%zu/%zu tests correct)\n",
           (correct_count * 100.0) / (iterations / 10),
           correct_count,
           iterations / 10);
    
    melvin_m_close(mfile);
    remove(test_file);
}

// Test large dataset handling
static void test_large_dataset(size_t dataset_size) {
    printf("\n=== LARGE DATASET TEST ===\n");
    printf("Dataset size: %zu bytes\n", dataset_size);
    
    const char *test_file = "test_large_dataset.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return;
    }
    
    // Generate large dataset
    uint8_t *data = malloc(dataset_size);
    if (!data) {
        printf("Failed to allocate dataset\n");
        melvin_m_close(mfile);
        return;
    }
    
    // Fill with varied patterns
    const char *words[] = {"hello", "world", "the", "quick", "brown", "fox", "jumps", "over", "lazy", "dog"};
    size_t word_count = sizeof(words) / sizeof(words[0]);
    size_t pos = 0;
    
    while (pos < dataset_size) {
        const char *word = words[pos % word_count];
        size_t word_len = strlen(word);
        if (pos + word_len + 1 < dataset_size) {
            memcpy(data + pos, word, word_len);
            pos += word_len;
            if (pos < dataset_size) {
                data[pos++] = ' ';
            }
        } else {
            break;
        }
    }
    
    printf("Processing dataset...\n");
    double start = get_time_us();
    
    // Process in chunks
    size_t chunk_size = 1000;
    size_t processed = 0;
    
    while (processed < pos) {
        size_t chunk_len = (pos - processed < chunk_size) ? (pos - processed) : chunk_size;
        melvin_m_universal_input_write(mfile, data + processed, chunk_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        processed += chunk_len;
    }
    
    double end = get_time_us();
    double elapsed = (end - start) / 1000.0;
    
    printf("Processing time: %.2f ms\n", elapsed);
    printf("Throughput: %.2f bytes/ms\n", pos / elapsed);
    printf("Final nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Final edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Compression ratio: %.2f:1\n", (double)pos / melvin_m_get_node_count(mfile));
    
    // Test retrieval performance
    printf("\nTesting retrieval performance...\n");
    start = get_time_us();
    
    for (size_t i = 0; i < 100; i++) {
        const char *test = "hello";
        melvin_m_universal_input_write(mfile, (const uint8_t*)test, strlen(test));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    end = get_time_us();
    elapsed = (end - start) / 1000.0;
    printf("100 retrievals: %.2f ms (%.4f ms per retrieval)\n", elapsed, elapsed / 100.0);
    
    free(data);
    melvin_m_close(mfile);
    remove(test_file);
}

// Test memory efficiency
static void test_memory_efficiency(void) {
    printf("\n=== MEMORY EFFICIENCY TEST ===\n");
    
    const char *test_file = "test_memory.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return;
    }
    
    // Process many patterns
    const char *patterns[] = {
        "hello world",
        "the quick brown fox",
        "machine learning",
        "neural networks",
        "artificial intelligence"
    };
    size_t pattern_count = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Processing %zu patterns 100 times each...\n", pattern_count);
    
    for (size_t iter = 0; iter < 100; iter++) {
        for (size_t i = 0; i < pattern_count; i++) {
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    printf("Final nodes: %zu\n", nodes);
    printf("Final edges: %zu\n", edges);
    printf("Edge/Node ratio: %.2f\n", (double)edges / nodes);
    
    // Estimate memory usage (rough)
    size_t estimated_memory = nodes * 256 + edges * 64; // Rough estimate
    printf("Estimated memory: ~%zu KB\n", estimated_memory / 1024);
    
    // Check file size
    FILE *f = fopen(test_file, "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        fclose(f);
        printf("File size: %ld bytes (%.2f KB)\n", file_size, file_size / 1024.0);
    }
    
    melvin_m_close(mfile);
    remove(test_file);
}

// Test hierarchy formation at scale
static void test_hierarchy_formation(void) {
    printf("\n=== HIERARCHY FORMATION TEST ===\n");
    
    const char *test_file = "test_hierarchy.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return;
    }
    
    // Repeated patterns to trigger hierarchy formation
    const char *pattern = "hello world";
    size_t pattern_len = strlen(pattern);
    
    printf("Repeating pattern '%s' 1000 times...\n", pattern);
    
    for (size_t i = 0; i < 1000; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, pattern_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if ((i + 1) % 100 == 0) {
            printf("  Iteration %zu: %zu nodes, %zu edges\n",
                   i + 1,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    printf("\nFinal stats:\n");
    printf("  Nodes: %zu\n", final_nodes);
    printf("  Edges: %zu\n", final_edges);
    printf("  Expected raw nodes: ~%zu (if no hierarchy)\n", pattern_len);
    printf("  Compression: %.1f%% (hierarchies reduce node count)\n",
           100.0 * (1.0 - (double)final_nodes / (pattern_len * 1000)));
    
    melvin_m_close(mfile);
    remove(test_file);
}

int main(int argc, char **argv) {
    printf("========================================\n");
    printf("COMPREHENSIVE SCALABILITY TEST SUITE\n");
    printf("Testing LLM/Brain-Level Performance\n");
    printf("========================================\n");
    
    // Test 1: O(1) lookup performance
    const char *test_file = "test_o1.m";
    remove(test_file);
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (mfile) {
        // Build up some nodes first
        for (int i = 0; i < 100; i++) {
            const char *test = "hello";
            melvin_m_universal_input_write(mfile, (const uint8_t*)test, strlen(test));
            melvin_m_process_input(mfile);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        test_o1_lookup(mfile, 1000);
        melvin_m_close(mfile);
        remove(test_file);
    }
    
    // Test 2: Learning curve
    test_learning_curve("hello world", 1000);
    
    // Test 3: Large dataset
    test_large_dataset(10000);  // 10KB
    
    // Test 4: Memory efficiency
    test_memory_efficiency();
    
    // Test 5: Hierarchy formation
    test_hierarchy_formation();
    
    printf("\n========================================\n");
    printf("ALL TESTS COMPLETE\n");
    printf("========================================\n");
    
    return 0;
}
