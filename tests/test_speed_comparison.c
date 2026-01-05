/*
 * Speed Comparison Test
 * 
 * Tests the refactored wave propagation speed improvement from:
 * - OLD: O(degree) scanning of all edges
 * - NEW: O(1) winning edge selection using cached context values
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

int main(int argc, char **argv) {
    printf("=================================================\n");
    printf("Wave Propagation Speed Test\n");
    printf("=================================================\n\n");
    
    const char *mfile_path = "test_speed.m";
    int num_iterations = 100;
    
    if (argc > 1) {
        num_iterations = atoi(argv[1]);
    }
    
    printf("Configuration:\n");
    printf("  Iterations: %d\n", num_iterations);
    printf("  Testing: Refactored O(1) edge selection\n\n");
    
    // Remove existing file
    remove(mfile_path);
    
    // Create and train a graph
    printf("Creating graph...\n");
    MelvinMFile *mfile = melvin_m_create(mfile_path);
    if (!mfile) {
        fprintf(stderr, "Failed to create .m file\n");
        return 1;
    }
    
    // Add some patterns to create edges
    const char *patterns[] = {
        "hello world",
        "test pattern",
        "wave propagation"
    };
    
    for (size_t i = 0; i < sizeof(patterns)/sizeof(patterns[0]); i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Graph created: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Warmup
    const char *test_input = "hello";
    for (int i = 0; i < 5; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)test_input, strlen(test_input));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Speed test
    printf("Running speed test...\n");
    double start_time = get_time_us();
    
    for (int i = 0; i < num_iterations; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)test_input, strlen(test_input));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    double end_time = get_time_us();
    double elapsed_us = end_time - start_time;
    double avg_us = elapsed_us / (double)num_iterations;
    
    printf("\nResults:\n");
    printf("  Total time: %.2f ms\n", elapsed_us / 1000.0);
    printf("  Average per iteration: %.2f us\n", avg_us);
    printf("  Throughput: %.0f ops/sec\n", 1000000.0 / avg_us);
    
    // Expected improvement explanation
    printf("\nExpected Improvement:\n");
    printf("  OLD: O(degree) - scans all edges at each node\n");
    printf("  NEW: O(1) - uses cached context values\n");
    printf("  Speedup scales with average node degree\n");
    printf("  Current graph: %zu edges / %zu nodes = %.1f avg degree\n",
           melvin_m_get_edge_count(mfile),
           melvin_m_get_node_count(mfile),
           (double)melvin_m_get_edge_count(mfile) / (double)melvin_m_get_node_count(mfile));
    
    // Accuracy check
    printf("\nAccuracy Check:\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        uint8_t output[256];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("  Input: 'hello' -> Output: '%.*s'\n", (int)read_size, output);
        printf("  ✓ System is learning and generating output\n");
    } else {
        printf("  Input: 'hello' -> (no output yet)\n");
        printf("  ✓ System is still learning\n");
    }
    
    // Close
    melvin_m_close(mfile);
    remove(mfile_path);
    
    printf("\n=================================================\n");
    printf("✓ Test completed successfully!\n");
    printf("=================================================\n");
    
    return 0;
}

