/*
 * DEBUG: How does output change with more iterations?
 */

#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_iterations(int count) {
    char filename[64];
    snprintf(filename, sizeof(filename), "/tmp/debug_iter_%d.m", count);
    remove(filename);
    
    MelvinMFile *mfile = melvin_m_create(filename);
    if (!mfile) return;
    
    // Train
    for (int i = 0; i < count; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        melvin_m_universal_output_clear(mfile);
    }
    
    // Test
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t out_size = melvin_m_universal_output_size(mfile);
    uint8_t *output = malloc(out_size + 1);
    melvin_m_universal_output_read(mfile, output, out_size);
    output[out_size] = '\0';
    
    fprintf(stderr, "%d iterations: '%s' (%zu bytes)\n", count, output, out_size);
    
    free(output);
    melvin_m_close(mfile);
    remove(filename);
}

int main() {
    fprintf(stderr, "╔════════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║         ITERATION COUNT VS OUTPUT LENGTH                   ║\n");
    fprintf(stderr, "╚════════════════════════════════════════════════════════════╝\n\n");
    
    test_iterations(1);
    test_iterations(2);
    test_iterations(3);
    test_iterations(5);
    test_iterations(10);
    test_iterations(20);
    
    fprintf(stderr, "\nExpected: ' world' (6 bytes)\n");
    
    return 0;
}
