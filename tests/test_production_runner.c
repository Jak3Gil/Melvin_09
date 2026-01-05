#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain.m> <input_file> [port_id]\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    const char *input_file = argv[2];
    uint8_t port_id = (argc > 3) ? (uint8_t)atoi(argv[3]) : 1;
    
    // Open or create .m file
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        mfile = melvin_m_create(brain_path);
        if (!mfile) {
            fprintf(stderr, "Error: Failed to create/open brain file\n");
            return 1;
        }
    }
    
    // Process input using melvin_in_port
    int result = melvin_in_port_handle_text_file(mfile, port_id, input_file);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to process input file\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    // Save the brain after processing
    melvin_m_save(mfile);
    
    // Get statistics
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
    
    printf("Processed: %s\n", input_file);
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    printf("  Adaptations: %llu\n", (unsigned long long)adaptations);
    
    // Check for output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("  Output size: %zu bytes\n", output_size);
    }
    
    melvin_m_close(mfile);
    return 0;
}
