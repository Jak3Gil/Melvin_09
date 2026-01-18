#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/resource.h>
#include "melvin.h"
#include "melvin_in_port.h"

// Get memory usage in KB
long get_memory_usage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss; // KB
    }
    return 0;
}

int main() {
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║        MELVIN MEMORY SCALABILITY TEST (Infinite Context)                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n\n");
    
    const char *test_file = "scale_memory_test.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Simple pattern that should work
    const char *pattern = "hello world";
    const char *test_input = "hello ";
    const char *expected = "world";
    
    printf("Configuration:\n");
    printf("  Training: '%s' (repeated)\n", pattern);
    printf("  Test: '%s' -> '%s'\n", test_input, expected);
    printf("  Target: 5000 iterations\n");
    printf("  Monitoring: Memory usage, graph growth, error rate\n\n");
    
    printf("Iteration | Nodes   | Edges   | Memory (MB) | Error %% | Growth Rate\n");
    printf("----------|---------|---------|-------------|---------|------------\n");
    
    time_t start_time = time(NULL);
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    long prev_memory = 0;
    int error_count = 0;
    int success_count = 0;
    
    for (int iter = 1; iter <= 5000; iter++) {
        // Train
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Test every 100 iterations
        if (iter % 100 == 0 || iter <= 20) {
            // Test
            melvin_in_port_process_device(mfile, 0, (uint8_t*)test_input, strlen(test_input));
            
            size_t output_len = melvin_m_universal_output_size(mfile);
            uint8_t *output = malloc(output_len + 1);
            int is_correct = 0;
            
            if (output && output_len > 0) {
                melvin_m_universal_output_read(mfile, output, output_len);
                output[output_len] = '\0';
                
                // Simple match check
                if (output_len == strlen(expected) && 
                    memcmp(output, expected, output_len) == 0) {
                    is_correct = 1;
                    success_count++;
                } else {
                    error_count++;
                }
                free(output);
            } else {
                error_count++;
            }
            
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
            
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            long memory_kb = get_memory_usage();
            double memory_mb = memory_kb / 1024.0;
            
            // Calculate growth rate
            double node_growth = (prev_nodes > 0) ? 
                ((double)(nodes - prev_nodes) / (double)prev_nodes * 100.0) : 0.0;
            double edge_growth = (prev_edges > 0) ? 
                ((double)(edges - prev_edges) / (double)prev_edges * 100.0) : 0.0;
            double memory_growth = (prev_memory > 0) ? 
                ((double)(memory_kb - prev_memory) / (double)prev_memory * 100.0) : 0.0;
            
            float error_rate = (error_count + success_count > 0) ?
                ((float)error_count / (float)(error_count + success_count) * 100.0f) : 0.0f;
            
            printf("%9d | %7zu | %7zu | %11.1f | %7.1f%% | N:%.1f%% E:%.1f%% M:%.1f%%\n",
                   iter, nodes, edges, memory_mb, error_rate,
                   node_growth, edge_growth, memory_growth);
            
            // Check for memory issues
            if (memory_mb > 1000.0) {
                printf("  ⚠️  WARNING: Memory usage > 1GB: %.1f MB\n", memory_mb);
            }
            
            // Check for unbounded growth
            if (node_growth > 50.0 && iter > 100) {
                printf("  ⚠️  WARNING: Rapid node growth: %.1f%%\n", node_growth);
            }
            
            prev_nodes = nodes;
            prev_edges = edges;
            prev_memory = memory_kb;
        }
        
        // Safety check
        size_t current_nodes = melvin_m_get_node_count(mfile);
        if (current_nodes > 100000) {
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
    double final_memory_mb = final_memory / 1024.0;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                        MEMORY SCALABILITY SUMMARY                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    printf("  Total Iterations: 5000\n");
    printf("  Final Graph: %zu nodes, %zu edges\n", final_nodes, final_edges);
    printf("  Final Memory: %.2f MB\n", final_memory_mb);
    printf("  Memory per Node: %.2f KB\n", (final_nodes > 0) ? (final_memory_mb * 1024.0 / final_nodes) : 0.0);
    printf("  Memory per Edge: %.2f bytes\n", (final_edges > 0) ? (final_memory_mb * 1024.0 * 1024.0 / final_edges) : 0.0);
    printf("  Execution Time: %.2f seconds\n", elapsed);
    printf("  Throughput: %.1f iterations/second\n", 5000.0 / elapsed);
    printf("  Success Rate: %.1f%% (%d/%d)\n", 
           (success_count + error_count > 0) ? 
           ((float)success_count / (float)(success_count + error_count) * 100.0f) : 0.0f,
           success_count, success_count + error_count);
    
    // Memory efficiency assessment
    printf("\n  Memory Efficiency:\n");
    double kb_per_node = (final_nodes > 0) ? (final_memory_mb * 1024.0 / final_nodes) : 0.0;
    if (kb_per_node < 1.0) {
        printf("    ✓✓ Excellent: < 1 KB per node\n");
    } else if (kb_per_node < 10.0) {
        printf("    ✓  Good: < 10 KB per node\n");
    } else if (kb_per_node < 100.0) {
        printf("    ⚠️  Moderate: < 100 KB per node\n");
    } else {
        printf("    ⚠️⚠️  High: > 100 KB per node - potential memory leak\n");
    }
    
    // Scalability verdict
    printf("\n  Scalability Verdict:\n");
    if (final_memory_mb < 100.0 && final_nodes > 1000) {
        printf("    ✓✓✓ EXCELLENT: Handles large graphs with low memory\n");
    } else if (final_memory_mb < 500.0 && final_nodes > 1000) {
        printf("    ✓✓  GOOD: Handles large graphs reasonably\n");
    } else if (final_memory_mb < 2000.0) {
        printf("    ✓  ACCEPTABLE: Works but high memory usage\n");
    } else {
        printf("    ⚠️  CONCERN: Very high memory usage - investigate leaks\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("\n=== Memory Scalability Test Complete ===\n");
    
    return (final_memory_mb < 2000.0 && success_count > error_count) ? 0 : 1;
}
