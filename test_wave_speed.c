/*
 * Test: Wave Propagation Speed and Accuracy
 * 
 * Tests the refactored wave propagation that uses only winning edges
 * instead of scanning all edges. Measures:
 * 1. Speed improvement from O(degree) to O(1) edge selection
 * 2. Accuracy of output generation
 * 3. Graph intelligence preservation
 */

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
    return (double)tv.tv_sec * 1000000.0 + (double)tv.tv_usec;
}

// Create a test graph with patterns
static void create_test_graph(const char *mfile_path, size_t num_patterns) {
    printf("Creating test graph with %zu patterns...\n", num_patterns);
    
    // Remove existing file
    remove(mfile_path);
    
    // Create new .m file
    MelvinMFile *mfile = melvin_m_create(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to create .m file\n");
        exit(1);
    }
    
    // Add test patterns
    const char *patterns[] = {
        "hello",
        "world",
        "hello world",
        "test pattern",
        "quick brown fox",
        "lazy dog",
        "the quick brown fox jumps over the lazy dog",
        "testing wave propagation",
        "speed and accuracy",
        "melvin intelligence"
    };
    
    for (size_t i = 0; i < num_patterns && i < sizeof(patterns)/sizeof(patterns[0]); i++) {
        const char *pattern = patterns[i];
        size_t len = strlen(pattern);
        
        // Write input
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, len);
        
        // Process input (wave propagation and learning)
        melvin_m_process_input(mfile);
        
        // Read output
        uint8_t output[1024];
        size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        // Clear for next iteration
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        if (i % 10 == 0) {
            printf("  Added pattern %zu/%zu\n", i+1, num_patterns);
        }
    }
    
    printf("Graph created with nodes and edges\n");
    melvin_m_close(mfile);
}

// Test wave propagation speed
static void test_wave_speed(const char *mfile_path, int num_iterations) {
    printf("\nTesting wave propagation speed...\n");
    
    MelvinMFile *mfile = melvin_m_load(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to load .m file\n");
        exit(1);
    }
    
    const char *test_input = "hello world test";
    size_t input_len = strlen(test_input);
    
    // Warmup
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)test_input, input_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Timed test
    double start_time = get_time_us();
    
    for (int i = 0; i < num_iterations; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)test_input, input_len);
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    double end_time = get_time_us();
    double elapsed_us = end_time - start_time;
    double avg_us = elapsed_us / (double)num_iterations;
    
    printf("Results:\n");
    printf("  Total iterations: %d\n", num_iterations);
    printf("  Total time: %.2f ms\n", elapsed_us / 1000.0);
    printf("  Average time per iteration: %.2f us\n", avg_us);
    printf("  Iterations per second: %.0f\n", 1000000.0 / avg_us);
    
    melvin_m_close(mfile);
}

// Test output accuracy
static void test_output_accuracy(const char *mfile_path) {
    printf("\nTesting output accuracy...\n");
    
    MelvinMFile *mfile = melvin_m_load(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to load .m file\n");
        exit(1);
    }
    
    // Test inputs
    const char *test_inputs[] = {
        "hello",
        "world",
        "test",
        "quick"
    };
    
    int successful_outputs = 0;
    int total_tests = sizeof(test_inputs) / sizeof(test_inputs[0]);
    
    for (int i = 0; i < total_tests; i++) {
        const char *input = test_inputs[i];
        size_t input_len = strlen(input);
        
        // Write input
        melvin_m_universal_input_write(mfile, (const uint8_t*)input, input_len);
        
        // Process input
        melvin_m_process_input(mfile);
        
        // Read output
        uint8_t output[1024];
        size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        printf("  Input: '%s' -> Output: ", input);
        if (output_len > 0) {
            printf("'%.*s' (%zu bytes)\n", (int)output_len, output, output_len);
            successful_outputs++;
        } else {
            printf("(no output - learning mode)\n");
        }
        
        // Clear for next iteration
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Results:\n");
    printf("  Successful outputs: %d/%d\n", successful_outputs, total_tests);
    printf("  Output accuracy: %.1f%%\n", 
           (100.0 * successful_outputs) / total_tests);
    
    melvin_m_close(mfile);
}

// Test graph intelligence preservation
static void test_intelligence_preservation(const char *mfile_path) {
    printf("\nTesting intelligence preservation...\n");
    
    MelvinMFile *mfile = melvin_m_load(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to load .m file\n");
        exit(1);
    }
    
    // Train on a simple pattern repeatedly
    const char *training_pattern = "ab";
    printf("  Training on pattern: '%s'\n", training_pattern);
    
    for (int i = 0; i < 20; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)training_pattern, 
                                       strlen(training_pattern));
        melvin_m_process_input(mfile);
        
        uint8_t output[1024];
        melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test if it learned the pattern
    printf("  Testing learned pattern recall...\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"a", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output[1024];
    size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
    
    printf("  Input: 'a' -> Output: ");
    if (output_len > 0) {
        printf("'%.*s'\n", (int)output_len, output);
        
        // Check if output contains 'b' (showing it learned the pattern)
        int contains_b = 0;
        for (size_t i = 0; i < output_len; i++) {
            if (output[i] == 'b') {
                contains_b = 1;
                break;
            }
        }
        
        if (contains_b) {
            printf("  ✓ Pattern learned successfully!\n");
        } else {
            printf("  ⚠ Pattern not clearly learned (expected 'b' in output)\n");
        }
    } else {
        printf("(no output)\n");
        printf("  ⚠ Still in learning mode\n");
    }
    
    melvin_m_close(mfile);
}

int main(int argc, char **argv) {
    printf("=================================================\n");
    printf("Wave Propagation Speed & Accuracy Test\n");
    printf("=================================================\n\n");
    
    const char *mfile_path = "test_wave.m";
    int num_patterns = 10;
    int num_iterations = 1000;
    
    // Parse command line args
    if (argc > 1) {
        num_iterations = atoi(argv[1]);
    }
    
    printf("Test configuration:\n");
    printf("  .m file: %s\n", mfile_path);
    printf("  Training patterns: %d\n", num_patterns);
    printf("  Speed test iterations: %d\n\n", num_iterations);
    
    // Create test graph
    create_test_graph(mfile_path, num_patterns);
    
    // Run tests
    test_wave_speed(mfile_path, num_iterations);
    test_output_accuracy(mfile_path);
    test_intelligence_preservation(mfile_path);
    
    printf("\n=================================================\n");
    printf("All tests completed!\n");
    printf("=================================================\n");
    
    // Clean up
    remove(mfile_path);
    
    return 0;
}

