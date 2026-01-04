#include "melvin.h"
#include "melvin_in_port.h"
#include <stdio.h>
#include <string.h>

int main() {
    MelvinMFile *mfile = melvin_m_create("debug_output.m");
    if (!mfile) return 1;
    
    printf("Testing: Input 'hello'\n");
    melvin_in_port_handle_buffer(mfile, 0, (uint8_t*)"hello", 5);
    
    size_t output_size = melvin_m_universal_output_size(mfile);
    printf("Output size: %zu\n", output_size);
    
    if (output_size > 0) {
        uint8_t buf[256];
        size_t read = melvin_m_universal_output_read(mfile, buf, sizeof(buf));
        printf("Output bytes: ");
        for (size_t i = 0; i < read; i++) {
            printf("'%c' (0x%02x) ", (buf[i] >= 32 && buf[i] < 127) ? buf[i] : '?', buf[i]);
        }
        printf("\n");
    }
    
    printf("Nodes: %zu, Edges: %zu\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    return 0;
}
