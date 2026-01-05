/* Test to inspect graph structure after learning */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Graph Structure Test ===\n\n");
    
    const char *path = "test_graph.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Input: '%s'\n", pattern);
    melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
    melvin_m_process_input(mfile);
    
    printf("\nNodes created: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges created: %zu\n\n", melvin_m_get_edge_count(mfile));
    
    // Note: We can't directly access the graph structure from here
    // but we can infer from the counts
    
    printf("Expected structure:\n");
    printf("  Nodes: h, e, ll (2-byte), o\n");
    printf("  Edges: h→e, e→ll, ll→o\n");
    printf("  Problem: ll node might not have outgoing edge to continue\n\n");
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[100];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Output: '");
        for (size_t i = 0; i < read_size; i++) {
            printf("%c", output[i]);
        }
        printf("'\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

