/*
 * Test program for processing large text datasets with Melvin
 * Usage: ./test_dataset <dataset_file> <brain.m>
 * Or: ./test_dataset (interactive mode - reads from stdin)
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    const char *brain_file = "brain.m";
    const char *input_file = NULL;
    
    // Parse arguments
    if (argc >= 2) {
        input_file = argv[1];
    }
    if (argc >= 3) {
        brain_file = argv[2];
    }
    
    printf("Melvin: Interactive Test\n");
    printf("Brain: %s\n\n", brain_file);
    
    // Create or open .m file
    MelvinMFile *mfile = melvin_m_create(brain_file);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to create/open brain file\n");
        return 1;
    }
    
    // Test 1: Process input file if provided
    if (input_file) {
        printf("=== Test 1: Processing file ===\n");
        printf("Input file: %s\n", input_file);
        
        int result = melvin_in_port_handle_text_file(mfile, 0, input_file);
        if (result < 0) {
            fprintf(stderr, "Error: Failed to process file\n");
        } else {
            printf("Processed successfully\n");
            
            // Read and display output
            printf("\nOutput: ");
            melvin_out_port_display_output(mfile);
        }
        printf("\n");
    }
    
    // Test 2: Interactive input/output loop
    printf("=== Test 2: Interactive Input/Output ===\n");
    printf("Type input (or 'quit' to exit):\n");
    
    char input_line[1024];
    while (1) {
        printf("\n> ");
        fflush(stdout);
        
        if (!fgets(input_line, sizeof(input_line), stdin)) {
            break;
        }
        
        // Remove newline
        size_t len = strlen(input_line);
        if (len > 0 && input_line[len-1] == '\n') {
            input_line[len-1] = '\0';
            len--;
        }
        
        if (len == 0) continue;
        
        // Check for quit
        if (strcmp(input_line, "quit") == 0 || strcmp(input_line, "exit") == 0) {
            break;
        }
        
        // Feed input through melvin_in_port
        printf("Processing: \"%s\"\n", input_line);
        int result = melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)input_line, len);
        
        if (result < 0) {
            fprintf(stderr, "Error processing input\n");
            continue;
        }
        
        // Read and display output using melvin_out_port
        printf("Output: ");
        melvin_out_port_display_output(mfile);
        
        // Show statistics
        printf("  (Nodes: %zu, Edges: %zu)\n", 
               melvin_m_get_node_count(mfile), 
               melvin_m_get_edge_count(mfile));
    }
    
    // Final statistics
    printf("\n=== Final Statistics ===\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Adaptations: %lu\n", (unsigned long)melvin_m_get_adaptation_count(mfile));
    
    // Cleanup
    melvin_m_close(mfile);
    
    printf("\nDone!\n");
    return 0;
}

