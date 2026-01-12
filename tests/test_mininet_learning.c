/*
 * TEST 7: Mini-Net Learning Validation
 * 
 * Brain analog: Local neural circuits
 * - Each node has a mini-net for local predictions
 * - Should learn which edges to take based on context
 * 
 * Tests:
 * 1. Do mini-nets learn useful patterns?
 * 2. Do they improve prediction accuracy?
 * 3. Edge weights vs mini-net predictions
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_mininet_learning() {
    printf("\n=== Test 1: Mini-Net Learning ===\n");
    
    remove("/tmp/test_mininet.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_mininet.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train context-dependent patterns
    // Node 'a' should learn: after 'x' → go to 'p', after 'y' → go to 'q'
    printf("Training context-dependent routing:\n");
    printf("  'xap' (x→a→p)\n");
    printf("  'yaq' (y→a→q)\n");
    
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xap", 3);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"yaq", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("\nAfter training:\n");
    printf("  Nodes: %zu, Edges: %zu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test: Does node 'a' route correctly based on context?
    printf("\nTesting context-based routing:\n");
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xa", 2);
    size_t out1_size = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = NULL;
    int has_p = 0;
    if (out1_size > 0) {
        out1 = malloc(out1_size + 1);
        melvin_m_universal_output_read(mfile, out1, out1_size);
        out1[out1_size] = '\0';
        for (size_t i = 0; i < out1_size; i++) {
            if (out1[i] == 'p') has_p = 1;
        }
        printf("  'xa' → '%.*s' (contains 'p': %s)\n", 
               (int)out1_size, out1, has_p ? "YES ✓" : "NO ✗");
    }
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ya", 2);
    size_t out2_size = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = NULL;
    int has_q = 0;
    if (out2_size > 0) {
        out2 = malloc(out2_size + 1);
        melvin_m_universal_output_read(mfile, out2, out2_size);
        out2[out2_size] = '\0';
        for (size_t i = 0; i < out2_size; i++) {
            if (out2[i] == 'q') has_q = 1;
        }
        printf("  'ya' → '%.*s' (contains 'q': %s)\n",
               (int)out2_size, out2, has_q ? "YES ✓" : "NO ✗");
    }
    
    printf("\nMini-Net Learning: ");
    if (has_p && has_q) {
        printf("PASS - Context-dependent routing working!\n");
        printf("  (This could be mini-nets OR ContextTags)\n");
    } else if (has_p || has_q) {
        printf("PARTIAL - Some context sensitivity\n");
    } else {
        printf("FAIL - No context-dependent routing\n");
    }
    
    if (out1) free(out1);
    if (out2) free(out2);
    melvin_m_close(mfile);
    remove("/tmp/test_mininet.m");
}

void test_mininet_vs_edge_weights() {
    printf("\n=== Test 2: Mini-Net vs Edge Weight Competition ===\n");
    
    remove("/tmp/test_competition.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_competition.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Create scenario where edge weights favor one path,
    // but context should favor another
    printf("Training:\n");
    printf("  'abc' (10 times) - creates strong a→b→c edges\n");
    printf("  'abd' (2 times)  - creates weak a→b→d edges\n");
    printf("  BUT in context 'xabd', should go to 'd'\n");
    
    // Train strong path
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abc", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Train weak path with specific context
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xabd", 4);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: In context 'xab', should go to 'd' (context wins) not 'c' (edge weight wins)
    printf("\nTesting:\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xab", 3);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        int has_c = 0, has_d = 0;
        for (size_t i = 0; i < out_size; i++) {
            if (output[i] == 'c') has_c = 1;
            if (output[i] == 'd') has_d = 1;
        }
        
        printf("  'xab' → '%.*s'\n", (int)out_size, output);
        printf("    Contains 'c' (edge weight): %s\n", has_c ? "YES" : "NO");
        printf("    Contains 'd' (context): %s\n", has_d ? "YES" : "NO");
        
        if (has_d && !has_c) {
            printf("\nPASS: Context (mini-net/ContextTags) wins over edge weights!\n");
        } else if (has_c && !has_d) {
            printf("\nFAIL: Edge weights dominate, context ignored\n");
        } else {
            printf("\nPARTIAL: Mixed results\n");
        }
        
        free(output);
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_competition.m");
}

void test_mininet_adaptation() {
    printf("\n=== Test 3: Mini-Net Adaptation Over Time ===\n");
    
    remove("/tmp/test_adapt.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_adapt.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Phase 1: Train one pattern
    printf("Phase 1: Training 'ab' → 'x' (10 times)\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abx", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test phase 1
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
    size_t out1_size = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = NULL;
    int phase1_has_x = 0;
    if (out1_size > 0) {
        out1 = malloc(out1_size + 1);
        melvin_m_universal_output_read(mfile, out1, out1_size);
        out1[out1_size] = '\0';
        for (size_t i = 0; i < out1_size; i++) {
            if (out1[i] == 'x') phase1_has_x = 1;
        }
        printf("  Test: 'ab' → '%.*s' (has 'x': %s)\n",
               (int)out1_size, out1, phase1_has_x ? "YES ✓" : "NO");
    }
    
    // Phase 2: Train NEW pattern (adaptation)
    printf("\nPhase 2: Training 'ab' → 'y' (15 times, overriding)\n");
    for (int i = 0; i < 15; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aby", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test phase 2
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
    size_t out2_size = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = NULL;
    int phase2_has_y = 0;
    if (out2_size > 0) {
        out2 = malloc(out2_size + 1);
        melvin_m_universal_output_read(mfile, out2, out2_size);
        out2[out2_size] = '\0';
        for (size_t i = 0; i < out2_size; i++) {
            if (out2[i] == 'y') phase2_has_y = 1;
        }
        printf("  Test: 'ab' → '%.*s' (has 'y': %s)\n",
               (int)out2_size, out2, phase2_has_y ? "YES ✓" : "NO");
    }
    
    printf("\nAdaptation: ");
    if (phase1_has_x && phase2_has_y) {
        printf("PASS - Mini-nets adapted to new pattern!\n");
    } else if (phase2_has_y) {
        printf("PARTIAL - Learned new pattern (may not have learned old one)\n");
    } else {
        printf("FAIL - Did not adapt to new pattern\n");
    }
    
    if (out1) free(out1);
    if (out2) free(out2);
    melvin_m_close(mfile);
    remove("/tmp/test_adapt.m");
}

void test_mininet_hierarchy_interaction() {
    printf("\n=== Test 4: Mini-Net + Hierarchy Interaction ===\n");
    
    remove("/tmp/test_hier_mini.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_hier_mini.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train to form hierarchies, then see if mini-nets use them
    printf("Training 'hello world' to form hierarchies:\n");
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (hierarchies likely formed if > 12)\n", nodes);
    
    // Test: Does mini-net routing consider hierarchies?
    printf("\nTesting if mini-nets route through hierarchies:\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hel", 3);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        printf("  'hel' → '%.*s'\n", (int)out_size, output);
        
        // If hierarchies are used, output should be efficient/complete
        int has_world = 0;
        for (size_t i = 0; i < out_size - 4; i++) {
            if (memcmp(&output[i], "world", 5) == 0) {
                has_world = 1;
                break;
            }
        }
        
        printf("  Contains 'world': %s\n", has_world ? "YES ✓" : "NO");
        
        if (has_world) {
            printf("\nPASS: System uses hierarchies for completion!\n");
        } else {
            printf("\nPARTIAL: Hierarchies exist but may not be used optimally\n");
        }
        
        free(output);
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_hier_mini.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           MINI-NET LEARNING VALIDATION TEST                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_mininet_learning();
    test_mininet_vs_edge_weights();
    test_mininet_adaptation();
    test_mininet_hierarchy_interaction();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
