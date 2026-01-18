/*
 * Test: Error Rate with New Edge Direction Rules
 * 
 * Measures error rate for simple and complex associations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/melvin.h"

float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    if (!expected) return 1.0f;
    size_t expected_len = strlen(expected);
    size_t errors = 0;
    size_t max_len = (actual_len > expected_len) ? actual_len : expected_len;
    
    for (size_t i = 0; i < max_len; i++) {
        if (i >= actual_len || i >= expected_len) {
            errors++;
        } else if (actual[i] != (uint8_t)expected[i]) {
            errors++;
        }
    }
    
    return (max_len > 0) ? ((float)errors / (float)max_len) : 1.0f;
}

int main(void) {
    printf("=== Error Rate Analysis (New Edge Direction) ===\n\n");
    
    // Test 1: Simple single association
    printf("TEST 1: Simple Association (cat -> meow)\n");
    printf("------------------------------------------------\n");
    
    MelvinMFile *mfile1 = melvin_m_create("test_error_simple.m");
    if (!mfile1) {
        printf("FAIL: Could not create .m file\n");
        return 1;
    }
    
    const char *pattern1 = "cat meow";
    const char *input1 = "cat";
    const char *expected1 = " meow";
    
    printf("Training: '%s' (iterations: 10, 50, 100, 200)\n", pattern1);
    
    int train_counts[] = {10, 50, 100, 200};
    for (int idx = 0; idx < 4; idx++) {
        int train_iterations = train_counts[idx];
        // Train
        for (int i = 0; i < train_iterations; i++) {
            melvin_m_universal_input_write(mfile1, (const uint8_t*)pattern1, strlen(pattern1));
            melvin_m_process_input(mfile1);
            melvin_m_universal_input_clear(mfile1);
            melvin_m_universal_output_clear(mfile1);
        }
        
        // Test
        melvin_m_universal_input_write(mfile1, (const uint8_t*)input1, strlen(input1));
        melvin_m_process_input(mfile1);
        
        uint8_t output1[256];
        size_t output1_len = melvin_m_universal_output_read(mfile1, output1, sizeof(output1));
        output1[output1_len] = '\0';
        
        float error_rate1 = calculate_error_rate(expected1, output1, output1_len);
        
        size_t nodes1 = melvin_m_get_node_count(mfile1);
        size_t edges1 = melvin_m_get_edge_count(mfile1);
        
        printf("  %3d iterations: Error=%.1f%%, Nodes=%zu, Edges=%zu (%.2f edges/node), Output='%.*s'\n",
               train_iterations, error_rate1 * 100.0f, nodes1, edges1,
               edges1 > 0 ? (float)edges1 / nodes1 : 0.0f,
               (int)(output1_len < 20 ? output1_len : 20), output1);
        
        melvin_m_universal_input_clear(mfile1);
        melvin_m_universal_output_clear(mfile1);
    }
    
    melvin_m_close(mfile1);
    printf("\n");
    
    // Test 2: Two associations (interference test)
    printf("TEST 2: Two Associations (cat->meow, dog->bark)\n");
    printf("------------------------------------------------\n");
    
    MelvinMFile *mfile2 = melvin_m_create("test_error_two.m");
    if (!mfile2) {
        printf("FAIL: Could not create .m file\n");
        return 1;
    }
    
    const char *patterns2[] = {"cat meow", "dog bark"};
    const char *inputs2[] = {"cat", "dog"};
    const char *expected2[] = {" meow", " bark"};
    
    printf("Training: '%s' and '%s' (50 iterations each)\n", patterns2[0], patterns2[1]);
    
    int train_counts2[] = {10, 50, 100};
    for (int idx2 = 0; idx2 < 3; idx2++) {
        int train_iterations = train_counts2[idx2];
        // Train both patterns
        for (int p = 0; p < 2; p++) {
            for (int i = 0; i < train_iterations; i++) {
                melvin_m_universal_input_write(mfile2, (const uint8_t*)patterns2[p], strlen(patterns2[p]));
                melvin_m_process_input(mfile2);
                melvin_m_universal_input_clear(mfile2);
                melvin_m_universal_output_clear(mfile2);
            }
        }
        
        // Test both
        float total_error = 0.0f;
        for (int t = 0; t < 2; t++) {
            melvin_m_universal_input_write(mfile2, (const uint8_t*)inputs2[t], strlen(inputs2[t]));
            melvin_m_process_input(mfile2);
            
            uint8_t output[256];
            size_t output_len = melvin_m_universal_output_read(mfile2, output, sizeof(output));
            output[output_len] = '\0';
            
            float error_rate = calculate_error_rate(expected2[t], output, output_len);
            total_error += error_rate;
            
            melvin_m_universal_input_clear(mfile2);
            melvin_m_universal_output_clear(mfile2);
        }
        
        float avg_error = total_error / 2.0f;
        size_t nodes2 = melvin_m_get_node_count(mfile2);
        size_t edges2 = melvin_m_get_edge_count(mfile2);
        
        printf("  %3d iterations each: Avg Error=%.1f%%, Nodes=%zu, Edges=%zu (%.2f edges/node)\n",
               train_iterations, avg_error * 100.0f, nodes2, edges2,
               edges2 > 0 ? (float)edges2 / nodes2 : 0.0f);
    }
    
    melvin_m_close(mfile2);
    printf("\n");
    
    // Test 3: Multiple associations (8 patterns)
    printf("TEST 3: Multiple Associations (8 patterns)\n");
    printf("------------------------------------------------\n");
    
    MelvinMFile *mfile3 = melvin_m_create("test_error_multi.m");
    if (!mfile3) {
        printf("FAIL: Could not create .m file\n");
        return 1;
    }
    
    const char *patterns3[] = {
        "cat meow", "dog bark", "bird tweet", "cow moo",
        "duck quack", "hello world", "good morning", "red apple"
    };
    const char *inputs3[] = {"cat", "dog", "bird", "cow", "duck", "hello", "good", "red"};
    const char *expected3[] = {" meow", " bark", " tweet", " moo", " quack", " world", " morning", " apple"};
    
    int num_patterns = 8;
    
    int train_counts3[] = {5, 10, 20, 50};
    for (int idx3 = 0; idx3 < 4; idx3++) {
        int train_iterations = train_counts3[idx3];
        // Train all patterns
        for (int p = 0; p < num_patterns; p++) {
            for (int i = 0; i < train_iterations; i++) {
                melvin_m_universal_input_write(mfile3, (const uint8_t*)patterns3[p], strlen(patterns3[p]));
                melvin_m_process_input(mfile3);
                melvin_m_universal_input_clear(mfile3);
                melvin_m_universal_output_clear(mfile3);
            }
        }
        
        // Test all patterns
        float total_error = 0.0f;
        int passed = 0;
        for (int t = 0; t < num_patterns; t++) {
            melvin_m_universal_input_write(mfile3, (const uint8_t*)inputs3[t], strlen(inputs3[t]));
            melvin_m_process_input(mfile3);
            
            uint8_t output[256];
            size_t output_len = melvin_m_universal_output_read(mfile3, output, sizeof(output));
            output[output_len] = '\0';
            
            float error_rate = calculate_error_rate(expected3[t], output, output_len);
            total_error += error_rate;
            if (error_rate < 0.2f) passed++; // < 20% error = pass
            
            melvin_m_universal_input_clear(mfile3);
            melvin_m_universal_output_clear(mfile3);
        }
        
        float avg_error = total_error / num_patterns;
        size_t nodes3 = melvin_m_get_node_count(mfile3);
        size_t edges3 = melvin_m_get_edge_count(mfile3);
        
        printf("  %2d iterations each: Avg Error=%.1f%%, Passed=%d/%d, Nodes=%zu, Edges=%zu (%.2f edges/node)\n",
               train_iterations, avg_error * 100.0f, passed, num_patterns, nodes3, edges3,
               edges3 > 0 ? (float)edges3 / nodes3 : 0.0f);
    }
    
    melvin_m_close(mfile3);
    printf("\n");
    
    printf("=== Summary ===\n");
    printf("Edge direction refactor: Forward-only edges\n");
    printf("Error rates measured above show current learning performance\n");
    
    return 0;
}
