/*
 * Self-Supervised Learning Test
 * 
 * Tests Melvin's ability to learn through prediction error:
 * - Splits training sequences into prefix + continuation
 * - Generates prediction from prefix
 * - Compares prediction to actual continuation
 * - Applies error feedback
 * 
 * This mimics how LLMs learn: predict next tokens, compare to actual, adjust weights.
 */

#include "melvin.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Levenshtein distance for error calculation */
static size_t min3(size_t a, size_t b, size_t c) {
    size_t min = a;
    if (b < min) min = b;
    if (c < min) min = c;
    return min;
}

static float levenshtein_distance(const uint8_t *s1, size_t len1, 
                                   const uint8_t *s2, size_t len2) {
    if (len1 == 0) return (float)len2;
    if (len2 == 0) return (float)len1;
    
    size_t *prev_row = malloc((len2 + 1) * sizeof(size_t));
    size_t *curr_row = malloc((len2 + 1) * sizeof(size_t));
    
    for (size_t j = 0; j <= len2; j++) {
        prev_row[j] = j;
    }
    
    for (size_t i = 1; i <= len1; i++) {
        curr_row[0] = i;
        for (size_t j = 1; j <= len2; j++) {
            size_t cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            curr_row[j] = min3(
                prev_row[j] + 1,      // deletion
                curr_row[j-1] + 1,    // insertion
                prev_row[j-1] + cost  // substitution
            );
        }
        size_t *tmp = prev_row;
        prev_row = curr_row;
        curr_row = tmp;
    }
    
    float distance = (float)prev_row[len2];
    free(prev_row);
    free(curr_row);
    return distance;
}

/* Self-supervised training on a single sequence */
static void train_sequence_self_supervised(MelvinMFile *mfile, 
                                           const char *sequence,
                                           size_t *total_predictions,
                                           float *total_error) {
    size_t seq_len = strlen(sequence);
    if (seq_len < 2) return;  // Need at least 2 chars for prefix+continuation
    
    // Try different split points (prefix lengths)
    // Start from 1 (at least 1 char prefix) to seq_len-1 (at least 1 char continuation)
    for (size_t prefix_len = 1; prefix_len < seq_len; prefix_len++) {
        // 1. Process prefix as input (this also does Hebbian learning)
        melvin_m_universal_input_write(mfile, (const uint8_t*)sequence, prefix_len);
        int result = melvin_m_process_input(mfile);
        if (result != 0) continue;
        
        // SOLUTION 2: Strengthen correct continuation edges explicitly
        // This gives the correct path a head start (like supervised learning)
        melvin_m_strengthen_continuation(mfile, (const uint8_t*)sequence, prefix_len, seq_len);
        
        // 2. Generate prediction
        uint8_t prediction[256];
        size_t pred_len = melvin_m_universal_output_read(mfile, prediction, sizeof(prediction));
        
        // 3. Get actual continuation (ground truth from same sequence)
        const uint8_t *actual = (const uint8_t*)(sequence + prefix_len);
        size_t actual_len = seq_len - prefix_len;
        
        // 4. Compute error signal
        float error_signal = 0.0f;
        if (pred_len > 0 && actual_len > 0) {
            // Use Levenshtein distance for more accurate error
            float distance = levenshtein_distance(prediction, pred_len, actual, actual_len);
            size_t max_len = (pred_len > actual_len) ? pred_len : actual_len;
            float error_rate = distance / (float)max_len;
            error_signal = 1.0f - error_rate;  // Convert to success signal (1.0 = perfect)
            
            // Clamp to [0, 1]
            if (error_signal < 0.0f) error_signal = 0.0f;
            if (error_signal > 1.0f) error_signal = 1.0f;
            
            *total_error += error_rate;
            (*total_predictions)++;
        }
        
        // 5. Apply error feedback (ERROR-BASED LEARNING)
        melvin_m_feedback_error(mfile, error_signal);
        
        // Clear output for next iteration
        melvin_m_universal_output_clear(mfile);
    }
}

