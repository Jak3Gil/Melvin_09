#include "melvin.h"
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    MelvinMFile *mfile = melvin_m_create("learning_brain.m");
    if (!mfile) return 1;
    
    const char *inputs[] = {"hello", "hello", "hello", "world", "world", "hello world", NULL};
    
    for (int i = 0; inputs[i]; i++) {
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        printf("Input %d: '%s'\n", i+1, inputs[i]);
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
        
        size_t nodes_before = melvin_m_get_node_count(mfile);
        size_t edges_before = melvin_m_get_edge_count(mfile);
        
        melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)inputs[i], strlen(inputs[i]));
        
        size_t nodes_after = melvin_m_get_node_count(mfile);
        size_t edges_after = melvin_m_get_edge_count(mfile);
        
        size_t output_size = melvin_m_universal_output_size(mfile);
        
        printf("Learning: +%zu nodes, +%zu edges\n", 
               nodes_after - nodes_before, edges_after - edges_before);
        printf("Total: %zu nodes, %zu edges\n", nodes_after, edges_after);
        printf("Output: %zu bytes\n", output_size);
        
        if (output_size > 0) {
            uint8_t buf[256];
            size_t read = melvin_m_universal_output_read(mfile, buf, sizeof(buf));
            printf("Content: \"");
            for (size_t j = 0; j < read && j < 50; j++) {
                if (buf[j] >= 32 && buf[j] < 127) printf("%c", buf[j]);
                else printf(".");
            }
            printf("\"\n");
            melvin_m_universal_output_clear(mfile);
        } else {
            printf("Content: (thinking mode - patterns not mature yet)\n");
        }
        printf("\n");
    }
    
    melvin_m_close(mfile);
    return 0;
}
