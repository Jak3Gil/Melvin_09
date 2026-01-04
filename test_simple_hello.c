/* Simple test to verify multi-byte patterns and loop detection */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Simple Hello Test ===\n\n");
    
    const char *path = "test_simple_hello.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Test: Input '%s' once\n", pattern);
    melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
    melvin_m_process_input(mfile);
    
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[256];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Output (%zu bytes): '", read_size);
        for (size_t i = 0; i < read_size && i < 50; i++) {
            if (output[i] >= 32 && output[i] < 127) {
                printf("%c", output[i]);
            } else {
                printf(".");
            }
        }
        if (read_size > 50) printf("...");
        printf("'\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