/* Test a query and return error rate */
static float test_query(MelvinMFile *mfile, 
                       const char *query, 
                       const char *expected_continuation) {
    // Process query
    melvin_m_universal_input_write(mfile, (const uint8_t*)query, strlen(query));
    int result = melvin_m_process_input(mfile);
    if (result != 0) return 1.0f;  // Failed to process
    
    // Get output
    uint8_t output[256];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    // Compute error
    float error_rate = 0.0f;
    if (output_len > 0 && strlen(expected_continuation) > 0) {
        float distance = levenshtein_distance(
            output, output_len,
            (const uint8_t*)expected_continuation, strlen(expected_continuation)
        );
        size_t max_len = (output_len > strlen(expected_continuation)) ? 
                         output_len : strlen(expected_continuation);
        error_rate = distance / (float)max_len;
    } else if (output_len == 0) {
        error_rate = 1.0f;  // No output = complete error
    }
    
    melvin_m_universal_output_clear(mfile);
    return error_rate;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m> [training_file]\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    const char *training_file = (argc > 2) ? argv[2] : NULL;
    
    // Load or create brain
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Failed to load/create brain: %s\n", brain_path);
        return 1;
    }
    
    // Training sequences
    const char *sequences[] = {
        "hello world",
        "the cat sat",
        "good morning"
    };
    size_t num_sequences = 3;
    
    // If training file provided, read from it
    char file_sequence[1024] = {0};
    if (training_file) {
        FILE *f = fopen(training_file, "r");
        if (f) {
            size_t bytes = fread(file_sequence, 1, sizeof(file_sequence) - 1, f);
            file_sequence[bytes] = '\0';
            fclose(f);
            
            // Remove trailing newline
            if (bytes > 0 && file_sequence[bytes-1] == '\n') {
                file_sequence[bytes-1] = '\0';
            }
            
            sequences[0] = file_sequence;
            num_sequences = 1;
        }
    }
    
    // Self-supervised training
    size_t total_predictions = 0;
    float total_error = 0.0f;
    
    printf("=== SELF-SUPERVISED TRAINING ===\n");
    for (size_t i = 0; i < num_sequences; i++) {
        printf("Training on: \"%s\"\n", sequences[i]);
        train_sequence_self_supervised(mfile, sequences[i], &total_predictions, &total_error);
    }
    
    float avg_training_error = (total_predictions > 0) ? 
                               (total_error / (float)total_predictions) : 1.0f;
    printf("Training predictions: %zu\n", total_predictions);
    printf("Average training error: %.4f\n\n", avg_training_error);
    
    // Test queries
    printf("=== TESTING QUERIES ===\n");
    
    struct {
        const char *query;
        const char *expected;
    } tests[] = {
        {"hello", " world"},
        {"the cat", " sat"},
        {"good", " morning"}
    };
    size_t num_tests = (num_sequences == 1) ? 1 : 3;
    
    float total_test_error = 0.0f;
    for (size_t i = 0; i < num_tests; i++) {
        printf("Query: \"%s\"\n", tests[i].query);
        printf("Expected: \"%s\"\n", tests[i].expected);
        
        float error = test_query(mfile, tests[i].query, tests[i].expected);
        
        // Show actual output
        melvin_m_universal_input_write(mfile, (const uint8_t*)tests[i].query, strlen(tests[i].query));
        melvin_m_process_input(mfile);
        uint8_t output[256];
        size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Output: \"");
        for (size_t j = 0; j < output_len; j++) {
            if (output[j] >= 32 && output[j] < 127) {
                printf("%c", output[j]);
            } else {
                printf("\\x%02x", output[j]);
            }
        }
        printf("\"\n");
        printf("Error rate: %.4f\n\n", error);
        
        total_test_error += error;
        melvin_m_universal_output_clear(mfile);
    }
    
    float avg_test_error = total_test_error / (float)num_tests;
    printf("Average test error: %.4f\n", avg_test_error);
    
    // Get graph stats
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    
    printf("\n=== GRAPH STATISTICS ===\n");
    printf("Nodes: %zu\n", node_count);
    printf("Edges: %zu\n", edge_count);
    
    // Save and close
    melvin_m_save(mfile);
    melvin_m_close(mfile);
    
    // Output CSV format for script parsing
    printf("\n=== CSV OUTPUT ===\n");
    printf("training_error,test_error,predictions,nodes,edges\n");
    printf("%.4f,%.4f,%zu,%zu,%zu\n", 
           avg_training_error, avg_test_error, total_predictions,
           node_count, edge_count);
    
    return 0;
}

