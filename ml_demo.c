#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    MelvinMFile *mfile = melvin_m_create("ml_demo_brain.m");
    if (!mfile) return 1;
    
    // Feed "hello" many times to strengthen the pattern
    printf("=== Strengthening 'hello' pattern ===\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (i % 3 == 0) {
            printf("After %d inputs: %zu nodes, %zu edges, output: %zu bytes\n",
                   i+1, melvin_m_get_node_count(mfile), 
                   melvin_m_get_edge_count(mfile), output_size);
        }
        if (output_size > 0) {
            uint8_t buf[256];
            size_t read = melvin_m_universal_output_read(mfile, buf, sizeof(buf));
            printf("  OUTPUT: \"");
            for (size_t j = 0; j < read && j < 50; j++) {
                if (buf[j] >= 32 && buf[j] < 127) printf("%c", buf[j]);
            }
            printf("\"\n");
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\n=== Strengthening 'world' pattern ===\n");
    for (int i = 0; i < 10; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"world", 5);
        size_t output_size = melvin_m_universal_output_size(mfile);
        if (i % 3 == 0) {
            printf("After %d inputs: %zu nodes, %zu edges, output: %zu bytes\n",
                   i+10+1, melvin_m_get_node_count(mfile), 
                   melvin_m_get_edge_count(mfile), output_size);
        }
        if (output_size > 0) {
            uint8_t buf[256];
            size_t read = melvin_m_universal_output_read(mfile, buf, sizeof(buf));
            printf("  OUTPUT: \"");
            for (size_t j = 0; j < read && j < 50; j++) {
                if (buf[j] >= 32 && buf[j] < 127) printf("%c", buf[j]);
            }
            printf("\"\n");
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\n=== Testing novel combination 'hello world' ===\n");
    for (int i = 0; i < 5; i++) {
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello world", 11);
        size_t output_size = melvin_m_universal_output_size(mfile);
        printf("Input %d: output: %zu bytes\n", i+1, output_size);
        if (output_size > 0) {
            uint8_t buf[256];
            size_t read = melvin_m_universal_output_read(mfile, buf, sizeof(buf));
            printf("  OUTPUT: \"");
            for (size_t j = 0; j < read && j < 50; j++) {
                if (buf[j] >= 32 && buf[j] < 127) printf("%c", buf[j]);
            }
            printf("\"\n");
            melvin_m_universal_output_clear(mfile);
        }
    }
    
    printf("\n=== Final Stats ===\n");
    printf("Nodes: %zu, Edges: %zu, Adaptations: %lu\n",
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile),
           (unsigned long)melvin_m_get_adaptation_count(mfile));
    
    melvin_m_close(mfile);
    return 0;
}
