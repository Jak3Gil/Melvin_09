#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "melvin.h"

void test_diversity(const char *name, const char *data, size_t data_len, int iterations) {
    char filename[256];
    snprintf(filename, sizeof(filename), "div_%s.m", name);
    
    remove(filename);
    
    MelvinMFile *mfile = melvin_m_create(filename);
    if (!mfile) return;
    
    for (int i = 0; i < iterations; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)data, data_len);
        melvin_m_process_input(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    size_t edges = melvin_m_get_edge_count(mfile);
    
    melvin_m_close(mfile);
    
    struct stat st;
    long file_size = 0;
    if (stat(filename, &st) == 0) {
        file_size = st.st_size;
    }
    
    float compression = (file_size > 0) ? ((float)data_len / (float)file_size) : 0.0f;
    float edge_per_node = (nodes > 0) ? ((float)edges / (float)nodes) : 0.0f;
    
    printf("%-15s | %6zu | %8ld | %5zu | %5zu | %5.2f | %4.2f\n",
           name, data_len, file_size, nodes, edges, compression, edge_per_node);
}

int main() {
    printf("=== DIVERSITY SCALABILITY TEST ===\n\n");
    printf("Data Type       | Input  | File     | Nodes | Edges | Comp  | E/N\n");
    printf("                | (bytes)| (bytes)  |       |       | Ratio |\n");
    printf("---------------------------------------------------------------------\n");
    
    // Test 1: Repetitive (low entropy)
    test_diversity("repetitive", "aaaaaaaaaa", 10, 10);
    
    // Test 2: Sequential (alphabet)
    test_diversity("sequential", "abcdefghijklmnopqrstuvwxyz", 26, 10);
    
    // Test 3: Random-like (high entropy)
    test_diversity("random", "qzxjkvbpmwfhgldnrtycusoiea", 26, 10);
    
    // Test 4: Natural language (English)
    const char *english = "The quick brown fox jumps over the lazy dog";
    test_diversity("english", english, strlen(english), 10);
    
    // Test 5: Code-like (structured)
    const char *code = "int main() { return 0; } void func() { }";
    test_diversity("code", code, strlen(code), 10);
    
    // Test 6: Numbers
    const char *numbers = "0123456789012345678901234567890123456789";
    test_diversity("numbers", numbers, strlen(numbers), 10);
    
    // Test 7: Mixed (high diversity)
    const char *mixed = "Hello123!@# World456$%^ Test789&*() End";
    test_diversity("mixed", mixed, strlen(mixed), 10);
    
    // Test 8: Repeated patterns
    const char *patterns = "abcabcabcabcabcabcabcabcabcabcabcabcabc";
    test_diversity("patterns", patterns, strlen(patterns), 10);
    
    // Test 9: Long repeated sequence
    char *long_rep = malloc(1000);
    if (long_rep) {
        for (int i = 0; i < 100; i++) {
            memcpy(long_rep + i*10, "helloworld", 10);
        }
        test_diversity("long_repeat", long_rep, 1000, 5);
        free(long_rep);
    }
    
    // Test 10: Long diverse sequence
    char *long_div = malloc(1000);
    if (long_div) {
        const char *words[] = {"alpha", "beta", "gamma", "delta", "epsilon", 
                               "zeta", "eta", "theta", "iota", "kappa"};
        int pos = 0;
        for (int i = 0; i < 100 && pos < 990; i++) {
            const char *word = words[i % 10];
            int len = strlen(word);
            memcpy(long_div + pos, word, len);
            pos += len;
            long_div[pos++] = ' ';
        }
        test_diversity("long_diverse", long_div, pos, 5);
        free(long_div);
    }
    
    printf("\n=== OBSERVATIONS ===\n");
    printf("Repetitive data: Should have high compression (few unique nodes)\n");
    printf("Diverse data: Lower compression (many unique nodes)\n");
    printf("Natural language: Moderate compression (repeated words/patterns)\n");
    printf("\nHigh compression ratio = System efficiently recognizes patterns\n");
    
    return 0;
}
