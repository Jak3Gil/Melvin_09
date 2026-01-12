/*
 * Large Corpus Training Tool
 * 
 * Feeds large amounts of text data into Melvin to create a unified .m brain file.
 * Monitors graph growth and statistics during training.
 * 
 * Usage: ./train_large_corpus <output.m> <input_file1> [input_file2] ...
 */

#include "../src/melvin.h"
#include "../src/melvin_in_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

/* Statistics tracking */
typedef struct {
    size_t total_bytes_fed;
    size_t total_lines_fed;
    size_t total_files_fed;
    size_t current_nodes;
    size_t current_edges;
    time_t start_time;
    time_t last_checkpoint;
} TrainingStats;

/* Print progress */
static void print_progress(TrainingStats *stats, MelvinMFile *mfile) {
    time_t now = time(NULL);
    double elapsed = difftime(now, stats->start_time);
    double mb_fed = (double)stats->total_bytes_fed / (1024.0 * 1024.0);
    double mb_per_sec = mb_fed / (elapsed + 1.0);
    
    stats->current_nodes = melvin_m_get_node_count(mfile);
    stats->current_edges = melvin_m_get_edge_count(mfile);
    
    printf("\r[%.1f MB | %.1f MB/s | %zu nodes | %zu edges | %.0fs]",
           mb_fed, mb_per_sec, stats->current_nodes, stats->current_edges, elapsed);
    fflush(stdout);
}

/* Feed a single line of text */
static int feed_line(MelvinMFile *mfile, const char *line, size_t len, TrainingStats *stats) {
    if (len == 0) return 0;
    
    // Feed the line
    int result = melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)line, len);
    if (result < 0) {
        fprintf(stderr, "\nError feeding line\n");
        return -1;
    }
    
    stats->total_bytes_fed += len;
    stats->total_lines_fed++;
    
    // Print progress every 1MB
    if (stats->total_bytes_fed - stats->last_checkpoint >= 1024 * 1024) {
        print_progress(stats, mfile);
        stats->last_checkpoint = stats->total_bytes_fed;
    }
    
    return 0;
}

/* Feed a file line by line */
static int feed_file(MelvinMFile *mfile, const char *filepath, TrainingStats *stats) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "Error opening file: %s\n", filepath);
        return -1;
    }
    
    printf("\nFeeding file: %s\n", filepath);
    
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t line_len;
    
    while ((line_len = getline(&line, &line_cap, fp)) > 0) {
        // Remove newline
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
            line_len--;
        }
        
        // Skip empty lines
        if (line_len == 0) continue;
        
        // Feed the line
        if (feed_line(mfile, line, line_len, stats) < 0) {
            free(line);
            fclose(fp);
            return -1;
        }
    }
    
    free(line);
    fclose(fp);
    stats->total_files_fed++;
    
    return 0;
}

/* Feed all .txt files in a directory recursively */
static int feed_directory(MelvinMFile *mfile, const char *dirpath, TrainingStats *stats) {
    DIR *dir = opendir(dirpath);
    if (!dir) {
        fprintf(stderr, "Error opening directory: %s\n", dirpath);
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        // Build full path
        char fullpath[4096];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", dirpath, entry->d_name);
        
        // Check if it's a directory
        struct stat st;
        if (stat(fullpath, &st) < 0) continue;
        
        if (S_ISDIR(st.st_mode)) {
            // Recursively feed directory
            feed_directory(mfile, fullpath, stats);
        } else if (S_ISREG(st.st_mode)) {
            // Check if it's a .txt file
            size_t name_len = strlen(entry->d_name);
            if (name_len > 4 && strcmp(entry->d_name + name_len - 4, ".txt") == 0) {
                feed_file(mfile, fullpath, stats);
            }
        }
    }
    
    closedir(dir);
    return 0;
}

/* Main */
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <output.m> <input_file_or_dir> [input2] ...\n", argv[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Feeds large amounts of text data into Melvin.\n");
        fprintf(stderr, "Input can be:\n");
        fprintf(stderr, "  - Text files (.txt)\n");
        fprintf(stderr, "  - Directories (recursively processes all .txt files)\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Example:\n");
        fprintf(stderr, "  %s trained_brain.m data/wikipedia/*.txt\n", argv[0]);
        fprintf(stderr, "  %s trained_brain.m data/books/\n", argv[0]);
        return 1;
    }
    
    const char *output_path = argv[1];
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║         MELVIN LARGE CORPUS TRAINING                      ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Output brain: %s\n", output_path);
    printf("Input sources: %d\n", argc - 2);
    printf("\n");
    
    // Create or open brain file
    printf("Creating brain file...\n");
    MelvinMFile *mfile = melvin_m_create(output_path);
    if (!mfile) {
        fprintf(stderr, "Error creating brain file: %s\n", output_path);
        return 1;
    }
    
    // Initialize statistics
    TrainingStats stats = {0};
    stats.start_time = time(NULL);
    stats.last_checkpoint = 0;
    
    printf("Starting training...\n");
    printf("\n");
    
    // Feed all input files/directories
    for (int i = 2; i < argc; i++) {
        const char *input = argv[i];
        
        // Check if it's a directory or file
        struct stat st;
        if (stat(input, &st) < 0) {
            fprintf(stderr, "\nWarning: Cannot access %s, skipping\n", input);
            continue;
        }
        
        if (S_ISDIR(st.st_mode)) {
            // Feed directory recursively
            feed_directory(mfile, input, &stats);
        } else if (S_ISREG(st.st_mode)) {
            // Feed single file
            feed_file(mfile, input, &stats);
        }
    }
    
    // Final progress
    print_progress(&stats, mfile);
    printf("\n\n");
    
    // Print final statistics
    time_t end_time = time(NULL);
    double total_time = difftime(end_time, stats.start_time);
    double mb_total = (double)stats.total_bytes_fed / (1024.0 * 1024.0);
    double gb_total = mb_total / 1024.0;
    
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                  TRAINING COMPLETE                         ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Data Fed:\n");
    printf("  Total bytes:  %zu (%.2f GB)\n", stats.total_bytes_fed, gb_total);
    printf("  Total lines:  %zu\n", stats.total_lines_fed);
    printf("  Total files:  %zu\n", stats.total_files_fed);
    printf("\n");
    printf("Graph Statistics:\n");
    printf("  Nodes:        %zu\n", stats.current_nodes);
    printf("  Edges:        %zu\n", stats.current_edges);
    printf("  Avg degree:   %.2f\n", stats.current_nodes > 0 ? 
           (double)stats.current_edges / (double)stats.current_nodes : 0.0);
    printf("  Bytes/node:   %.2f\n", stats.current_nodes > 0 ?
           (double)stats.total_bytes_fed / (double)stats.current_nodes : 0.0);
    printf("\n");
    printf("Performance:\n");
    printf("  Training time: %.0f seconds (%.1f minutes)\n", total_time, total_time / 60.0);
    printf("  Throughput:    %.2f MB/s\n", mb_total / (total_time + 1.0));
    printf("  Lines/sec:     %.0f\n", (double)stats.total_lines_fed / (total_time + 1.0));
    printf("\n");
    printf("Brain saved to: %s\n", output_path);
    printf("\n");
    
    // Close brain
    melvin_m_close(mfile);
    
    return 0;
}
