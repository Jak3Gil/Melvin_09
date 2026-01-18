#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../src/melvin.h"

int main() {
    const char *test_file = "test_two.m";
    unlink(test_file);
    MelvinMFile *mfile = melvin_m_create(test_file);
    
    printf("Training 'cat meow' 10 times...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"cat meow", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training 'dog bark' 10 times...\n");
    for (int i = 0; i < 10; i++) {
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_input_write(mfile, (const uint8_t*)"dog bark", 8);
        melvin_m_process_input(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("\nNodes: %zu, Edges: %zu\n", melvin_m_get_node_count(mfile), melvin_m_get_edge_count(mfile));
    
    printf("\nTest 'cat': ");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"cat", 3);
    melvin_m_process_input(mfile);
    size_t sz = melvin_m_universal_output_size(mfile);
    if (sz > 0) {
        uint8_t *out = malloc(sz + 1);
        melvin_m_universal_output_read(mfile, out, sz);
        for (size_t i = 0; i < sz && i < 10; i++) printf("%c", out[i] >= 32 ? out[i] : '.');
        free(out);
    }
    printf("\n");
    
    printf("Test 'dog': ");
    melvin_m_universal_input_clear(mfile);
    melvin_m_universal_input_write(mfile, (const uint8_t*)"dog", 3);
    melvin_m_process_input(mfile);
    sz = melvin_m_universal_output_size(mfile);
    if (sz > 0) {
        uint8_t *out = malloc(sz + 1);
        melvin_m_universal_output_read(mfile, out, sz);
        for (size_t i = 0; i < sz && i < 10; i++) printf("%c", out[i] >= 32 ? out[i] : '.');
        free(out);
    }
    printf("\n");
    
    melvin_m_close(mfile);
    unlink(test_file);
    return 0;
}
