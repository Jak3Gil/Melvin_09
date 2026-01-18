#include "../src/melvin.h"
#include <stdio.h>
#include <string.h>

int main() {
    printf("=== Context Transformer Test ===\n\n");
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create("test_context.m");
    if (!mfile) {
        fprintf(stderr, "Error: Failed to create brain file\n");
        return 1;
    }
    
    // Train on ambiguous patterns
    const char *patterns[] = {
        "the bank by the river",  // bank = financial institution near river
        "the bank loaned money",  // bank = financial institution
        "the river bank was muddy", // bank = river edge
        "the bank by the river",
        "the bank loaned money",
        "the river bank was muddy"
    };
    
    printf("Training on ambiguous patterns:\n");
    for (int i = 0; i < 6; i++) {
        printf("  %d. %s\n", i+1, patterns[i]);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (uint8_t*)patterns[i], strlen(patterns[i]));
        melvin_m_process_input(mfile);
    }
    
    printf("\n--- Testing Context Disambiguation ---\n");
    
    // Test 1: "the bank" after "river" context
    printf("\nTest 1: Context = 'the river'\n");
    printf("Input:  'the river bank'\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (uint8_t*)"the river bank", 14);
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    uint8_t output[256];
    size_t bytes_read = melvin_m_universal_output_read(mfile, output, 256);
    printf("Output: '%.*s' (len=%zu)\n", (int)bytes_read, output, bytes_read);
    
    // Test 2: "the bank" after "money" context
    printf("\nTest 2: Context = 'loaned money'\n");
    printf("Input:  'the bank loaned'\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (uint8_t*)"the bank loaned", 15);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    bytes_read = melvin_m_universal_output_read(mfile, output, 256);
    printf("Output: '%.*s' (len=%zu)\n", (int)bytes_read, output, bytes_read);
    
    // Test 3: Simple pattern to verify basic functionality
    printf("\nTest 3: Simple pattern 'hello world'\n");
    const char *simple_patterns[] = {
        "hello world",
        "hello world",
        "hello world"
    };
    
    for (int i = 0; i < 3; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (uint8_t*)simple_patterns[i], strlen(simple_patterns[i]));
        melvin_m_process_input(mfile);
    }
    
    printf("Input:  'hello'\n");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (uint8_t*)"hello", 5);
    melvin_m_process_input(mfile);
    
    output_size = melvin_m_universal_output_size(mfile);
    bytes_read = melvin_m_universal_output_read(mfile, output, 256);
    printf("Output: '%.*s' (len=%zu)\n", (int)bytes_read, output, bytes_read);
    
    printf("\n--- Statistics ---\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("Adaptations: %lu\n", (unsigned long)melvin_m_get_adaptation_count(mfile));
    
    melvin_m_close(mfile);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
