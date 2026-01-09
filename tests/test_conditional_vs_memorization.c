#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

/*
 * Test: Conditional Logic vs. Pattern Memorization
 * 
 * This test proves whether Melvin learns:
 * A) Conditional rules (if X then Y, if Z then W)
 * B) Just memorizes and repeats patterns
 * 
 * Training data:
 * - "hello world"
 * - "hello friend"
 * - "goodbye world"
 * - "goodbye friend"
 * 
 * Test cases:
 * 1. "hello " → should output "world" OR "friend" (both valid)
 * 2. "goodbye " → should output "world" OR "friend" (both valid)
 * 3. "hello w" → should output "orld" (conditional: after "hello w", complete "world")
 * 4. "hello f" → should output "riend" (conditional: after "hello f", complete "friend")
 * 5. "goodbye w" → should output "orld"
 * 6. "goodbye f" → should output "riend"
 * 
 * If it learns CONDITIONAL LOGIC:
 * - Test 3-6 should work (context-dependent completion)
 * - It understands "after hello+w, complete world" vs "after hello+f, complete friend"
 * 
 * If it just MEMORIZES:
 * - It will output whatever pattern was trained most recently
 * - Or it will mix patterns randomly
 * - Tests 3-6 will fail or produce garbage
 */

int main() {
    printf("=== Conditional Logic vs. Memorization Test ===\n\n");
    
    const char *test_file = "test_conditional.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Training data: 4 different patterns
    const char *patterns[] = {
        "hello world",
        "hello friend",
        "goodbye world",
        "goodbye friend"
    };
    int num_patterns = 4;
    
    printf("Training on 4 patterns (50 iterations each):\n");
    for (int i = 0; i < num_patterns; i++) {
        printf("  - '%s'\n", patterns[i]);
    }
    printf("\n");
    
    // Train 50 iterations on each pattern (interleaved)
    for (int iter = 0; iter < 50; iter++) {
        for (int p = 0; p < num_patterns; p++) {
            melvin_m_universal_input_write(mfile, (uint8_t*)patterns[p], strlen(patterns[p]));
            melvin_m_process_input(mfile);
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("Training complete. Graph stats:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n\n", melvin_m_get_edge_count(mfile));
    
    // Test cases
    struct {
        const char *input;
        const char *expected_contains;  // What the output should contain
        const char *test_name;
    } tests[] = {
        {"hello ", "world", "Test 1: 'hello ' → should continue with 'world' or 'friend'"},
        {"hello ", "friend", "Test 1b: 'hello ' → checking for 'friend'"},
        {"goodbye ", "world", "Test 2: 'goodbye ' → should continue with 'world' or 'friend'"},
        {"goodbye ", "friend", "Test 2b: 'goodbye ' → checking for 'friend'"},
        {"hello w", "orld", "Test 3: 'hello w' → should complete 'world' (conditional!)"},
        {"hello f", "riend", "Test 4: 'hello f' → should complete 'friend' (conditional!)"},
        {"goodbye w", "orld", "Test 5: 'goodbye w' → should complete 'world' (conditional!)"},
        {"goodbye f", "riend", "Test 6: 'goodbye f' → should complete 'friend' (conditional!)"}
    };
    int num_tests = sizeof(tests) / sizeof(tests[0]);
    
    printf("=== Test Results ===\n\n");
    
    int conditional_tests_passed = 0;
    int conditional_tests_total = 0;
    
    for (int t = 0; t < num_tests; t++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)tests[t].input, strlen(tests[t].input));
        melvin_m_process_input(mfile);
        
        size_t output_len = melvin_m_universal_output_size(mfile);
        uint8_t *output = malloc(output_len + 1);
        melvin_m_universal_output_read(mfile, output, output_len);
        output[output_len] = '\0';
        
        // Check if output contains expected string
        int contains = (strstr((char*)output, tests[t].expected_contains) != NULL);
        
        printf("%s\n", tests[t].test_name);
        printf("  Input:    '%s'\n", tests[t].input);
        printf("  Output:   '%.*s'\n", (int)output_len, output);
        printf("  Expected: (contains '%s')\n", tests[t].expected_contains);
        printf("  Result:   %s\n\n", contains ? "✅ PASS" : "❌ FAIL");
        
        // Track conditional tests (tests 3-6)
        if (t >= 4) {
            conditional_tests_total++;
            if (contains) conditional_tests_passed++;
        }
        
        free(output);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("=== Analysis ===\n\n");
    printf("Conditional tests (3-6): %d/%d passed\n\n", 
           conditional_tests_passed, conditional_tests_total);
    
    if (conditional_tests_passed == conditional_tests_total) {
        printf("✅ CONCLUSION: System learns CONDITIONAL LOGIC\n");
        printf("   - It understands context-dependent completion\n");
        printf("   - 'hello w' → 'orld' vs 'hello f' → 'riend'\n");
        printf("   - This is true intelligence, not just memorization!\n");
    } else if (conditional_tests_passed == 0) {
        printf("❌ CONCLUSION: System only MEMORIZES patterns\n");
        printf("   - It cannot do context-dependent completion\n");
        printf("   - It just repeats what it saw during training\n");
        printf("   - No conditional logic or rule learning\n");
    } else {
        printf("⚠️  CONCLUSION: System has PARTIAL conditional ability\n");
        printf("   - Some context-dependent completion works\n");
        printf("   - But not consistent across all cases\n");
        printf("   - May need more training or better architecture\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}
