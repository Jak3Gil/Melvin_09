#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "src/melvin.h"

float calculate_error_rate(const char *expected, uint8_t *actual, size_t actual_len) {
    if (!expected || !actual) return 1.0f;
    size_t expected_len = strlen(expected);
    if (expected_len == 0 && actual_len == 0) return 0.0f;
    if (expected_len == 0 || actual_len == 0) return 1.0f;
    
    size_t errors = 0;
    size_t max_len = (actual_len > expected_len) ? actual_len : expected_len;
    for (size_t i = 0; i < max_len; i++) {
        if (i >= actual_len || i >= expected_len || actual[i] != (uint8_t)expected[i]) {
            errors++;
        }
    }
    return (max_len > 0) ? ((float)errors / (float)max_len) : 1.0f;
}

int main() {
    printf("=== Error Rate with New Edge Rules ===\n\n");
    
    const char *test_file = "test_error_new_edges.m";
    unlink(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create brain\n");
        return 1;
    }
    
    struct {
        const char *input;
        const char *expected;
    } patterns[] = {
        {"cat", " meow"},
        {"dog", " bark"},
        {"bird", " chirp"},
    };
    int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    
    printf("Training %d patterns (50 iterations each)...\n", num_patterns);
    float first_error = 1.0f;
    float last_error = 1.0f;
    
    for (int cycle = 0; cycle < 50; cycle++) {
        // Train
        for (int p = 0; p < num_patterns; p++) {
            char full[64];
            snprintf(full, sizeof(full), "%s%s", patterns[p].input, patterns[p].expected);
            
            melvin_m_universal_input_clear(mfile);
            melvin_m_universal_input_write(mfile, (const uint8_t*)full, strlen(full));
            melvin_m_process_input(mfile);
            melvin_m_universal_output_clear(mfile);
        }
        
        // Test every 10 cycles
        if ((cycle + 1) % 10 == 0 || cycle == 0) {
            float total_error = 0.0f;
            
            for (int p = 0; p < num_patterns; p++) {
                melvin_m_universal_input_clear(mfile);
                melvin_m_universal_input_write(mfile, (const uint8_t*)patterns[p].input, strlen(patterns[p].input));
                melvin_m_process_input(mfile);
                
                uint8_t output[256];
                size_t out_len = melvin_m_universal_output_read(mfile, output, sizeof(output));
                
                total_error += calculate_error_rate(patterns[p].expected, output, out_len);
                melvin_m_universal_output_clear(mfile);
            }
            
            float avg_error = total_error / num_patterns;
            if (cycle == 0) first_error = avg_error;
            last_error = avg_error;
            
            printf("  Cycle %d: Error = %.1f%%\n", cycle + 1, avg_error * 100.0f);
        }
    }
    
    printf("\nSummary:\n");
    printf("  Nodes: %zu, Edges: %zu (%.2f edges/node)\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile),
           (float)melvin_m_get_edge_count(mfile) / (float)melvin_m_get_node_count(mfile));
    printf("  Initial error: %.1f%%\n", first_error * 100.0f);
    printf("  Final error: %.1f%%\n", last_error * 100.0f);
    printf("  Improvement: %.1f%%\n", (first_error - last_error) * 100.0f);
    
    melvin_m_close(mfile);
    unlink(test_file);
    
    return 0;
}
