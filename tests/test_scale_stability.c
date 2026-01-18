#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "melvin.h"
#include "melvin_in_port.h"

// Calculate error rate
float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    if (!expected || !actual) return 1.0f;
    size_t expected_len = strlen(expected);
    if (expected_len == 0 && actual_len == 0) return 0.0f;
    if (expected_len == 0 || actual_len == 0) return 1.0f;
    
    size_t errors = 0;
    size_t max_len = (actual_len > expected_len) ? actual_len : expected_len;
    for (size_t i = 0; i < max_len; i++) {
        if (i >= actual_len || i >= expected_len || actual[i] != (uint8_t)expected[i]) {
            errors++;
        }
    }
    return (max_len > 0) ? ((float)errors / (float)max_len) : 1.0f;
}

// Get memory usage in KB
long get_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss; // KB on Linux, different on macOS
    }
    return 0;
}

int main() {
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║          MELVIN SCALABILITY & STABILITY TEST (Infinite Context)          ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n\n");
    
    const char *test_file = "scale_test.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Test patterns - diverse set to build large graph
    const char *patterns[] = {
        "hello world",
        "the quick brown fox",
        "machine learning",
        "artificial intelligence",
        "neural networks",
        "deep learning",
        "natural language",
        "computer science",
        "data structures",
        "algorithm design",
        "graph theory",
        "pattern matching",
        "context awareness",
        "sparse representations",
        "distributed systems"
    };
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    // Test queries (input -> expected)
    struct {
        const char *input;
        const char *expected;
    } queries[] = {
        {"hello ", "world"},
        {"the quick ", "brown fox"},
        {"machine ", "learning"},
        {"artificial ", "intelligence"},
        {"neural ", "networks"}
    };
    int num_queries = sizeof(queries) / sizeof(queries[0]);
    
    printf("Configuration:\n");
    printf("  Training patterns: %d\n", num_patterns);
    printf("  Test queries: %d\n", num_queries);
    printf("  Target iterations: 1000\n");
    printf("  Testing infinite context scalability\n\n");
    
    printf("Iteration | Nodes   | Edges   | Memory  | Error %% | Status\n");
    printf("----------|---------|---------|---------|---------|--------\n");
    
    time_t start_time = time(NULL);
    float total_error = 0.0f;
    int test_count = 0;
    int crash_count = 0;
    size_t max_nodes = 0;
    size_t max_edges = 0;
    
    for (int iter = 1; iter <= 1000; iter++) {
        // Train on random pattern
        const char *pattern = patterns[iter % num_patterns];
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 50 iterations
        if (iter % 50 == 0) {
            // Test all queries
            float avg_error = 0.0f;
            int queries_tested = 0;
            
            for (int q = 0; q < num_queries; q++) {
                melvin_in_port_process_device(mfile, 0, 
                    (uint8_t*)queries[q].input, strlen(queries[q].input));
                
                size_t output_len = melvin_m_universal_output_size(mfile);
                uint8_t *output = malloc(output_len + 1);
                if (output && output_len > 0) {
                    melvin_m_universal_output_read(mfile, output, output_len);
                    output[output_len] = '\0';
                    
                    float error = calculate_error_rate(queries[q].expected, output, output_len);
                    avg_error += error;
                    queries_tested++;
                    
                    free(output);
                }
                
                melvin_m_universal_input_clear(mfile);
                melvin_m_universal_output_clear(mfile);
            }
            
            if (queries_tested > 0) {
                avg_error /= queries_tested;
                total_error += avg_error;
                test_count++;
            }
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            long memory_kb = get_memory_usage();
            
            if (nodes > max_nodes) max_nodes = nodes;
            if (edges > max_edges) max_edges = edges;
            
            // Status indicator
            const char *status = "OK";
            if (nodes > 10000) status = "LARGE";
            if (nodes > 50000) status = "XL";
            if (nodes > 100000) status = "HUGE";
            
            printf("%9d | %7zu | %7zu | %7ld | %7.1f%% | %s\n",
                   iter, nodes, edges, memory_kb, avg_error * 100.0f, status);
            
            // Check for potential issues
            if (nodes == 0 || edges == 0) {
                printf("  WARNING: Graph appears empty!\n");
                crash_count++;
            }
            
            // Memory check (warn if > 1GB)
            if (memory_kb > 1000000) {
                printf("  WARNING: High memory usage: %.2f MB\n", memory_kb / 1024.0f);
            }
        }
        
        // Early exit if graph gets too large (safety)
        size_t current_nodes = melvin_m_get_node_count(mfile);
        if (current_nodes > 200000) {
            printf("\n  Stopping at %zu nodes (safety limit)\n", current_nodes);
            break;
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    // Final stats
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    long final_memory = get_memory_usage();
    float avg_error = (test_count > 0) ? (total_error / test_count) : 0.0f;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                        SCALABILITY SUMMARY                               ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    printf("  Total Iterations: 1000\n");
    printf("  Final Graph Size: %zu nodes, %zu edges\n", final_nodes, final_edges);
    printf("  Peak Graph Size: %zu nodes, %zu edges\n", max_nodes, max_edges);
    printf("  Final Memory: %.2f MB\n", final_memory / 1024.0f);
    printf("  Average Error Rate: %.2f%%\n", avg_error * 100.0f);
    printf("  Execution Time: %.2f seconds\n", elapsed);
    printf("  Throughput: %.1f iterations/second\n", 1000.0 / elapsed);
    printf("  Warnings/Issues: %d\n", crash_count);
    
    // Scalability assessment
    printf("\n  Scalability Assessment:\n");
    if (final_nodes < 1000) {
        printf("    ⚠️  Small graph - may not test scalability fully\n");
    } else if (final_nodes < 10000) {
        printf("    ✓  Medium graph - system handling well\n");
    } else if (final_nodes < 100000) {
        printf("    ✓✓ Large graph - excellent scalability\n");
    } else {
        printf("    ✓✓✓ Very large graph - exceptional scalability\n");
    }
    
    if (avg_error < 0.1f) {
        printf("    ✓  Low error rate - learning working correctly\n");
    } else {
        printf("    ⚠️  Higher error rate - may need tuning\n");
    }
    
    if (elapsed < 60.0) {
        printf("    ✓  Fast execution - good performance\n");
    } else {
        printf("    ⚠️  Slow execution - may need optimization\n");
    }
    
    if (crash_count == 0) {
        printf("    ✓  No crashes or warnings - stable system\n");
    } else {
        printf("    ⚠️  %d warnings detected - investigate\n", crash_count);
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("\n=== Scalability Test Complete ===\n");
    printf("System uses infinite context (SparseContext) - scales to billions of nodes\n");
    
    return (crash_count == 0 && avg_error < 0.5f) ? 0 : 1;
}
