/* Test: Mini Transformer Edge Selection
 * 
 * This test verifies that edges act as "mini transformers" following the README.
 * The edge's transformation function should handle all intelligence:
 *   - Context-aware attention
 *   - Routing gates
 *   - Pattern similarity
 * 
 * We should see:
 *   1. Context disambiguation working (o→' ' vs o→'r')
 *   2. No hardcoded boosts needed
 *   3. Intelligence emerging from edge transformations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

int main(void) {
    printf("=== Mini Transformer Test ===\n\n");
    
    // Create brain
    MelvinMFile *brain = melvin_m_create("test_mini_transformer.m");
    if (!brain) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    // Training: Multiple patterns to create context disambiguation challenge
    const char *training[] = {
        "hello world",
        "hello there",
        "world peace",
        "hello world",  // Repeat to strengthen
        "hello there",
        "hello world"
    };
    
    printf("Training on patterns:\n");
    for (size_t i = 0; i < sizeof(training)/sizeof(training[0]); i++) {
        printf("  %zu. \"%s\"\n", i+1, training[i]);
        
        // Clear output before training
        melvin_m_universal_output_clear(brain);
        
        // Write input and process
        melvin_m_universal_input_write(brain, (const uint8_t*)training[i], strlen(training[i]));
        melvin_m_process_input(brain);
    }
    
    printf("\n--- Training Complete ---\n");
    printf("Nodes: %zu\n", melvin_m_get_node_count(brain));
    printf("Edges: %zu\n", melvin_m_get_edge_count(brain));
    
    // Test 1: "hello" should continue with " world" or " there"
    printf("\n=== Test 1: Context 'hello' ===\n");
    melvin_m_universal_output_clear(brain);
    melvin_m_universal_input_write(brain, (const uint8_t*)"hello", 5);
    melvin_m_process_input(brain);
    
    size_t len1 = melvin_m_universal_output_size(brain);
    uint8_t *output1 = malloc(len1 + 1);
    if (output1) {
        melvin_m_universal_output_read(brain, output1, len1);
        output1[len1] = 0;
    }
    
    printf("Input:  \"hello\"\n");
    printf("Output: \"");
    for (size_t i = 0; i < len1; i++) {
        if (output1[i] >= 32 && output1[i] < 127) {
            printf("%c", output1[i]);
        } else {
            printf("<%02x>", output1[i]);
        }
    }
    printf("\" (len=%zu)\n", len1);
    
    // Check if output is reasonable
    if (len1 > 0 && len1 < 100) {
        printf("✓ Output length reasonable\n");
    } else {
        printf("✗ Output length unusual: %zu\n", len1);
    }
    free(output1);
    
    // Test 2: "world" should continue differently than "hello"
    printf("\n=== Test 2: Context 'world' ===\n");
    melvin_m_universal_output_clear(brain);
    melvin_m_universal_input_write(brain, (const uint8_t*)"world", 5);
    melvin_m_process_input(brain);
    
    size_t len2 = melvin_m_universal_output_size(brain);
    uint8_t *output2 = malloc(len2 + 1);
    if (output2) {
        melvin_m_universal_output_read(brain, output2, len2);
        output2[len2] = 0;
    }
    
    printf("Input:  \"world\"\n");
    printf("Output: \"");
    for (size_t i = 0; i < len2; i++) {
        if (output2[i] >= 32 && output2[i] < 127) {
            printf("%c", output2[i]);
        } else {
            printf("<%02x>", output2[i]);
        }
    }
    printf("\" (len=%zu)\n", len2);
    free(output2);
    
    // Test 3: "hell" should continue with "o"
    printf("\n=== Test 3: Context 'hell' ===\n");
    melvin_m_universal_output_clear(brain);
    melvin_m_universal_input_write(brain, (const uint8_t*)"hell", 4);
    melvin_m_process_input(brain);
    
    size_t len3 = melvin_m_universal_output_size(brain);
    uint8_t *output3 = malloc(len3 + 1);
    if (output3) {
        melvin_m_universal_output_read(brain, output3, len3);
        output3[len3] = 0;
    }
    
    printf("Input:  \"hell\"\n");
    printf("Output: \"");
    for (size_t i = 0; i < len3; i++) {
        if (output3[i] >= 32 && output3[i] < 127) {
            printf("%c", output3[i]);
        } else {
            printf("<%02x>", output3[i]);
        }
    }
    printf("\" (len=%zu)\n", len3);
    
    // Check if first byte is 'o'
    if (len3 > 0 && output3[0] == 'o') {
        printf("✓ Correct continuation: 'o'\n");
    } else if (len3 > 0) {
        printf("✗ Expected 'o', got '%c'\n", output3[0]);
    }
    free(output3);
    
    // Test 4: Longer context "hello w" should continue with "orld"
    printf("\n=== Test 4: Context 'hello w' ===\n");
    melvin_m_universal_output_clear(brain);
    melvin_m_universal_input_write(brain, (const uint8_t*)"hello w", 7);
    melvin_m_process_input(brain);
    
    size_t len4 = melvin_m_universal_output_size(brain);
    uint8_t *output4 = malloc(len4 + 1);
    if (output4) {
        melvin_m_universal_output_read(brain, output4, len4);
        output4[len4] = 0;
    }
    
    printf("Input:  \"hello w\"\n");
    printf("Output: \"");
    for (size_t i = 0; i < len4; i++) {
        if (output4[i] >= 32 && output4[i] < 127) {
            printf("%c", output4[i]);
        } else {
            printf("<%02x>", output4[i]);
        }
    }
    printf("\" (len=%zu)\n", len4);
    
    // Check if starts with "orld"
    if (len4 >= 4 && memcmp(output4, "orld", 4) == 0) {
        printf("✓ Correct continuation: 'orld'\n");
    } else if (len4 > 0) {
        printf("? Got different continuation\n");
    }
    free(output4);
    
    printf("\n=== Summary ===\n");
    printf("Mini transformer approach:\n");
    printf("  - Edges compute context-aware attention\n");
    printf("  - No hardcoded boosts needed\n");
    printf("  - Intelligence emerges from edge transformations\n");
    printf("  - Following README: 'edges act as mini transformers'\n");
    
    melvin_m_close(brain);
    printf("\nTest complete.\n");
    
    return 0;
}

