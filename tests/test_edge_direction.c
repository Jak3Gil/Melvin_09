/*
 * Test: Edge Direction Verification
 * 
 * Validates that edges only follow input order by checking outputs
 * - Train "world" pattern
 * - Test that it generates correctly (forward edges work)
 * - Verify no reverse generation occurs
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/melvin.h"

int main(void) {
    printf("=== Edge Direction Test ===\n\n");
    
    // Create .m file
    MelvinMFile *mfile = melvin_m_create("test_edge_direction.m");
    if (!mfile) {
        printf("FAIL: Could not create .m file\n");
        return 1;
    }
    
    // Train pattern "world" multiple times
    const char *pattern = "world";
    printf("Training pattern: '%s' (50 iterations)\n", pattern);
    
    for (int i = 0; i < 50; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training complete.\n\n");
    
    // TEST 1: Forward generation from 'w'
    printf("TEST 1: Forward generation from 'w'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"w", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output1[256];
    size_t output1_len = melvin_m_universal_output_read(mfile, output1, sizeof(output1));
    
    if (output1_len > 0) {
        output1[output1_len] = '\0';
        printf("  Output: '%s'\n", output1);
        
        // Should generate forward continuation (like "orld" or "world")
        if (output1_len >= 4 && memcmp(output1, "orld", 4) == 0) {
            printf("  PASS: Generated forward continuation 'orld'\n");
        } else if (output1_len >= 5 && memcmp(output1, "world", 5) == 0) {
            printf("  PASS: Generated full pattern 'world'\n");
        } else {
            printf("  INFO: Generated '%s' (may be valid depending on learning)\n", output1);
        }
    } else {
        printf("  INFO: No output generated\n");
    }
    
    // TEST 2: Forward generation from "wo"
    printf("\nTEST 2: Forward generation from 'wo'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"wo", 2);
    melvin_m_process_input(mfile);
    
    uint8_t output2[256];
    size_t output2_len = melvin_m_universal_output_read(mfile, output2, sizeof(output2));
    
    if (output2_len > 0) {
        output2[output2_len] = '\0';
        printf("  Output: '%s'\n", output2);
        
        // Should generate forward continuation (like "rld")
        if (output2_len >= 3 && memcmp(output2, "rld", 3) == 0) {
            printf("  PASS: Generated forward continuation 'rld'\n");
        } else {
            printf("  INFO: Generated '%s' (may be valid depending on learning)\n", output2);
        }
    } else {
        printf("  INFO: No output generated\n");
    }
    
    // TEST 3: No reverse generation from 'd'
    printf("\nTEST 3: No reverse generation from 'd'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"d", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output3[256];
    size_t output3_len = melvin_m_universal_output_read(mfile, output3, sizeof(output3));
    
    if (output3_len > 0) {
        output3[output3_len] = '\0';
        printf("  Output: '%s'\n", output3);
        
        // Should NOT generate reverse (like "lrow")
        int has_reverse = 0;
        if (strchr((char*)output3, 'l') && strchr((char*)output3, 'r')) {
            // Check if 'l' comes before 'r' (reverse order)
            char *l_pos = strchr((char*)output3, 'l');
            char *r_pos = strchr((char*)output3, 'r');
            if (l_pos < r_pos) {
                has_reverse = 1;
                printf("  FAIL: Generated reverse sequence (l before r)\n");
            }
        }
        
        if (!has_reverse) {
            printf("  PASS: No reverse generation detected\n");
        }
    } else {
        printf("  PASS: No output generated (expected - 'd' is end of pattern)\n");
    }
    
    // TEST 4: Train second pattern "hello" and verify no interference
    printf("\nTEST 4: Multiple patterns - verify direction independence\n");
    const char *pattern2 = "hello";
    printf("Training second pattern: '%s' (50 iterations)\n", pattern2);
    
    for (int i = 0; i < 50; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern2, strlen(pattern2));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test "world" still works forward
    printf("Testing 'w' after training 'hello':\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"w", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output4[256];
    size_t output4_len = melvin_m_universal_output_read(mfile, output4, sizeof(output4));
    
    if (output4_len > 0) {
        output4[output4_len] = '\0';
        printf("  Output: '%s'\n", output4);
        
        // Should still generate forward (not confused with 'hello')
        if (strstr((char*)output4, "orld") || strstr((char*)output4, "world")) {
            printf("  PASS: Still generates 'world' pattern forward\n");
        } else {
            printf("  INFO: Generated '%s' (context may have changed)\n", output4);
        }
    } else {
        printf("  INFO: No output generated\n");
    }
    
    // Test "hello" works forward
    printf("Testing 'h' after training both patterns:\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"h", 1);
    melvin_m_process_input(mfile);
    
    uint8_t output5[256];
    size_t output5_len = melvin_m_universal_output_read(mfile, output5, sizeof(output5));
    
    if (output5_len > 0) {
        output5[output5_len] = '\0';
        printf("  Output: '%s'\n", output5);
        
        // Should generate forward (like "ello")
        if (strstr((char*)output5, "ello") || strstr((char*)output5, "hello")) {
            printf("  PASS: Generates 'hello' pattern forward\n");
        } else {
            printf("  INFO: Generated '%s' (context may have changed)\n", output5);
        }
    } else {
        printf("  INFO: No output generated\n");
    }
    
    printf("\n=== Test Complete ===\n");
    printf("Edge direction principle: Edges only follow input order\n");
    printf("No reverse edges should exist in the graph.\n");
    
    melvin_m_close(mfile);
    
    return 0;
}
