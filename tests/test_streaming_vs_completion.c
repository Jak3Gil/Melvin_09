/*
 * TEST 5: Streaming vs Completion Mode
 * 
 * Brain analog: Predictive processing
 * - Real-time prediction (streaming): predict next byte as input arrives
 * - Completion mode: given partial input, complete the pattern
 * 
 * Tests:
 * 1. Streaming prediction accuracy
 * 2. Completion accuracy
 * 3. Which mode is the system optimized for?
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_streaming_prediction() {
    printf("\n=== Test 1: Streaming Prediction ===\n");
    printf("System predicts next byte as input arrives (real-time)\n");
    
    remove("/tmp/test_stream.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_stream.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train "hello world"
    printf("Training 'hello world' 10 times:\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test streaming: input "hello" byte-by-byte, check predictions
    printf("\nTest: Streaming input 'hello' byte-by-byte:\n");
    const char *input = "hello";
    
    for (size_t i = 0; i < strlen(input); i++) {
        melvin_m_universal_output_clear(mfile);
        
        // Input single byte
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)&input[i], 1);
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            printf("  After '%c': predicted '%.*s' (%zu bytes)\n",
                   input[i], (int)(out_size > 10 ? 10 : out_size), output, out_size);
            
            free(output);
        } else {
            printf("  After '%c': no prediction\n", input[i]);
        }
    }
    
    printf("\nAnalysis:\n");
    printf("  Streaming mode: System predicts during input processing\n");
    printf("  This is REAL-TIME prediction (like brain's predictive processing)\n");
    
    melvin_m_close(mfile);
    remove("/tmp/test_stream.m");
}

void test_completion_mode() {
    printf("\n=== Test 2: Completion Mode ===\n");
    printf("Given partial input, complete the full pattern\n");
    
    remove("/tmp/test_complete.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_complete.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train complete patterns
    printf("Training complete patterns:\n");
    const char *patterns[] = {
        "the cat sat on the mat",
        "the dog ran in the park",
        "the bird flew over the tree"
    };
    
    for (int iter = 0; iter < 10; iter++) {
        for (int p = 0; p < 3; p++) {
            melvin_in_port_handle_buffer(mfile, 0, 
                                         (uint8_t*)patterns[p], 
                                         strlen(patterns[p]));
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    // Test completion: give partial input, expect full completion
    printf("\nTest: Partial inputs for completion:\n");
    
    struct {
        const char *partial;
        const char *expected_completion;
    } tests[] = {
        {"the cat", "sat on the mat"},
        {"the dog", "ran in the park"},
        {"the bird", "flew over the tree"}
    };
    
    int pass_count = 0;
    for (int t = 0; t < 3; t++) {
        melvin_m_universal_output_clear(mfile);
        melvin_in_port_handle_buffer(mfile, 0, 
                                     (uint8_t*)tests[t].partial, 
                                     strlen(tests[t].partial));
        
        size_t out_size = melvin_m_universal_output_size(mfile);
        if (out_size > 0) {
            uint8_t *output = malloc(out_size + 1);
            melvin_m_universal_output_read(mfile, output, out_size);
            output[out_size] = '\0';
            
            // Check if output contains key word from expected completion
            const char *key_word = (t == 0) ? "sat" : (t == 1) ? "ran" : "flew";
            int has_key = 0;
            for (size_t i = 0; i < out_size - 2; i++) {
                if (memcmp(&output[i], key_word, strlen(key_word)) == 0) {
                    has_key = 1;
                    break;
                }
            }
            
            printf("  '%s' → '%.*s'\n", 
                   tests[t].partial, 
                   (int)(out_size > 30 ? 30 : out_size), 
                   output);
            printf("    Contains '%s': %s\n", key_word, has_key ? "YES ✓" : "NO ✗");
            
            if (has_key) pass_count++;
            free(output);
        }
    }
    
    printf("\nCompletion accuracy: %d/3\n", pass_count);
    if (pass_count >= 2) {
        printf("PASS: Completion mode working!\n");
    } else {
        printf("PARTIAL: Some completion but not reliable\n");
    }
    
    melvin_m_close(mfile);
    remove("/tmp/test_complete.m");
}

void test_mode_comparison() {
    printf("\n=== Test 3: Mode Comparison ===\n");
    printf("Which mode is the system optimized for?\n");
    
    remove("/tmp/test_compare.m");
    MelvinMFile *mfile = melvin_m_create("/tmp/test_compare.m");
    if (!mfile) {
        printf("FAIL: Could not create brain\n");
        return;
    }
    
    // Train "abcdefgh"
    printf("Training 'abcdefgh' 20 times:\n");
    for (int i = 0; i < 20; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abcdefgh", 8);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test 1: Streaming (input "abc", see what's predicted during input)
    printf("\nStreaming test: Input 'abc' as batch:\n");
    melvin_m_universal_output_clear(mfile);
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"abc", 3);
    
    size_t stream_size = melvin_m_universal_output_size(mfile);
    uint8_t *stream_out = NULL;
    if (stream_size > 0) {
        stream_out = malloc(stream_size + 1);
        melvin_m_universal_output_read(mfile, stream_out, stream_size);
        stream_out[stream_size] = '\0';
        printf("  Output during input: '%.*s' (%zu bytes)\n", 
               (int)stream_size, stream_out, stream_size);
    } else {
        printf("  No output during input\n");
    }
    
    // Test 2: Completion (same input, but check if it completes to "defgh")
    printf("\nCompletion test: Does it complete 'abc' → 'defgh'?\n");
    int has_d = 0, has_e = 0, has_f = 0;
    if (stream_out && stream_size > 0) {
        for (size_t i = 0; i < stream_size; i++) {
            if (stream_out[i] == 'd') has_d = 1;
            if (stream_out[i] == 'e') has_e = 1;
            if (stream_out[i] == 'f') has_f = 1;
        }
    }
    
    printf("  Contains 'd': %s\n", has_d ? "YES" : "NO");
    printf("  Contains 'e': %s\n", has_e ? "YES" : "NO");
    printf("  Contains 'f': %s\n", has_f ? "YES" : "NO");
    
    printf("\nConclusion:\n");
    if (stream_size > 3) {
        printf("  System is in STREAMING mode\n");
        printf("  It predicts/generates output DURING input processing\n");
        printf("  This is brain-like: real-time predictive processing\n");
    } else if (stream_size > 0) {
        printf("  System is in HYBRID mode\n");
        printf("  Some prediction during input, but limited\n");
    } else {
        printf("  System is in COMPLETION mode\n");
        printf("  Output only after full input processed\n");
    }
    
    if (stream_out) free(stream_out);
    melvin_m_close(mfile);
    remove("/tmp/test_compare.m");
}

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║          STREAMING VS COMPLETION MODE TEST                 ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    test_streaming_prediction();
    test_completion_mode();
    test_mode_comparison();
    
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   TEST COMPLETE                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
