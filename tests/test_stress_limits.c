/* Stress Test: Push Melvin to Its Limits
 * 
 * Tests to find breaking points:
 * 1. Very long sequences (100+ bytes)
 * 2. Many iterations (10,000+)
 * 3. Complex patterns (multiple overlapping)
 * 4. Rapid pattern switching
 * 5. Large vocabulary (many unique bytes)
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void test_long_sequences() {
    printf("\n=== TEST 1: Long Sequences ===\n");
    printf("Testing with increasingly long patterns\n\n");
    
    remove("test_long.m");
    MelvinMFile *mfile = melvin_m_create("test_long.m");
    
    const char *patterns[] = {
        "hello world",                                    // 11 bytes
        "the quick brown fox jumps over the lazy dog",   // 44 bytes
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.", // 123 bytes
    };
    
    for (int p = 0; p < 3; p++) {
        size_t len = strlen(patterns[p]);
        printf("Pattern %d: %zu bytes\n", p+1, len);
        
        // Train 50 iterations
        for (int i = 0; i < 50; i++) {
            melvin_in_port_process_device(mfile, 0, (uint8_t*)patterns[p], len);
            melvin_m_universal_output_clear(mfile);
        }
        
        // Test with first 10 bytes
        size_t test_len = (len > 10) ? 10 : len/2;
        melvin_in_port_process_device(mfile, 0, (uint8_t*)patterns[p], test_len);
        
        size_t output_len = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        printf("  Input: '%.*s...'\n", (int)test_len, patterns[p]);
        printf("  Output: '%.*s' (%zu bytes)\n", 
               (int)(output_len > 50 ? 50 : output_len), output, output_len);
        
        size_t nodes = melvin_m_get_node_count(mfile);
        size_t edges = melvin_m_get_edge_count(mfile);
        printf("  Graph: %zu nodes, %zu edges\n", nodes, edges);
        
        free(output);
        melvin_m_universal_output_clear(mfile);
    }
    
    melvin_m_close(mfile);
}

void test_many_iterations() {
    printf("\n=== TEST 2: Many Iterations ===\n");
    printf("Training for 10,000 iterations\n\n");
    
    remove("test_many.m");
    MelvinMFile *mfile = melvin_m_create("test_many.m");
    
    const char *pattern = "hello world";
    clock_t start = clock();
    
    for (int i = 1; i <= 10000; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_output_clear(mfile);
        
        // Check every 1000 iterations
        if (i % 1000 == 0) {
            melvin_in_port_process_device(mfile, 0, (uint8_t*)"hello ", 6);
            
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            melvin_m_universal_output_read(mfile, output, output_len);
            output[output_len] = '\0';
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            
            printf("Iter %5d: %zu nodes, %zu edges, output='%.*s'\n", 
                   i, nodes, edges, (int)(output_len > 20 ? 20 : output_len), output);
            
            free(output);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    clock_t end = clock();
    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("\nCompleted 10,000 iterations in %.2f seconds (%.0f iter/sec)\n", 
           seconds, 10000.0/seconds);
    
    melvin_m_close(mfile);
}

void test_complex_patterns() {
    printf("\n=== TEST 3: Complex Overlapping Patterns ===\n");
    printf("Training multiple patterns with shared prefixes\n\n");
    
    remove("test_complex.m");
    MelvinMFile *mfile = melvin_m_create("test_complex.m");
    
    const char *patterns[] = {
        "the cat sat",
        "the cat ran",
        "the dog sat",
        "the dog ran",
        "a cat sat",
        "a dog ran",
    };
    
    // Train all patterns
    for (int iter = 0; iter < 100; iter++) {
        for (int p = 0; p < 6; p++) {
            melvin_in_port_process_device(mfile, 0, (uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("Trained on 6 overlapping patterns (100 iterations each)\n\n");
    
    // Test each prefix
    const char *tests[] = {
        "the cat ",
        "the dog ",
        "a cat ",
        "a dog ",
    };
    
    for (int t = 0; t < 4; t++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)tests[t], strlen(tests[t]));
        
        size_t output_len = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        printf("Input: '%s' → Output: '%.*s'\n", 
               tests[t], (int)(output_len > 30 ? 30 : output_len), output);
        
        free(output);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nFinal graph: %zu nodes, %zu edges\n", nodes, edges);
    
    melvin_m_close(mfile);
}

void test_rapid_switching() {
    printf("\n=== TEST 4: Rapid Pattern Switching ===\n");
    printf("Switching between patterns every iteration\n\n");
    
    remove("test_switch.m");
    MelvinMFile *mfile = melvin_m_create("test_switch.m");
    
    const char *patterns[] = {
        "AAAA",
        "BBBB",
        "CCCC",
        "DDDD",
    };
    
    // Rapidly switch between patterns
    for (int iter = 0; iter < 400; iter++) {
        int p = iter % 4;
        melvin_in_port_process_device(mfile, 0, (uint8_t*)patterns[p], 4);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Trained 100 iterations per pattern (rapidly switching)\n\n");
    
    // Test each pattern
    for (int p = 0; p < 4; p++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)patterns[p], 2);  // First 2 bytes
        
        size_t output_len = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        printf("Input: '%.*s' → Output: '%s' (expected: '%s')\n", 
               2, patterns[p], output, patterns[p]+2);
        
        free(output);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("\nGraph: %zu nodes, %zu edges\n", nodes, edges);
    
    melvin_m_close(mfile);
}

void test_large_vocabulary() {
    printf("\n=== TEST 5: Large Vocabulary ===\n");
    printf("Training with many unique bytes\n\n");
    
    remove("test_vocab.m");
    MelvinMFile *mfile = melvin_m_create("test_vocab.m");
    
    // Create pattern with many unique characters
    char pattern[101];
    for (int i = 0; i < 100; i++) {
        pattern[i] = 'A' + (i % 52);  // A-Z, a-z cycling
    }
    pattern[100] = '\0';
    
    printf("Pattern: 100 bytes with 52 unique characters\n");
    
    // Train
    for (int i = 0; i < 100; i++) {
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, 100);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test
    melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, 10);
    
    size_t output_len = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(output_len + 1);
    melvin_m_universal_output_read(mfile, output, output_len);
    output[output_len] = '\0';
    
    printf("Input: '%.*s...'\n", 10, pattern);
    printf("Output: '%.*s' (%zu bytes)\n", 
           (int)(output_len > 50 ? 50 : output_len), output, output_len);
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    printf("Graph: %zu nodes, %zu edges\n", nodes, edges);
    
    free(output);
    melvin_m_close(mfile);
}

int main() {
    printf("=== MELVIN STRESS TEST: FINDING THE LIMITS ===\n");
    printf("Testing where the system breaks or degrades\n");
    
    test_long_sequences();
    test_many_iterations();
    test_complex_patterns();
    test_rapid_switching();
    test_large_vocabulary();
    
    printf("\n=== STRESS TEST COMPLETE ===\n");
    printf("\nLook for:\n");
    printf("  • Memory issues (crashes, slowdowns)\n");
    printf("  • Learning degradation (wrong outputs)\n");
    printf("  • Graph explosion (too many nodes/edges)\n");
    printf("  • Performance issues (slow iterations)\n");
    
    return 0;
}
