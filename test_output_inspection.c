/* test_output_inspection.c
 * Inspect what's actually being generated
 */

#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_bytes(const uint8_t *data, size_t len, size_t max_display) {
    size_t display_len = (len < max_display) ? len : max_display;
    
    printf("  Hex: ");
    for (size_t i = 0; i < display_len; i++) {
        printf("%02x ", data[i]);
    }
    if (len > max_display) printf("...");
    printf("\n");
    
    printf("  ASCII: '");
    for (size_t i = 0; i < display_len; i++) {
        if (isprint(data[i])) {
            printf("%c", data[i]);
        } else {
            printf(".");
        }
    }
    if (len > max_display) printf("...");
    printf("'\n");
}

int main() {
    printf("=== Output Inspection Test ===\n\n");
    
    const char *path = "test_inspect.m";
    MelvinMFile *mfile = melvin_m_create(path);
    if (!mfile) {
        printf("❌ Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Test 1: After 1 learning iteration\n");
    printf("Input: '%s'\n", pattern);
    melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
    melvin_m_process_input(mfile);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[100];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Read %zu bytes:\n", read_size);
        print_bytes(output, read_size, 50);
    }
    
    printf("\nTest 2: After 10 learning iterations\n");
    for (int i = 0; i < 9; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
    }
    
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[100];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Read %zu bytes:\n", read_size);
        print_bytes(output, read_size, 50);
    }
    
    printf("\nTest 3: After 50 learning iterations\n");
    for (int i = 0; i < 40; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
    }
    
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu bytes\n", output_size);
    
    if (output_size > 0) {
        uint8_t output[100];
        size_t read_size = melvin_m_universal_output_read(mfile, output, sizeof(output));
        printf("Read %zu bytes:\n", read_size);
        print_bytes(output, read_size, 50);
    }
    
    printf("\n=== Analysis ===\n");
    printf("Expected: Output should converge toward input pattern\n");
    printf("Expected: Later iterations should show better pattern completion\n");
    printf("Expected: Output should be readable/meaningful\n");
    
    melvin_m_close(mfile);
    return 0;
}

