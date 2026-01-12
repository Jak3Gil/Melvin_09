/*
 * TEST 6: Context Window Effectiveness
 * 
 * Brain analog: Working memory capacity
 * - How far back does context influence decisions?
 * - Does older context fade appropriately?
 * 
 * Tests:
 * 1. Short-range context (2-5 nodes back)
 * 2. Medium-range context (10-20 nodes back)
 * 3. Long-range context (50+ nodes back)
 * 4. Context decay over distance
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_short_range_context() {
    printf("\n=== Test 1: Short-Range Context (2-5 nodes) ===\n");
    
    remove("/tmp/test_short_ctx.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_short_ctx.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train patterns where last 2-3 chars determine outcome
    printf("Training context-dependent patterns:\n");
    printf("  'aax' → 'p'\n");
    printf("  'aay' → 'q'\n");
    printf("  'aaz' → 'r'\n");
    
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aaxp", 4);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aayq", 4);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aazr", 4);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: Does context of last 2 chars ('ax', 'ay', 'az') determine output?
    printf("\nTesting:\n");
    
    struct {
        const char *input;
        char expected;
    } tests[] = {
        {"aax", 'p'},
        {"aay", 'q'},
        {"aaz", 'r'}
    };
    
    int correct = 0;
    for (int t = 0; t < 3; t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)tests[t].input, 3);
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            int has_expected = 0;
            for (size_t i = 0; i < out_size; i++) {
                if (output[i] == tests[t].expected) {
                    has_expected = 1;
                    break;
                }
            }
            
            printf("  '%s' → '%c' (expected '%c': %s)\n",
                   tests[t].input, out_size > 0 ? output[0] : '?',
                   tests[t].expected, has_expected ? "YES ✓" : "NO ✗");
            
            if (has_expected) correct++;
            free(output);
        }
    }
    
    printf("\nShort-range context: %d/3 correct\n", correct);
    if (correct >= 2) {
        printf("PASS: Short-range context working!\n");
    } else {
        printf("FAIL: Short-range context not effective\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_short_ctx.m");
}

void test_medium_range_context() {
    printf("\n=== Test 2: Medium-Range Context (10-20 nodes) ===\n");
    
    remove("/tmp/test_med_ctx.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_med_ctx.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train patterns where context 10+ chars back matters
    printf("Training patterns with distant context:\n");
    printf("  'morning...hello' → 'good'\n");
    printf("  'evening...hello' → 'nice'\n");
    
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"morning hello good", 18);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"evening hello nice", 18);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: Does "morning" vs "evening" (7 chars back) influence output?
    printf("\nTesting:\n");
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"morning hello", 13);
    size_t out1_size = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = NULL;
    if (out1_size > 0) {
        out1 = malloc(out1_size + 1);
        melvin_m_universal_output_read(mfile, out1, out1_size);
        out1[out1_size] = '\0';
        printf("  'morning hello' → '%.*s'\n", (int)out1_size, out1);
    }
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"evening hello", 13);
    size_t out2_size = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = NULL;
    if (out2_size > 0) {
        out2 = malloc(out2_size + 1);
        melvin_m_universal_output_read(mfile, out2, out2_size);
        out2[out2_size] = '\0';
        printf("  'evening hello' → '%.*s'\n", (int)out2_size, out2);
    }
    
    // Check if outputs are different (context-dependent)
    int different = 0;
    if (out1 && out2) {
        different = (out1_size != out2_size || memcmp(out1, out2, out1_size) != 0);
    }
    
    printf("\nOutputs different: %s\n", different ? "YES ✓" : "NO ✗");
    
    if (different) {
        printf("PASS: Medium-range context influences output!\n");
    } else {
        printf("FAIL: Context not effective at medium range\n");
    }
    
    if (out1) free(out1);
    if (out2) free(out2);
    melvin_m_close(mfile);
    remove("/tmp/test_med_ctx.m");
}

void test_long_range_context() {
    printf("\n=== Test 3: Long-Range Context (50+ nodes) ===\n");
    
    remove("/tmp/test_long_ctx.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_long_ctx.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train patterns with very distant context
    const char *pattern1 = "CONTEXT_A followed by many words and then RESULT_X";
    const char *pattern2 = "CONTEXT_B followed by many words and then RESULT_Y";
    
    printf("Training patterns with 50+ char separation:\n");
    printf("  Pattern 1: 'CONTEXT_A ... RESULT_X'\n");
    printf("  Pattern 2: 'CONTEXT_B ... RESULT_Y'\n");
    
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern1, strlen(pattern1));
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern2, strlen(pattern2));
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: Does CONTEXT_A vs CONTEXT_B influence RESULT?
    printf("\nTesting:\n");
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, 
                                 (uint8_t*)"CONTEXT_A followed by many words and then RESULT", 
                                 49);
    size_t out1_size = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = NULL;
    if (out1_size > 0) {
        out1 = malloc(out1_size + 1);
        melvin_m_universal_output_read(mfile, out1, out1_size);
        out1[out1_size] = '\0';
        
        int has_x = 0;
        for (size_t i = 0; i < out1_size; i++) {
            if (out1[i] == 'X') has_x = 1;
        }
        printf("  CONTEXT_A → contains 'X': %s\n", has_x ? "YES ✓" : "NO ✗");
    }
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0,
                                 (uint8_t*)"CONTEXT_B followed by many words and then RESULT",
                                 49);
    size_t out2_size = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = NULL;
    if (out2_size > 0) {
        out2 = malloc(out2_size + 1);
        melvin_m_universal_output_read(mfile, out2, out2_size);
        out2[out2_size] = '\0';
        
        int has_y = 0;
        for (size_t i = 0; i < out2_size; i++) {
            if (out2[i] == 'Y') has_y = 1;
        }
        printf("  CONTEXT_B → contains 'Y': %s\n", has_y ? "YES ✓" : "NO ✗");
    }
    
    printf("\nConclusion:\n");
    printf("  Long-range context (50+ chars) is challenging\n");
    printf("  Brain: Working memory ~7±2 items\n");
    printf("  Melvin: SparseContext accumulates all nodes\n");
    printf("  May need attention mechanism or context decay\n");
    
    if (out1) free(out1);
    if (out2) free(out2);
    melvin_m_close(mfile);
    remove("/tmp/test_long_ctx.m");
}

void test_context_decay() {
    printf("\n=== Test 4: Context Decay Over Distance ===\n");
    
    remove("/tmp/test_decay.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_decay.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train pattern where recent context should dominate
    printf("Training: 'xyz abc def'\n");
    printf("  Recent context 'def' should dominate over distant 'xyz'\n");
    
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xyz abc def ghi", 15);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: Input "xyz abc def", expect "ghi" (not influenced by distant "xyz")
    printf("\nTesting context priority:\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xyz abc def", 11);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        printf("  Output: '%.*s'\n", (int)out_size, output);
        
        // Check if output contains 'g' (from "ghi")
        int has_g = 0;
        for (size_t i = 0; i < out_size; i++) {
            if (output[i] == 'g') has_g = 1;
        }
        
        printf("  Contains 'g' (correct continuation): %s\n", has_g ? "YES ✓" : "NO");
        
        if (has_g) {
            printf("PASS: Recent context dominates (implicit decay working)\n");
        } else {
            printf("PARTIAL: May need explicit context decay mechanism\n");
        }
        
        free(output);
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_decay.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           CONTEXT WINDOW EFFECTIVENESS TEST                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_short_range_context();
    test_medium_range_context();
    test_long_range_context();
    test_context_decay();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
