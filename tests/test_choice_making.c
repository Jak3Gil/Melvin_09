/* Test: Can Melvin make choices and learn conditional patterns?
 * 
 * This tests if Melvin can:
 * 1. Learn different responses based on context (like if-statements)
 * 2. Make choices between multiple valid paths
 * 3. Compound learning (learn multiple patterns that build on each other)
 */

#include "melvin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("=== Testing Choice Making & Conditional Learning ===\n\n");
    
    // Delete old brain file
    remove("test_choice.m");
    
    MelvinMFile *m = melvin_m_create("test_choice.m");
    if (!m) {
        fprintf(stderr, "Failed to create Melvin instance\n");
        return 1;
    }
    
    // Test 1: Can it learn conditional responses?
    // Pattern: "cat" → "meow", "dog" → "woof"
    printf("Test 1: Conditional Pattern Learning\n");
    printf("Training: 'cat' → 'meow', 'dog' → 'woof'\n\n");
    
    for (int i = 0; i < 50; i++) {
        // Train on cat → meow
        melvin_m_universal_input_write(m, (uint8_t*)"catmeow", 7);
        melvin_m_process_input(m);
        
        // Train on dog → woof
        melvin_m_universal_input_write(m, (uint8_t*)"dogwoof", 7);
        melvin_m_process_input(m);
    }
    
    // Test: Does it respond correctly to each input?
    printf("After 50 iterations of training:\n");
    
    // Test cat
    melvin_m_universal_input_write(m, (uint8_t*)"cat", 3);
    melvin_m_process_input(m);
    size_t cat_len = melvin_m_universal_output_size(m);
    uint8_t *cat_output = malloc(cat_len + 1);
    melvin_m_universal_output_read(m, cat_output, cat_len);
    cat_output[cat_len] = '\0';
    printf("  Input 'cat' → Output: '%s' (expected: 'meow')\n", cat_output);
    free(cat_output);
    melvin_m_universal_output_clear(m);
    
    // Test dog
    melvin_m_universal_input_write(m, (uint8_t*)"dog", 3);
    melvin_m_process_input(m);
    size_t dog_len = melvin_m_universal_output_size(m);
    uint8_t *dog_output = malloc(dog_len + 1);
    melvin_m_universal_output_read(m, dog_output, dog_len);
    dog_output[dog_len] = '\0';
    printf("  Input 'dog' → Output: '%s' (expected: 'woof')\n", dog_output);
    free(dog_output);
    melvin_m_universal_output_clear(m);
    
    // Test 2: Can learning compound? (Learn pattern A, then pattern B that uses A)
    printf("\n\nTest 2: Compound Learning\n");
    printf("First learn: 'a' → 'b'\n");
    printf("Then learn: 'b' → 'c'\n");
    printf("Question: Can it chain 'a' → 'b' → 'c'?\n\n");
    
    remove("test_compound.m");
    MelvinMFile *m2 = melvin_m_create("test_compound.m");
    
    // Phase 1: Learn a → b
    for (int i = 0; i < 30; i++) {
        melvin_m_universal_input_write(m2, (uint8_t*)"ab", 2);
        melvin_m_process_input(m2);
    }
    
    printf("After learning 'a' → 'b':\n");
    melvin_m_universal_input_write(m2, (uint8_t*)"a", 1);
    melvin_m_process_input(m2);
    size_t ab_len = melvin_m_universal_output_size(m2);
    uint8_t *ab_output = malloc(ab_len + 1);
    melvin_m_universal_output_read(m2, ab_output, ab_len);
    ab_output[ab_len] = '\0';
    printf("  Input 'a' → Output: '%s' (expected: 'b')\n", ab_output);
    free(ab_output);
    melvin_m_universal_output_clear(m2);
    
    // Phase 2: Learn b → c
    for (int i = 0; i < 30; i++) {
        melvin_m_universal_input_write(m2, (uint8_t*)"bc", 2);
        melvin_m_process_input(m2);
    }
    
    printf("\nAfter ALSO learning 'b' → 'c':\n");
    melvin_m_universal_input_write(m2, (uint8_t*)"b", 1);
    melvin_m_process_input(m2);
    size_t bc_len = melvin_m_universal_output_size(m2);
    uint8_t *bc_output = malloc(bc_len + 1);
    melvin_m_universal_output_read(m2, bc_output, bc_len);
    bc_output[bc_len] = '\0';
    printf("  Input 'b' → Output: '%s' (expected: 'c')\n", bc_output);
    free(bc_output);
    melvin_m_universal_output_clear(m2);
    
    // Phase 3: Test chaining
    printf("\nNow test if it can chain 'a' → 'b' → 'c':\n");
    melvin_m_universal_input_write(m2, (uint8_t*)"a", 1);
    melvin_m_process_input(m2);
    size_t chain_len = melvin_m_universal_output_size(m2);
    uint8_t *chain_output = malloc(chain_len + 1);
    melvin_m_universal_output_read(m2, chain_output, chain_len);
    chain_output[chain_len] = '\0';
    printf("  Input 'a' → Output: '%s' (expected: 'bc' or 'b')\n", chain_output);
    free(chain_output);
    melvin_m_universal_output_clear(m2);
    
    // Test 3: Multiple choice - can it learn to pick the right path?
    printf("\n\nTest 3: Multiple Choice Learning\n");
    printf("Training: 'go left' → 'forest', 'go right' → 'cave'\n\n");
    
    remove("test_multichoice.m");
    MelvinMFile *m3 = melvin_m_create("test_multichoice.m");
    
    for (int i = 0; i < 40; i++) {
        // Learn: go left → forest
        melvin_m_universal_input_write(m3, (uint8_t*)"go leftforest", 13);
        melvin_m_process_input(m3);
        
        // Learn: go right → cave
        melvin_m_universal_input_write(m3, (uint8_t*)"go rightcave", 12);
        melvin_m_process_input(m3);
    }
    
    printf("After 40 iterations:\n");
    
    melvin_m_universal_input_write(m3, (uint8_t*)"go left", 7);
    melvin_m_process_input(m3);
    size_t left_len = melvin_m_universal_output_size(m3);
    uint8_t *left_output = malloc(left_len + 1);
    melvin_m_universal_output_read(m3, left_output, left_len);
    left_output[left_len] = '\0';
    printf("  'go left' → '%s' (expected: 'forest')\n", left_output);
    free(left_output);
    melvin_m_universal_output_clear(m3);
    
    melvin_m_universal_input_write(m3, (uint8_t*)"go right", 8);
    melvin_m_process_input(m3);
    size_t right_len = melvin_m_universal_output_size(m3);
    uint8_t *right_output = malloc(right_len + 1);
    melvin_m_universal_output_read(m3, right_output, right_len);
    right_output[right_len] = '\0';
    printf("  'go right' → '%s' (expected: 'cave')\n", right_output);
    free(right_output);
    melvin_m_universal_output_clear(m3);
    
    // Cleanup
    melvin_m_close(m);
    melvin_m_close(m2);
    melvin_m_close(m3);
    
    printf("\n=== Test Complete ===\n");
    printf("\nInterpretation:\n");
    printf("- If outputs match expectations: Melvin CAN make choices like if-statements\n");
    printf("- If chaining works: Learning CAN compound\n");
    printf("- If outputs are wrong: Context gating may need tuning\n");
    
    return 0;
}
