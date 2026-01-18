/* Test associations - verify edges are created and associations work */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/melvin.h"

int main(void) {
    printf("=== Testing Associations ===\n\n");
    
    // Create brain
    MelvinMFile *mfile = melvin_m_create("test_associations.m");
    if (!mfile) {
        fprintf(stderr, "Failed to create brain file\n");
        return 1;
    }
    
    // Train on simple patterns
    const char *patterns[] = {
        "hello world",
        "hello friend",
        "goodbye world",
        "cat dog bird",
        "apple banana cherry"
    };
    
    printf("Training on %zu patterns (10 iterations each)...\n", sizeof(patterns)/sizeof(patterns[0]));
    
    for (int iter = 0; iter < 10; iter++) {
        for (size_t i = 0; i < sizeof(patterns)/sizeof(patterns[0]); i++) {
            melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[i], strlen(patterns[i]));
            melvin_m_process_input(mfile);
        }
    }
    
    // Get graph stats using API
    size_t node_count = melvin_m_get_node_count(mfile);
    size_t edge_count = melvin_m_get_edge_count(mfile);
    
    printf("\nGraph Statistics:\n");
    printf("  Nodes: %zu\n", node_count);
    printf("  Edges: %zu\n", edge_count);
    
    if (node_count > 0) {
        float ratio = (float)edge_count / (float)node_count;
        printf("  Edge-to-Node Ratio: %.2f:1\n", ratio);
        
        if (ratio >= 10.0f) {
            printf("  ✅ PASS: Ratio >= 10:1 (associations working!)\n");
        } else if (ratio >= 5.0f) {
            printf("  ⚠️  PARTIAL: Ratio >= 5:1 (some associations)\n");
        } else {
            printf("  ❌ FAIL: Ratio < 5:1 (associations not working)\n");
        }
    }
    
    // Test output generation
    printf("\nTesting Output Generation:\n");
    
    const char *test_inputs[] = {
        "hello",
        "cat",
        "apple"
    };
    
    for (size_t i = 0; i < sizeof(test_inputs)/sizeof(test_inputs[0]); i++) {
        printf("\n  Input: \"%s\"\n", test_inputs[i]);
        
        melvin_m_universal_input_write(mfile, (const uint8_t*)test_inputs[i], strlen(test_inputs[i]));
        melvin_m_process_input(mfile);
        
        uint8_t output[256];
        size_t output_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
        
        printf("  Output: \"");
        for (size_t j = 0; j < output_len && j < 50; j++) {
            if (output[j] >= 32 && output[j] < 127) {
                printf("%c", output[j]);
            } else {
                printf("?");
            }
        }
        printf("\" (%zu bytes)\n", output_len);
        
        melvin_m_universal_output_clear(mfile);
    }
    
    // Cleanup
    melvin_m_close(mfile);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
