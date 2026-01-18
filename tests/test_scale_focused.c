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
        return usage.ru_maxrss;
    }
    return 0;
}

int main() {
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║     MELVIN SCALE TEST: Graph Growth & Memory (Infinite Context)          ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n\n");
    
    const char *test_file = "scale_focused_test.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Single pattern - should NOT grow unbounded
    const char *pattern = "hello world";
    
    printf("Test: Repeated training on same pattern\n");
    printf("  Pattern: '%s'\n", pattern);
    printf("  Expected: Graph should stabilize (reuse nodes/edges)\n");
    printf("  Target: 2000 iterations\n\n");
    
    printf("Iteration | Nodes | Edges | Memory (MB) | Growth | Status\n");
    printf("----------|-------|-------|-------------|--------|--------\n");
    
    time_t start_time = time(NULL);
    size_t prev_nodes = 0;
    size_t prev_edges = 0;
    int stable_count = 0;
    int unbounded_growth = 0;
    
    for (int iter = 1; iter <= 2000; iter++) {
        // Train on same pattern
        melvin_in_port_process_device(mfile, 0, (uint8_t*)pattern, strlen(pattern));
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
        
        // Check every 100 iterations
        if (iter % 100 == 0 || iter <= 20) {
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            long memory_kb = get_memory_usage();
            double memory_mb = memory_kb / 1024.0;
            
            // Calculate growth
            size_t node_growth = (prev_nodes > 0) ? (nodes - prev_nodes) : nodes;
            size_t edge_growth = (prev_edges > 0) ? (edges - prev_edges) : edges;
            
            // Check if graph is stable
            const char *status = "GROWING";
            if (node_growth == 0 && edge_growth == 0 && iter > 100) {
                stable_count++;
                status = "STABLE";
            } else if (node_growth > 10 || edge_growth > 50) {
                unbounded_growth++;
                status = "UNBOUNDED";
            }
            
            printf("%9d | %5zu | %5zu | %11.1f | +%zu/+%zu | %s\n",
                   iter, nodes, edges, memory_mb, node_growth, edge_growth, status);
            
            // Memory check
            if (memory_mb > 2000.0) {
                printf("  ⚠️  CRITICAL: Memory > 2GB: %.1f MB\n", memory_mb);
                printf("  Stopping test - memory leak detected\n");
                break;
            }
            
            // Unbounded growth check
            if (unbounded_growth > 5 && iter > 500) {
                printf("  ⚠️  CRITICAL: Unbounded graph growth detected\n");
                printf("  System not recognizing repeated patterns\n");
                break;
            }
            
            prev_nodes = nodes;
            prev_edges = edges;
        }
    }
    
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);
    
    // Final assessment
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    long final_memory = get_memory_usage();
    double final_memory_mb = final_memory / 1024.0;
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                        SCALE TEST SUMMARY                               ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════╝\n");
    printf("  Final Graph: %zu nodes, %zu edges\n", final_nodes, final_edges);
    printf("  Final Memory: %.2f MB\n", final_memory_mb);
    printf("  Execution Time: %.2f seconds\n", elapsed);
    printf("  Stable Periods: %d\n", stable_count);
    printf("  Unbounded Growth Events: %d\n", unbounded_growth);
    
    // Scalability verdict
    printf("\n  Scalability Assessment:\n");
    
    // Graph growth assessment
    if (final_nodes < 100) {
        printf("    ✓✓✓ EXCELLENT: Graph stabilized at %zu nodes\n", final_nodes);
    } else if (final_nodes < 500) {
        printf("    ✓✓  GOOD: Graph reasonable size (%zu nodes)\n", final_nodes);
    } else if (final_nodes < 2000) {
        printf("    ⚠️  MODERATE: Graph larger than expected (%zu nodes)\n", final_nodes);
        printf("      System may not be recognizing repeated patterns\n");
    } else {
        printf("    ⚠️⚠️  POOR: Unbounded graph growth (%zu nodes)\n", final_nodes);
        printf("      System creating new nodes for each iteration\n");
    }
    
    // Memory assessment
    double kb_per_node = (final_nodes > 0) ? (final_memory_mb * 1024.0 / final_nodes) : 0.0;
    if (kb_per_node < 1.0) {
        printf("    ✓✓✓ EXCELLENT: < 1 KB per node\n");
    } else if (kb_per_node < 10.0) {
        printf("    ✓✓  GOOD: < 10 KB per node\n");
    } else if (kb_per_node < 100.0) {
        printf("    ⚠️  MODERATE: < 100 KB per node\n");
    } else {
        printf("    ⚠️⚠️  POOR: > 100 KB per node - memory leak likely\n");
    }
    
    // Stability assessment
    if (stable_count > 10) {
        printf("    ✓✓✓ EXCELLENT: Graph stabilized (reuses existing structure)\n");
    } else if (stable_count > 5) {
        printf("    ✓✓  GOOD: Graph mostly stable\n");
    } else if (unbounded_growth == 0) {
        printf("    ✓  ACCEPTABLE: Graph growing but bounded\n");
    } else {
        printf("    ⚠️  CONCERN: Unbounded growth detected\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    printf("\n=== Scale Test Complete ===\n");
    printf("Key Question: Does graph stabilize when training on same pattern?\n");
    printf("Expected: Yes (reuse nodes/edges)\n");
    printf("Actual: %s\n", (stable_count > 5) ? "YES ✓" : "NO ⚠️");
    
    return (stable_count > 5 && final_memory_mb < 2000.0) ? 0 : 1;
}
