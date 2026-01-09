#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../src/melvin.h"

int main() {
    printf("=== Long Sequence Scalability Test ===\n\n");
    
    // Create a long repeating pattern (200 bytes)
    const char *base_pattern = "The quick brown fox jumps over the lazy dog. ";
    size_t base_len = strlen(base_pattern);
    
    // Repeat pattern 4 times to get ~200 bytes
    size_t total_len = base_len * 4;
    char *long_pattern = malloc(total_len + 1);
    if (!long_pattern) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }
    
    for (size_t i = 0; i < 4; i++) {
        memcpy(long_pattern + i * base_len, base_pattern, base_len);
    }
    long_pattern[total_len] = '\0';
    
    printf("Training on long sequence (%zu bytes):\n", total_len);
    printf("Pattern: \"%s...\"\n\n", base_pattern);
    
    // Create Melvin instance
    MelvinMFile *mfile = melvin_m_create("test_long.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create Melvin instance\n");
        free(long_pattern);
        return 1;
    }
    
    // Train on long pattern (10 iterations)
    printf("Training for 10 iterations...\n");
    clock_t start_train = clock();
    
    for (int iter = 0; iter < 10; iter++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)long_pattern, total_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if ((iter + 1) % 5 == 0) {
            printf("  Iteration %d complete\n", iter + 1);
        }
    }
    
    clock_t end_train = clock();
    double train_time = ((double)(end_train - start_train)) / CLOCKS_PER_SEC;
    
    printf("\nTraining complete in %.2f seconds\n", train_time);
    printf("Graph stats:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    // Test generation with first 50 bytes as input
    printf("\n=== Generation Test ===\n");
    printf("Input: first 50 bytes of pattern\n");
    printf("Expected: continuation of pattern\n\n");
    
    melvin_m_universal_input_write(mfile, (const uint8_t*)long_pattern, 50);
    
    clock_t start_gen = clock();
    melvin_m_process_input(mfile);
    clock_t end_gen = clock();
    
    double gen_time = ((double)(end_gen - start_gen)) / CLOCKS_PER_SEC;
    
    // Get output
    size_t output_len = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_len);
    if (output) {
        melvin_m_universal_output_read(mfile, output, output_len);
    }
    
    printf("Generated %zu bytes in %.3f seconds\n", output_len, gen_time);
    printf("Generation speed: %.0f bytes/sec\n", output_len / gen_time);
    
    if (output && output_len > 0) {
        printf("\nOutput (first 100 bytes):\n\"");
        for (size_t i = 0; i < output_len && i < 100; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf(".");
            }
        }
        printf("\"\n");
        
        // Check if output matches expected continuation
        size_t match_count = 0;
        size_t check_len = (output_len < 50) ? output_len : 50;
        for (size_t i = 0; i < check_len; i++) {
            if (output[i] == (uint8_t)long_pattern[50 + i]) {
                match_count++;
            }
        }
        
        float match_rate = (float)match_count / (float)check_len * 100.0f;
        printf("\nMatch rate (first 50 bytes): %.1f%%\n", match_rate);
        
        if (match_rate > 50.0f) {
            printf("✅ PASS: Good continuation\n");
        } else {
            printf("❌ FAIL: Poor continuation\n");
        }
    }
    
    // Performance analysis
    printf("\n=== Performance Analysis ===\n");
    printf("Training time: %.2f seconds for %zu bytes\n", train_time, total_len * 10);
    printf("Training speed: %.0f bytes/sec\n", (total_len * 10) / train_time);
    printf("Generation time: %.3f seconds for %zu bytes\n", gen_time, output_len);
    printf("Generation speed: %.0f bytes/sec\n", output_len / gen_time);
    
    // Memory usage estimate
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    size_t est_memory = node_count * 1024 + edge_count * 256;  // Rough estimate
    printf("\nEstimated memory usage: %.2f MB\n", est_memory / (1024.0 * 1024.0));
    
    printf("\n=== Scalability Assessment ===\n");
    if (gen_time < 1.0 && output_len > 100) {
        printf("✅ EXCELLENT: Fast generation for long sequences\n");
    } else if (gen_time < 5.0) {
        printf("✅ GOOD: Acceptable generation speed\n");
    } else {
        printf("⚠️  SLOW: Generation may not scale well\n");
    }
    
    // Cleanup
    if (output) free(output);
    melvin_m_close(mfile);
    free(long_pattern);
    remove("test_long.m");
    
    printf("\n=== Test Complete ===\n");
    
    return 0;
}
