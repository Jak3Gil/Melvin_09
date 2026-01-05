/*
 * Test Self-Modification Mechanisms
 * 
 * Tests:
 * 1. Meta-learning: bias adaptation based on outcomes
 * 2. Self-optimization: boosting weak nodes on errors
 * 3. Extended context: state accumulation for long-range dependencies
 * 4. Aggressive hierarchies: faster hierarchy formation
 * 
 * Expected: Lower error rates, faster hierarchy formation, improved output quality
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "melvin.h"

/* Calculate simple error rate (Levenshtein-like) */
static float calculate_error_rate(const char *expected, const char *actual, size_t actual_len) {
    if (!expected || !actual || actual_len == 0) return 1.0f;
    
    size_t expected_len = strlen(expected);
    if (expected_len == 0) return (actual_len > 0) ? 1.0f : 0.0f;
    
    int matches = 0;
    size_t compare_len = (expected_len < actual_len) ? expected_len : actual_len;
    
    for (size_t i = 0; i < compare_len; i++) {
        if (expected[i] == actual[i]) matches++;
    }
    
    float match_rate = (float)matches / (float)expected_len;
    return 1.0f - match_rate;
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    
    printf("=========================================\n");
    printf("MELVIN SELF-MODIFICATION TEST\n");
    printf("=========================================\n\n");
    
    // Clean up old test file
    remove("test_self_mod.m");
    
    // Create fresh brain
    MelvinMFile *brain = melvin_m_create("test_self_mod.m");
    if (!brain) {
        fprintf(stderr, "ERROR: Failed to create brain\n");
        return 1;
    }
    
    // Training sequences
    const char *sequences[] = {
        "hello world",
        "hello there",
        "hello friend",
        "good morning",
        "good evening",
        "the cat sat",
        "the dog ran"
    };
    int num_sequences = sizeof(sequences) / sizeof(sequences[0]);
    
    printf("PHASE 1: Training with %d sequences\n", num_sequences);
    printf("-----------------------------------------\n");
    
    // Training phase
    for (int iter = 0; iter < 50; iter++) {
        for (int s = 0; s < num_sequences; s++) {
            const char *seq = sequences[s];
            size_t len = strlen(seq);
            
            melvin_m_universal_input_write(brain, (const uint8_t *)seq, len);
            melvin_m_process_input(brain);
            melvin_m_universal_output_clear(brain);  // Clear output after training
            
            // Self-supervised: strengthen correct continuation
            melvin_m_strengthen_continuation(brain, (const uint8_t *)seq, 5, len);
        }
        
        if ((iter + 1) % 10 == 0) {
            printf("  Iteration %d: nodes=%zu, edges=%zu\n",
                   iter + 1,
                   melvin_m_get_node_count(brain),
                   melvin_m_get_edge_count(brain));
        }
    }
    
    printf("\nPHASE 2: Testing Output Quality\n");
    printf("-----------------------------------------\n");
    
    // Test queries
    const char *queries[] = {"hello", "good", "the"};
    const char *expected[] = {" world", " morning", " cat"};  // Expected first output
    int num_queries = sizeof(queries) / sizeof(queries[0]);
    
    float total_error = 0.0f;
    
    for (int q = 0; q < num_queries; q++) {
        const char *query = queries[q];
        const char *expect = expected[q];
        
        // Send query
        melvin_m_universal_input_write(brain, (const uint8_t *)query, strlen(query));
        melvin_m_process_input(brain);
        
        // Get output
        size_t output_size = melvin_m_universal_output_size(brain);
        uint8_t output[256] = {0};
        if (output_size > 0) {
            melvin_m_universal_output_read(brain, output, sizeof(output) - 1);
        }
        
        // Calculate error
        float error = calculate_error_rate(expect, (char *)output, output_size);
        total_error += error;
        
        printf("  Query: '%s'\n", query);
        printf("  Expected: '%s'\n", expect);
        printf("  Got: '");
        for (size_t i = 0; i < output_size && i < 30; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf("?");
            }
        }
        printf("' (len=%zu)\n", output_size);
        printf("  Error: %.2f%%\n\n", error * 100.0f);
        
        // Feedback error for meta-learning
        float error_signal = 1.0f - error;  // Convert to success signal
        melvin_m_feedback_error(brain, error_signal);
        
        melvin_m_universal_output_clear(brain);
    }
    
    float avg_error = total_error / (float)num_queries;
    printf("Average Error: %.2f%%\n\n", avg_error * 100.0f);
    
    printf("PHASE 3: Testing Meta-Learning Improvement\n");
    printf("-----------------------------------------\n");
    
    // More training with error feedback
    for (int round = 0; round < 3; round++) {
        printf("\nRound %d:\n", round + 1);
        
        // Train more
        for (int iter = 0; iter < 20; iter++) {
            for (int s = 0; s < num_sequences; s++) {
                const char *seq = sequences[s];
                size_t len = strlen(seq);
                
                melvin_m_universal_input_write(brain, (const uint8_t *)seq, len);
                melvin_m_process_input(brain);
                melvin_m_universal_output_clear(brain);  // Clear output after training
                melvin_m_strengthen_continuation(brain, (const uint8_t *)seq, 5, len);
            }
        }
        
        // Test again
        float round_error = 0.0f;
        for (int q = 0; q < num_queries; q++) {
            const char *query = queries[q];
            const char *expect = expected[q];
            
            melvin_m_universal_input_write(brain, (const uint8_t *)query, strlen(query));
            melvin_m_process_input(brain);
            
            size_t output_size = melvin_m_universal_output_size(brain);
            uint8_t output[256] = {0};
            if (output_size > 0) {
                melvin_m_universal_output_read(brain, output, sizeof(output) - 1);
            }
            
            float error = calculate_error_rate(expect, (char *)output, output_size);
            round_error += error;
            
            // Meta-learning feedback
            melvin_m_feedback_error(brain, 1.0f - error);
            melvin_m_universal_output_clear(brain);
            
            printf("  '%s' -> '", query);
            for (size_t i = 0; i < output_size && i < 15; i++) {
                if (output[i] >= 32 && output[i] < 127) printf("%c", output[i]);
            }
            printf("' (error: %.0f%%)\n", error * 100.0f);
        }
        
        float round_avg = round_error / (float)num_queries;
        printf("  Round %d Average Error: %.2f%%\n", round + 1, round_avg * 100.0f);
    }
    
    printf("\n=========================================\n");
    printf("FINAL STATISTICS\n");
    printf("=========================================\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(brain));
    printf("Edges: %zu\n", melvin_m_get_edge_count(brain));
    printf("Adaptations: %llu\n", (unsigned long long)melvin_m_get_adaptation_count(brain));
    
    // Close
    melvin_m_close(brain);
    
    printf("\n✅ Test complete!\n");
    printf("\nLook for:\n");
    printf("  - Error rate should decrease over rounds (meta-learning)\n");
    printf("  - Hierarchies should form (check stderr for [HIERARCHY] messages)\n");
    printf("  - Output should become more coherent\n");
    
    return 0;
}

