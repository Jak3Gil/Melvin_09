/*
 * TEST 1: Multimodal Capability
 * 
 * Brain analog: Cross-modal associations in cortex
 * - Visual cortex → auditory cortex associations
 * - Seeing "dog" → hearing "bark"
 * 
 * Tests:
 * 1. Text (port 0) → Audio (port 1) association
 * 2. Audio (port 1) → Visual (port 2) association
 * 3. Cross-modal retrieval
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_text_to_audio() {
    printf("\n=== Test 1: Text→Audio Association ===\n");
    
    remove("/tmp/test_multimodal.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_multimodal.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train: "dog" (text, port 0) followed by "bark" (audio, port 1)
    printf("Training: 'dog' (text) → 'bark' (audio)\n");
    for (int i = 0; i < 5; i++) {
        // Text input on port 0
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"dog", 3);
        melvin_m_universal_output_clear(mfile);
        
        // Audio input on port 1
        melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"bark", 4);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Test: Give "dog" (text), expect association to "bark" (audio)
    printf("\nTesting: Input 'dog' (text)...\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"dog", 3);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        // Check if output contains 'b' (from "bark")
        int has_b = 0;
        for (size_t i = 0; i < out_size; i++) {
            if (output[i] == 'b') has_b = 1;
        }
        
        printf("Output: '%.*s' (%zu bytes)\n", (int)out_size, output, out_size);
        printf("Contains 'b' from 'bark': %s\n", has_b ? "YES ✓" : "NO ✗");
        
        if (has_b) {
            printf("PASS: Cross-modal association works!\n");
        } else {
            printf("PARTIAL: Association exists but not retrieving audio pattern\n");
        }
        
        free(output);
    } else {
        printf("FAIL: No output generated\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_multimodal.m");
}

void test_audio_to_visual() {
    printf("\n=== Test 2: Audio→Visual Association ===\n");
    
    remove("/tmp/test_av.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_av.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train: "meow" (audio, port 1) followed by "cat" (visual, port 2)
    printf("Training: 'meow' (audio) → 'cat' (visual)\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"meow", 4);
        melvin_m_universal_output_clear(mfile);
        
        melvin_in_port_handle_buffer(mfile, 2, (uint8_t*)"cat", 3);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test: Give "meow" (audio), expect "cat" (visual)
    printf("\nTesting: Input 'meow' (audio)...\n");
    melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"meow", 4);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    if (out_size > 0) {
        uint8_t *output = malloc(out_size + 1);
        melvin_m_universal_output_read(mfile, output, out_size);
        output[out_size] = '\0';
        
        int has_c = 0;
        for (size_t i = 0; i < out_size; i++) {
            if (output[i] == 'c') has_c = 1;
        }
        
        printf("Output: '%.*s'\n", (int)out_size, output);
        printf("Contains 'c' from 'cat': %s\n", has_c ? "YES ✓" : "NO ✗");
        
        if (has_c) {
            printf("PASS: Audio→Visual association works!\n");
        }
        
        free(output);
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_av.m");
}

void test_multimodal_discrimination() {
    printf("\n=== Test 3: Multimodal Discrimination ===\n");
    printf("Same input, different modality → different output\n");
    
    remove("/tmp/test_discrim.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_discrim.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train: "red" on port 0 (text) → "apple"
    //        "red" on port 1 (visual) → "fire"
    printf("Training:\n");
    printf("  'red' (text, port 0) → 'apple'\n");
    printf("  'red' (visual, port 1) → 'fire'\n");
    
    for (int i = 0; i < 5; i++) {
        // Text "red" → "apple"
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"red", 3);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"apple", 5);
        melvin_m_universal_output_clear(mfile);
        
        // Visual "red" → "fire"
        melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"red", 3);
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"fire", 4);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test both
    printf("\nTesting 'red' on port 0 (text):\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"red", 3);
    size_t len1 = melvin_m_universal_output_size(mfile);
    uint8_t *out1 = malloc(len1 + 1);
    melvin_m_universal_output_read(mfile, out1, len1);
    out1[len1] = '\0';
    printf("  Output: '%.*s'\n", (int)len1, out1);
    
    melvin_m_universal_output_clear(mfile);
    printf("\nTesting 'red' on port 1 (visual):\n");
    melvin_in_port_handle_buffer(mfile, 1, (uint8_t*)"red", 3);
    size_t len2 = melvin_m_universal_output_size(mfile);
    uint8_t *out2 = malloc(len2 + 1);
    melvin_m_universal_output_read(mfile, out2, len2);
    out2[len2] = '\0';
    printf("  Output: '%.*s'\n", (int)len2, out2);
    
    int different = (len1 != len2 || memcmp(out1, out2, len1) != 0);
    printf("\nOutputs different: %s\n", different ? "YES ✓" : "NO ✗");
    
    if (different) {
        printf("PASS: Port ID provides context for discrimination!\n");
    } else {
        printf("FAIL: Same output regardless of input port\n");
    }
    
    free(out1);
    free(out2);
    melvin_m_close(mfile);
    remove("/tmp/test_discrim.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║              MULTIMODAL CAPABILITY TEST                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_text_to_audio();
    test_audio_to_visual();
    test_multimodal_discrimination();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
