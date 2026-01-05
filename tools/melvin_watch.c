/*
 * Melvin: Real-Time Brain Monitor
 * 
 * Watches brain file and displays real-time statistics
 */

#include "../src/melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

/* Format file size */
static void format_size(long long size, char *buffer, size_t buffer_size) {
    if (size >= 1024LL * 1024LL * 1024LL) {
        snprintf(buffer, buffer_size, "%.2f GB", size / (1024.0 * 1024.0 * 1024.0));
    } else if (size >= 1024LL * 1024LL) {
        snprintf(buffer, buffer_size, "%.2f MB", size / (1024.0 * 1024.0));
    } else if (size >= 1024LL) {
        snprintf(buffer, buffer_size, "%.2f KB", size / 1024.0);
    } else {
        snprintf(buffer, buffer_size, "%lld B", size);
    }
}

/* Print usage */
static void print_usage(const char *prog_name) {
    printf("Usage: %s <brain.m> [--interval SECONDS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --interval N    Refresh interval in seconds (default: 1)\n");
    printf("  --help          Show this help message\n");
    printf("\nExamples:\n");
    printf("  %s brain.m\n", prog_name);
    printf("  %s brain.m --interval 2\n", prog_name);
    printf("\nPress Ctrl+C to stop\n");
}

/* Main function */
int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Check for help
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    const char *brain_file = argv[1];
    int interval = 1;  // Default: 1 second
    
    // Parse interval
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "--interval") == 0 && i + 1 < argc) {
            interval = atoi(argv[++i]);
            if (interval < 1) interval = 1;
        }
    }
    
    printf("Watching: %s (refresh every %d second%s)\n", 
           brain_file, interval, interval == 1 ? "" : "s");
    printf("Press Ctrl+C to stop\n\n");
    
    size_t last_nodes = 0;
    size_t last_edges = 0;
    uint64_t last_adaptations = 0;
    time_t last_update = 0;
    
    while (1) {
        MelvinMFile *mfile = melvin_m_load(brain_file);
        if (!mfile) {
            fprintf(stderr, "Error: Cannot load brain (file may not exist yet)\n");
            sleep(interval);
            continue;
        }
        
        // Clear screen and move cursor to top
        printf("\033[2J\033[H");
        
        // Get current stats
        size_t nodes = melvin_m_get_node_count(mfile);
        size_t edges = melvin_m_get_edge_count(mfile);
        uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
        
        // Get file size
        struct stat st;
        char file_size_str[64] = "Unknown";
        if (stat(brain_file, &st) == 0) {
            format_size(st.st_size, file_size_str, sizeof(file_size_str));
        }
        
        // Compute deltas
        size_t delta_nodes = (last_nodes > 0) ? nodes - last_nodes : 0;
        size_t delta_edges = (last_edges > 0) ? edges - last_edges : 0;
        uint64_t delta_adaptations = (last_adaptations > 0) ? 
            adaptations - last_adaptations : 0;
        
        // Compute rate
        time_t now = time(NULL);
        double time_delta = (last_update > 0) ? difftime(now, last_update) : interval;
        double node_rate = (time_delta > 0) ? delta_nodes / time_delta : 0;
        double edge_rate = (time_delta > 0) ? delta_edges / time_delta : 0;
        
        // Get current time
        struct tm *tm_info = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        
        // Display header
        printf("╔══════════════════════════════════════════════════════════╗\n");
        printf("║ Melvin Brain Monitor                                    ║\n");
        printf("╠══════════════════════════════════════════════════════════╣\n");
        printf("║ Time:   %-45s ║\n", time_str);
        printf("║ Brain:   %-45s ║\n", brain_file);
        printf("║ Size:    %-45s ║\n", file_size_str);
        printf("╚══════════════════════════════════════════════════════════╝\n\n");
        
        // Display statistics
        printf("Statistics:\n");
        printf("  Nodes:      %zu", nodes);
        if (delta_nodes > 0) {
            printf(" (+%zu @ %.1f/sec)", delta_nodes, node_rate);
        }
        printf("\n");
        
        printf("  Edges:      %zu", edges);
        if (delta_edges > 0) {
            printf(" (+%zu @ %.1f/sec)", delta_edges, edge_rate);
        }
        printf("\n");
        
        printf("  Adaptations: %llu", (unsigned long long)adaptations);
        if (delta_adaptations > 0) {
            printf(" (+%llu)", (unsigned long long)delta_adaptations);
        }
        printf("\n");
        
        // Compute ratios
        double edge_per_node = nodes > 0 ? (double)edges / nodes : 0.0;
        printf("  Edges/Node: %.2f\n", edge_per_node);
        
        // Compute growth rates
        if (last_nodes > 0) {
            double node_growth = 100.0 * delta_nodes / last_nodes;
            double edge_growth = last_edges > 0 ? 
                100.0 * delta_edges / last_edges : 0.0;
            
            if (delta_nodes > 0 || delta_edges > 0) {
                printf("\nGrowth (last %d second%s):\n", interval, interval == 1 ? "" : "s");
                if (delta_nodes > 0) {
                    printf("  Nodes:      +%.2f%%\n", node_growth);
                }
                if (delta_edges > 0) {
                    printf("  Edges:      +%.2f%%\n", edge_growth);
                }
            }
        }
        
        printf("\nStatus: ");
        if (delta_nodes > 0 || delta_edges > 0) {
            printf("Processing... (active)\n");
        } else {
            printf("Idle (waiting for input)\n");
        }
        
        // Update last values
        last_nodes = nodes;
        last_edges = edges;
        last_adaptations = adaptations;
        last_update = now;
        
        melvin_m_close(mfile);
        
        sleep(interval);
    }
    
    return 0;
}

