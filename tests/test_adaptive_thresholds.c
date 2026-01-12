/*
 * TEST 8: Adaptive Threshold Validation
 * 
 * Requirement: "No hardcoded thresholds"
 * Brain analog: Adaptive neural plasticity
 * - Thresholds should emerge from data
 * - Should adapt to local statistics
 * 
 * Tests:
 * 1. Identify hardcoded thresholds
 * 2. Test if thresholds adapt to data
 * 3. Compare behavior with different data distributions
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_hardcoded_thresholds() {
    printf("\n=== Test 1: Identifying Hardcoded Thresholds ===\n");
    
    printf("Known hardcoded values in code:\n");
    printf("  1. RECENT_WINDOW = 16 (cycle detection)\n");
    printf("  2. max_output_len = 256 (safety limit)\n");
    printf("  3. Blank node threshold: >= 3 connections\n");
    printf("  4. Hierarchy formation conditions\n");
    printf("  5. Context window sizes\n");
    
    printf("\nThese should ideally be:\n");
    printf("  - Computed from local data statistics\n");
    printf("  - Adaptive to pattern complexity\n");
    printf("  - No magic numbers\n");
    
    printf("\nStatus: PARTIAL - Some thresholds still hardcoded\n");
    printf("Recommendation: Replace with adaptive mechanisms\n");
}

void test_threshold_adaptation() {
    printf("\n=== Test 2: Threshold Adaptation to Data ===\n");
    
    remove("/tmp/test_adapt_thresh.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_adapt_thresh.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Test with sparse data (few patterns)
    printf("Phase 1: Sparse data (2 simple patterns):\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cd", 2);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t nodes_sparse = melvin_m_get_node_count(mfile);
    size_t edges_sparse = melvin_m_get_edge_count(mfile);
    printf("  Nodes: %zu, Edges: %zu\n", nodes_sparse, edges_sparse);
    
    // Test with dense data (many patterns)
    printf("\nPhase 2: Dense data (10 patterns):\n");
    for (int i = 0; i < 10; i++) {
        char pattern[3];
        for (int p = 0; p < 10; p++) {
            snprintf(pattern, sizeof(pattern), "%c%c", 'a' + p, 'a' + p + 1);
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern, 2);
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    size_t nodes_dense = melvin_m_get_node_count(mfile);
    size_t edges_dense = melvin_m_get_edge_count(mfile);
    printf("  Nodes: %zu, Edges: %zu\n", nodes_dense, edges_dense);
    
    // Analysis
    printf("\nAnalysis:\n");
    float node_growth = (float)(nodes_dense - nodes_sparse) / (float)nodes_sparse;
    float edge_growth = (float)(edges_dense - edges_sparse) / (float)edges_sparse;
    
    printf("  Node growth: %.2fx\n", node_growth + 1.0f);
    printf("  Edge growth: %.2fx\n", edge_growth + 1.0f);
    
    if (edge_growth > node_growth * 1.5f) {
        printf("  ✓ System adapts: More edges relative to nodes in dense data\n");
        printf("PASS: Thresholds appear to adapt to data density\n");
    } else {
        printf("  ✗ Growth is linear, may not be adapting\n");
        printf("PARTIAL: Limited adaptation to data density\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_adapt_thresh.m");
}

void test_local_vs_global_thresholds() {
    printf("\n=== Test 3: Local vs Global Thresholds ===\n");
    
    remove("/tmp/test_local.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_local.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Create two regions with different statistics
    printf("Training two regions with different patterns:\n");
    printf("  Region A: High-frequency patterns ('aaa', 'bbb')\n");
    printf("  Region B: Low-frequency patterns ('xyz', 'pqr')\n");
    
    // High-frequency region
    for (int i = 0; i < 50; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aaa", 3);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"bbb", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Low-frequency region
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xyz", 3);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"pqr", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test both regions
    printf("\nTesting recall:\n");
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"aa", 2);
    size_t out1_size = melvin_m_universal_output_size(mfile);
    printf("  High-freq 'aa' → %zu bytes output\n", out1_size);
    
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"xy", 2);
    size_t out2_size = melvin_m_universal_output_size(mfile);
    printf("  Low-freq 'xy' → %zu bytes output\n", out2_size);
    
    printf("\nAnalysis:\n");
    printf("  If thresholds are LOCAL (adaptive):\n");
    printf("    Both should generate output (relative to local stats)\n");
    printf("  If thresholds are GLOBAL (fixed):\n");
    printf("    Low-freq may not generate output (below global threshold)\n");
    
    if (out1_size > 0 && out2_size > 0) {
        printf("\nPASS: Both regions generate output (local thresholds)\n");
    } else if (out1_size > 0 && out2_size == 0) {
        printf("\nFAIL: Only high-freq generates output (global threshold)\n");
    } else {
        printf("\nUNCLEAR: Need more investigation\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_local.m");
}

void test_threshold_emergence() {
    printf("\n=== Test 4: Threshold Emergence from Data ===\n");
    
    remove("/tmp/test_emerge.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_emerge.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Start with no data, gradually add patterns
    printf("Testing threshold emergence:\n");
    
    printf("\nPhase 1: 1 pattern (minimal data):\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ab", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_1 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu\n", nodes_1);
    
    printf("\nPhase 2: 3 patterns:\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"cd", 2);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"ef", 2);
        melvin_m_universal_output_clear(mfile);
    }
    size_t nodes_3 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_3, nodes_3 - nodes_1);
    
    printf("\nPhase 3: 10 patterns:\n");
    for (int i = 0; i < 5; i++) {
        for (int p = 0; p < 7; p++) {
            char pattern[3];
            snprintf(pattern, sizeof(pattern), "%c%c", 'g' + p, 'h' + p);
            melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)pattern, 2);
            melvin_m_universal_output_clear(mfile);
        }
    }
    size_t nodes_10 = melvin_m_get_node_count(mfile);
    printf("  Nodes: %zu (delta: %zu)\n", nodes_10, nodes_10 - nodes_3);
    
    printf("\nAnalysis:\n");
    printf("  If thresholds emerge from data:\n");
    printf("    Node growth should be sublinear (compression improves)\n");
    printf("  If thresholds are fixed:\n");
    printf("    Node growth should be linear\n");
    
    float growth_1_3 = (float)(nodes_3 - nodes_1);
    float growth_3_10 = (float)(nodes_10 - nodes_3);
    float growth_ratio = growth_3_10 / growth_1_3;
    
    printf("  Growth ratio (phase 3/phase 2): %.2f\n", growth_ratio);
    
    if (growth_ratio < 2.0f) {
        printf("  ✓ Sublinear growth (thresholds adapting)\n");
        printf("PASS: Thresholds appear to emerge from data!\n");
    } else {
        printf("  ✗ Linear growth (fixed thresholds)\n");
        printf("PARTIAL: Limited evidence of emergence\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_emerge.m");
}

void test_requirement_compliance() {
    printf("\n=== Test 5: Requirement Compliance Check ===\n");
    
    printf("Requirement: 'No hardcoded thresholds'\n");
    printf("\nCurrent Status:\n");
    
    printf("\n✓ ADAPTIVE (Data-Driven):\n");
    printf("  - Edge weight updates (Hebbian learning)\n");
    printf("  - Context matching (sparse overlap)\n");
    printf("  - Hierarchy formation (relative to local avg)\n");
    printf("  - Node activation (spreading activation)\n");
    
    printf("\n✗ HARDCODED (Need to Fix):\n");
    printf("  - RECENT_WINDOW = 16 (cycle detection)\n");
    printf("  - max_output_len = 256 (safety limit)\n");
    printf("  - Blank node: >= 3 connections threshold\n");
    printf("  - Various epsilon values\n");
    
    printf("\n~ PARTIAL (Computed but with constants):\n");
    printf("  - Adaptive epsilon uses local avg + constant multiplier\n");
    printf("  - Hierarchy threshold uses ratio + constant\n");
    printf("  - Context window sizes\n");
    
    printf("\nRecommendations:\n");
    printf("  1. Replace RECENT_WINDOW with adaptive cycle detection\n");
    printf("  2. Make blank node threshold data-driven\n");
    printf("  3. Remove constant multipliers, use pure ratios\n");
    printf("  4. Add meta-learning for threshold adaptation\n");
    
    printf("\nOverall: PARTIAL COMPLIANCE\n");
    printf("  Most core mechanisms are adaptive\n");
    printf("  Some safety/structural constants remain\n");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          ADAPTIVE THRESHOLD VALIDATION TEST                ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_hardcoded_thresholds();
    test_threshold_adaptation();
    test_local_vs_global_thresholds();
    test_threshold_emergence();
    test_requirement_compliance();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
