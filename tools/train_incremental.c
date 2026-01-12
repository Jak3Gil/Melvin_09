/*
 * Incremental Training Tool
 * 
 * Trains Melvin incrementally with periodic saves and progress monitoring.
 * Processes data in small batches to avoid memory issues.
 * 
 * Usage: ./train_incremental <brain.m> <input.txt> [lines_per_batch]
 */

#include "../src/melvin.h"
#include "../src/melvin_in_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain.m> <input.txt> [lines_per_batch]\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    const char *input_path = argv[2];
    size_t lines_per_batch = (argc > 3) ? atoi(argv[3]) : 100;
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         MELVIN INCREMENTAL TRAINING                        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Brain: %s\n", brain_path);
    printf("Input: %s\n", input_path);
    printf("Batch size: %zu lines\n", lines_per_batch);
    printf("\n");
    
    // Open input file
    FILE *fp = fopen(input_path, "r");
    if (!fp) {
        fprintf(stderr, "Error opening input file: %s\n", input_path);
        return 1;
    }
    
    // Create brain
    printf("Creating brain...\n");
    MelvinMFile *mfile = melvin_m_create(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error creating brain file\n");
        fclose(fp);
        return 1;
    }
    
    printf("Training started...\n\n");
    
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;
    
    size_t total_lines = 0;
    size_t total_bytes = 0;
    size_t batch_count = 0;
    time_t start_time = time(NULL);
    
    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        // Remove newline
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
            line_len--;
        }
        
        // Skip empty lines
        if (line_len == 0) continue;
        
        // Feed line
        int result = melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)line, line_len);
        if (result < 0) {
            fprintf(stderr, "\nError feeding line %zu\n", total_lines);
            break;
        }
        
        total_lines++;
        total_bytes += line_len;
        
        // Print progress every batch
        if (total_lines % lines_per_batch == 0) {
            batch_count++;
            size_t nodes = melvin_m_get_node_count(mfile);
            size_t edges = melvin_m_get_edge_count(mfile);
            double mb = (double)total_bytes / (1024.0 * 1024.0);
            time_t now = time(NULL);
            double elapsed = difftime(now, start_time);
            double lines_per_sec = (elapsed > 0) ? (double)total_lines / elapsed : 0.0;
            
            printf("\rBatch %zu: %zu lines (%.2f MB) | %zu nodes, %zu edges | %.0f lines/sec",
                   batch_count, total_lines, mb, nodes, edges, lines_per_sec);
            fflush(stdout);
        }
    }
    
    free(line);
    fclose(fp);
    
    // Final statistics
    printf("\n\n");
    
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    double final_mb = (double)total_bytes / (1024.0 * 1024.0);
    time_t end_time = time(NULL);
    double total_time = difftime(end_time, start_time);
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  TRAINING COMPLETE                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Data Fed:\n");
    printf("  Lines:        %zu\n", total_lines);
    printf("  Bytes:        %zu (%.2f MB)\n", total_bytes, final_mb);
    printf("  Batches:      %zu\n", batch_count);
    printf("\n");
    printf("Graph:\n");
    printf("  Nodes:        %zu\n", final_nodes);
    printf("  Edges:        %zu\n", final_edges);
    printf("  Avg degree:   %.2f\n", final_nodes > 0 ? 
           (double)final_edges / (double)final_nodes : 0.0);
    printf("  Compression:  %.0f bytes/node\n", final_nodes > 0 ?
           (double)total_bytes / (double)final_nodes : 0.0);
    printf("\n");
    printf("Performance:\n");
    printf("  Time:         %.0f seconds (%.1f minutes)\n", total_time, total_time / 60.0);
    printf("  Throughput:   %.0f lines/sec\n", total_lines / (total_time + 1.0));
    printf("  Speed:        %.2f MB/sec\n", final_mb / (total_time + 1.0));
    printf("\n");
    printf("Brain saved: %s\n", brain_path);
    printf("\n");
    
    melvin_m_close(mfile);
    
    return 0;
}
