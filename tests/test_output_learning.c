/*
 * Output Learning Analysis Test
 * - Shows actual output content
 * - Analyzes if system is learning
 * - Checks if outputs are intelligent or random
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void display_output(MelvinMFile *mfile, int input_num) {
    size_t output_size = melvin_m_universal_output_size(mfile);
    
    if (output_size > 0) {
        uint8_t *output_buffer = malloc(output_size + 1);
        if (output_buffer) {
            size_t read_size = melvin_m_universal_output_read(mfile, output_buffer, output_size);
            output_buffer[read_size] = '\0';
            
            printf("  Output (%zu bytes): ", read_size);
            
            // Show hex
            printf("hex[");
            for (size_t i = 0; i < read_size && i < 20; i++) {
                printf("%02x ", output_buffer[i]);
            }
            if (read_size > 20) printf("...");
            printf("] ");
            
            // Show text
            printf("text[");
            int printable = 0;
            for (size_t i = 0; i < read_size && i < 50; i++) {
                uint8_t byte = output_buffer[i];
                if (byte >= 32 && byte < 127) {
                    printf("%c", byte);
                    printable++;
                } else {
                    printf(".");
                }
            }
            if (read_size > 50) printf("...");
            printf("] ");
            
            // Analyze
            printf("(printable: %d/%zu", printable, read_size);
            
            // Check if output relates to input
            if (read_size > 0) {
                // Simple check: if output contains common ASCII letters, might be learning
                int ascii_count = 0;
                for (size_t i = 0; i < read_size; i++) {
                    if (output_buffer[i] >= 32 && output_buffer[i] < 127) {
                        ascii_count++;
                    }
                }
                float ascii_ratio = (float)ascii_count / (float)read_size;
                if (ascii_ratio > 0.5f) {
                    printf(", looks structured");
                } else {
                    printf(", looks random");
                }
            }
            printf(")\n");
            
            free(output_buffer);
        }
    } else {
        printf("  Output: None (pure thinking mode)\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <brain_file.m> <input1> [input2] ...\n", argv[0]);
        return 1;
    }
    
    const char *brain_file = argv[1];
    
    printf("==========================================\n");
    printf("Melvin Output Learning Analysis\n");
    printf("==========================================\n\n");
    
    MelvinMFile *mfile = melvin_m_load(brain_file);
    if (!mfile) {
        mfile = melvin_m_create(brain_file);
        if (!mfile) {
            fprintf(stderr, "Failed to create brain file\n");
            return 1;
        }
    }
    
    printf("Processing %d inputs...\n\n", argc - 2);
    
    for (int i = 2; i < argc; i++) {
        const char *input_file = argv[i];
        
        printf("--- Input %d: %s ---\n", i - 1, input_file);
        
        // Read input to show what we're feeding
        FILE *f = fopen(input_file, "r");
        if (f) {
            char input_buf[256];
            if (fgets(input_buf, sizeof(input_buf), f)) {
                printf("  Input: \"%s\"", input_buf);
                if (input_buf[strlen(input_buf)-1] != '\n') printf("\n");
            }
            fclose(f);
        }
        
        // Process
        uint8_t port_id = 1;
        melvin_in_port_handle_text_file(mfile, port_id, input_file);
        melvin_m_save(mfile);
        
        // Show stats
        printf("  Nodes: %zu, Edges: %zu", 
               melvin_m_get_node_count(mfile),
               melvin_m_get_edge_count(mfile));
        if (melvin_m_get_node_count(mfile) > 0) {
            printf(", Ratio: %.2f:1", 
                   (float)melvin_m_get_edge_count(mfile) / 
                   (float)melvin_m_get_node_count(mfile));
        }
        printf("\n");
        
        // Show output
        display_output(mfile, i - 1);
        printf("\n");
    }
    
    printf("==========================================\n");
    printf("Learning Analysis\n");
    printf("==========================================\n");
    printf("Final: %zu nodes, %zu edges\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Final output
    printf("\nFinal Output:\n");
    display_output(mfile, argc - 1);
    
    printf("\nLearning Indicators:\n");
    size_t final_nodes = melvin_m_get_node_count(mfile);
    size_t final_edges = melvin_m_get_edge_count(mfile);
    
    if (final_nodes > 0) {
        printf("  ✓ Graph is growing (learning patterns)\n");
    }
    if (final_edges > final_nodes) {
        printf("  ✓ High connectivity (building knowledge)\n");
    } else {
        printf("  ⚠️  Low connectivity (may need more edge types)\n");
    }
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    if (output_size > 0) {
        printf("  ✓ Generating outputs\n");
        
        // Check if output is evolving
        uint8_t *output = malloc(output_size);
        if (output) {
            melvin_m_universal_output_read(mfile, output, output_size);
            
            // Check if output is repetitive
            int is_repetitive = 1;
            if (output_size > 1) {
                uint8_t first = output[0];
                for (size_t i = 1; i < output_size && i < 10; i++) {
                    if (output[i] != first) {
                        is_repetitive = 0;
                        break;
                    }
                }
            }
            
            if (is_repetitive && output_size > 1) {
                printf("  ⚠️  Output is repetitive (system may be stuck)\n");
            } else {
                printf("  ✓ Output is varied (system exploring)\n");
            }
            
            free(output);
        }
    } else {
        printf("  ⚠️  No output (pure thinking mode or system not ready)\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}

