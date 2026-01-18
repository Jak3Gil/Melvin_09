/*
 * Complex Association Learning & Scalability Test
 * Tests if the system can learn multiple associations in one graph
 * and maintain accuracy as the graph scales
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

// Calculate accuracy (simplified Levenshtein-based)
float calculate_accuracy(const uint8_t *output, size_t output_len, const char *expected) {
    if (!expected || strlen(expected) == 0) return 0.0f;
    if (output_len == 0) return 0.0f;
    
    // Simple prefix matching
    size_t expected_len = strlen(expected);
    size_t match_len = (output_len < expected_len) ? output_len : expected_len;
    
    int matches = 0;
    for (size_t i = 0; i < match_len; i++) {
        if (output[i] == expected[i]) {
            matches++;
        } else {
            break;
        }
    }
    
    return (float)matches / (float)expected_len;
}

// Check for loops
int has_loop(const uint8_t *output, size_t len) {
    if (len < 4) return 0;
    
    // Check for 2-char repeating patterns
    for (size_t i = 0; i < len - 3 && i < len; i++) {
        if (output[i] == output[i+2] && output[i+1] == output[i+3]) {
            int repeat_count = 1;
            for (size_t j = i + 2; j < len - 1 && j < len; j += 2) {
                if (j + 1 < len && output[i] == output[j] && output[i+1] == output[j+1]) {
                    repeat_count++;
                } else {
                    break;
                }
            }
            if (repeat_count >= 3) return 1;
        }
    }
    
    // Check for single char repetition (4+ times)
    if (len >= 4) {
        for (size_t i = 0; i < len - 3 && i < len; i++) {
            if (i + 3 < len &&
                output[i] == output[i+1] && 
                output[i] == output[i+2] && 
                output[i] == output[i+3]) {
                return 1;
            }
        }
    }
    
    return 0;
}

typedef struct {
    const char *pattern;
    const char *query;
    const char *expected;
    const char *description;
} Association;

int main() {
    printf("=== Complex Association Learning & Scalability Test ===\n");
    printf("Testing multiple associations in one graph with scaling\n\n");
    
    const char *test_file = "test_association_scaling.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        fprintf(stderr, "ERROR: Failed to create brain file\n");
        return 1;
    }
    
    // Define associations to learn (all in one graph)
    Association associations[] = {
        {.pattern = "hello world", .query = "hello", .expected = " world", .description = "Basic greeting"},
        {.pattern = "the quick brown", .query = "the quick", .expected = " brown", .description = "Color association"},
        {.pattern = "test testing", .query = "test", .expected = "ing", .description = "Word form"},
        {.pattern = "cat meow", .query = "cat", .expected = " meow", .description = "Animal sound"},
        {.pattern = "dog bark", .query = "dog", .expected = " bark", .description = "Animal sound 2"},
        {.pattern = "red apple", .query = "red", .expected = " apple", .description = "Color object"},
        {.pattern = "blue sky", .query = "blue", .expected = " sky", .description = "Color object 2"},
        {.pattern = "fast car", .query = "fast", .expected = " car", .description = "Adjective noun"},
        {.pattern = "slow turtle", .query = "slow", .expected = " turtle", .description = "Adjective noun 2"},
        {.pattern = "big house", .query = "big", .expected = " house", .description = "Size object"},
        {.pattern = "small mouse", .query = "small", .expected = " mouse", .description = "Size object 2"},
        {.pattern = "hot fire", .query = "hot", .expected = " fire", .description = "Temperature object"},
        {.pattern = "cold ice", .query = "cold", .expected = " ice", .description = "Temperature object 2"},
        {.pattern = "happy smile", .query = "happy", .expected = " smile", .description = "Emotion action"},
        {.pattern = "sad frown", .query = "sad", .expected = " frown", .description = "Emotion action 2"},
        {.pattern = "run fast", .query = "run", .expected = " fast", .description = "Action adverb"},
        {.pattern = "walk slow", .query = "walk", .expected = " slow", .description = "Action adverb 2"},
        {.pattern = "eat food", .query = "eat", .expected = " food", .description = "Action object"},
        {.pattern = "drink water", .query = "drink", .expected = " water", .description = "Action object 2"},
        {.pattern = "read book", .query = "read", .expected = " book", .description = "Action object 3"}
    };
    
    int num_associations = sizeof(associations) / sizeof(associations[0]);
    
    printf("Learning %d associations in one graph...\n\n", num_associations);
    
    // Phase 1: Learn all associations
    printf("=== Phase 1: Learning Associations ===\n");
    for (int i = 0; i < num_associations; i++) {
        // Train each association multiple times
        for (int j = 0; j < 10; j++) {
            melvin_m_universal_input_write(mfile, 
                (const uint8_t*)associations[i].pattern, 
                strlen(associations[i].pattern));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        if ((i + 1) % 5 == 0) {
            printf("Learned %d/%d associations (nodes: %zu, edges: %zu)\n", 
                   i + 1, num_associations,
                   melvin_m_get_node_count(mfile),
                   melvin_m_get_edge_count(mfile));
        }
    }
    
    printf("\nFinal graph size: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Phase 2: Test all associations for accuracy
    printf("=== Phase 2: Testing Accuracy ===\n");
    int passed = 0;
    int total = 0;
    int loops_detected = 0;
    float total_accuracy = 0.0f;
    float min_accuracy = 1.0f;
    float max_accuracy = 0.0f;
    
    for (int i = 0; i < num_associations; i++) {
        melvin_m_universal_input_write(mfile, 
            (const uint8_t*)associations[i].query, 
            strlen(associations[i].query));
        melvin_m_process_input(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_size + 1);
        if (!output) {
            fprintf(stderr, "ERROR: Failed to allocate output buffer\n");
            continue;
        }
        
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        if (read > output_size) read = output_size;
        if (read > 0 && read <= output_size) {
            output[read] = '\0';
        } else {
            read = 0;
            output[0] = '\0';
        }
        
        // Check for loops
        int has_loop_flag = has_loop(output, read);
        if (has_loop_flag) {
            loops_detected++;
        }
        
        // Calculate accuracy
        float accuracy = calculate_accuracy(output, read, associations[i].expected);
        total_accuracy += accuracy;
        if (accuracy < min_accuracy) min_accuracy = accuracy;
        if (accuracy > max_accuracy) max_accuracy = accuracy;
        
        // Consider passed if no loop and some accuracy
        int test_passed = (!has_loop_flag && accuracy > 0.0f);
        if (test_passed) passed++;
        total++;
        
        printf("Test %2d: %-20s | Query: '%-10s' | Output: '%-15s' | Accuracy: %5.1f%% | Loop: %s\n",
               i + 1,
               associations[i].description,
               associations[i].query,
               read > 0 ? (char*)output : "(empty)",
               accuracy * 100.0f,
               has_loop_flag ? "YES ❌" : "NO ✅");
        
        free(output);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Phase 3: Test interference (query similar patterns)
    printf("\n=== Phase 3: Testing Interference ===\n");
    printf("Testing if similar queries cause confusion...\n\n");
    
    struct {
        const char *query;
        const char *expected_prefix;
        const char *description;
    } interference_tests[] = {
        {.query = "hello", .expected_prefix = " world", .description = "After learning many patterns"},
        {.query = "test", .expected_prefix = "ing", .description = "Word form still works"},
        {.query = "cat", .expected_prefix = " meow", .description = "Animal sound still works"},
        {.query = "red", .expected_prefix = " apple", .description = "Color object still works"}
    };
    
    int interference_passed = 0;
    int interference_total = 0;
    
    for (int i = 0; i < sizeof(interference_tests) / sizeof(interference_tests[0]); i++) {
        melvin_m_universal_input_write(mfile, 
            (const uint8_t*)interference_tests[i].query, 
            strlen(interference_tests[i].query));
        melvin_m_process_input(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_size + 1);
        if (!output) continue;
        
        size_t read = melvin_m_universal_output_read(mfile, output, output_size);
        if (read > output_size) read = output_size;
        if (read > 0 && read <= output_size) {
            output[read] = '\0';
        } else {
            read = 0;
            output[0] = '\0';
        }
        
        int has_loop_flag = has_loop(output, read);
        float accuracy = calculate_accuracy(output, read, interference_tests[i].expected_prefix);
        
        int test_passed = (!has_loop_flag && accuracy > 0.3f);  // Lower threshold for interference
        if (test_passed) interference_passed++;
        interference_total++;
        
        printf("Interference %d: %-30s | Output: '%-15s' | Accuracy: %5.1f%% | Loop: %s\n",
               i + 1,
               interference_tests[i].description,
               read > 0 ? (char*)output : "(empty)",
               accuracy * 100.0f,
               has_loop_flag ? "YES ❌" : "NO ✅");
        
        free(output);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Final results
    printf("\n=== Final Results ===\n");
    printf("Graph size: %zu nodes, %zu edges\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    printf("\nAssociation Tests:\n");
    printf("  Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0f) / total);
    printf("  Loops detected: %d\n", loops_detected);
    printf("  Average accuracy: %.1f%%\n", (total_accuracy / total) * 100.0f);
    printf("  Min accuracy: %.1f%%\n", min_accuracy * 100.0f);
    printf("  Max accuracy: %.1f%%\n", max_accuracy * 100.0f);
    printf("\nInterference Tests:\n");
    printf("  Passed: %d/%d (%.1f%%)\n", 
           interference_passed, interference_total,
           (interference_passed * 100.0f) / interference_total);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    // Success criteria
    int success = (loops_detected == 0 && passed >= total * 0.7f);  // 70% accuracy threshold
    
    if (success) {
        printf("\n✅ TEST PASSED: System scales and maintains accuracy\n");
        return 0;
    } else {
        printf("\n❌ TEST FAILED: Accuracy or loop issues detected\n");
        return 1;
    }
}
