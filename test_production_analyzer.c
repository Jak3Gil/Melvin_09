#include "melvin.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m>\n", argv[0]);
        return 1;
    }
    
    const char *brain_path = argv[1];
    
    // Open .m file
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to open brain file\n");
        return 1;
    }
    
    printf("==========================================\n");
    printf("Brain Analysis (melvin_out_port)\n");
    printf("==========================================\n");
    printf("\n");
    
    // Statistics
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    uint64_t adaptations = melvin_m_get_adaptation_count(mfile);
    
    printf("Statistics:\n");
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    printf("  Adaptations: %llu\n", (unsigned long long)adaptations);
    printf("\n");
    
    // Check for output
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("Output:\n");
        printf("  Size: %zu bytes\n", output_size);
        
        // Read and display output
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            printf("  Content (first 200 bytes): ");
            for (size_t i = 0; i < output_size && i < 200; i++) {
                if (output[i] >= 32 && output[i] < 127) {
                    printf("%c", output[i]);
                } else {
                    printf("\\x%02x", output[i]);
                }
            }
            if (output_size > 200) {
                printf("... (truncated)");
            }
            printf("\n");
            free(output);
        }
        printf("\n");
    } else {
        printf("No output generated (pure thinking mode)\n");
        printf("\n");
    }
    
    // Display output using melvin_out_port
    printf("==========================================\n");
    printf("Output Port Analysis\n");
    printf("==========================================\n");
    printf("\n");
    
    // Set up routing (input port 1 -> output port 1)
    PortRoute route = {1, 1};
    melvin_out_port_set_routing_table(&route, 1);
    
    // Read and route output
    printf("Routing output...\n");
    int result = melvin_out_port_read_and_route(mfile);
    if (result == 0) {
        printf("  Output routed successfully\n");
    } else {
        printf("  No output to route (or routing not configured)\n");
    }
    printf("\n");
    
    // Display output
    printf("Displaying output:\n");
    melvin_out_port_display_output(mfile);
    printf("\n");
    
    melvin_m_close(mfile);
    return 0;
}
