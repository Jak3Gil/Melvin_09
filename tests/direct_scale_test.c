#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "melvin.h"

void test_scale(const char *name, const char *data, size_t data_len, int iterations) {
    char filename[256];
    snprintf(filename, sizeof(filename), "scale_%s.m", name);
    
    // Remove old file
    remove(filename);
    
    // Create and train
    MelvinMFile *mfile = melvin_m_create(filename);
    if (!mfile) {
        printf("ERROR: Failed to create %s\n", filename);
        return;
    }
    
    // Train multiple iterations
    for (int i = 0; i < iterations; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)data, data_len);
        melvin_m_process_input(mfile);
    }
    
    // Get stats
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    melvin_m_close(mfile);
    
    // Get file size
    struct stat st;
    long file_size = 0;
    if (stat(filename, &st) == 0) {
        file_size = st.st_size;
    }
    
    // Calculate metrics
    float compression = (file_size > 0) ? ((float)data_len / (float)file_size) : 0.0f;
    float edge_per_node = (nodes > 0) ? ((float)edges / (float)nodes) : 0.0f;
    
    printf("%-10s | %6zu | %8ld | %5zu | %5zu | %5.2f | %4.2f\n",
           name, data_len, file_size, nodes, edges, compression, edge_per_node);
}

int main() {
    printf("=== .M FILE SCALABILITY TEST ===\n\n");
    printf("Dataset    | Input  | File     | Nodes | Edges | Comp  | E/N\n");
    printf("           | (bytes)| (bytes)  |       |       | Ratio |\n");
    printf("----------------------------------------------------------------\n");
    
    // Test 1: Tiny (10 bytes)
    test_scale("tiny", "hello", 5, 10);
    
    // Test 2: Small (60 bytes)
    test_scale("small", "hello world\nthe cat sat\ngood morning\n", 39, 10);
    
    // Test 3: Medium (500 bytes)
    const char *medium = 
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "How vexingly quick daft zebras jump! "
        "The five boxing wizards jump quickly. "
        "Sphinx of black quartz, judge my vow. "
        "Waltz, bad nymph, for quick jigs vex. "
        "Quick zephyrs blow, vexing daft Jim. "
        "Two driven jocks help fax my big quiz. "
        "Five quacking zephyrs jolt my wax bed. "
        "The jay, pig, fox, zebra and my wolves quack!";
    test_scale("medium", medium, strlen(medium), 5);
    
    // Test 4: Large (5KB - generated)
    char *large = malloc(5000);
    if (large) {
        const char *words[] = {"hello", "world", "the", "cat", "sat", "good", "morning"};
        int pos = 0;
        for (int i = 0; i < 500 && pos < 4900; i++) {
            int word_idx = i % 7;
            int len = strlen(words[word_idx]);
            memcpy(large + pos, words[word_idx], len);
            pos += len;
            large[pos++] = ' ';
        }
        large[pos] = '\0';
        test_scale("large", large, pos, 3);
        free(large);
    }
    
    // Test 5: XLarge (50KB - generated)
    char *xlarge = malloc(50000);
    if (xlarge) {
        const char *words[] = {"hello", "world", "the", "cat", "sat"};
        int pos = 0;
        for (int i = 0; i < 5000 && pos < 49900; i++) {
            int word_idx = i % 5;
            int len = strlen(words[word_idx]);
            memcpy(xlarge + pos, words[word_idx], len);
            pos += len;
            xlarge[pos++] = ' ';
        }
        xlarge[pos] = '\0';
        test_scale("xlarge", xlarge, pos, 2);
        free(xlarge);
    }
    
    printf("\n=== ANALYSIS ===\n");
    printf("Compression Ratio = Input Size / File Size\n");
    printf("  > 1.0 = File is smaller than input (good compression)\n");
    printf("  < 1.0 = File is larger than input (storing structure)\n");
    printf("\nEdge/Node Ratio:\n");
    printf("  ~2.0 = Optimal sparse graph (linear structure)\n");
    printf("  >3.0 = Dense graph (many connections)\n");
    
    return 0;
}
