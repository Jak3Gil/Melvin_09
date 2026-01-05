#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// We need to access internal structures to inspect hierarchies
// This is a debug-only test

// Declare internal structures (from melvin.c)
typedef struct Node {
    uint8_t *payload;
    size_t payload_size;
    uint32_t abstraction_level;
    // ... other fields we don't need for this test
} Node;

typedef struct Graph {
    Node **nodes;
    size_t node_count;
    // ... other fields
} Graph;

typedef struct MFile {
    Graph *graph;
    // ... other fields
} MFile;

// External access functions
extern Node** melvin_m_get_nodes(void *mfile_ptr);
extern size_t melvin_m_get_node_count(void *mfile_ptr);

int main() {
    printf("=== Hierarchy Inspection Test ===\n\n");
    
    const char *test_file = "test_inspect.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    const char *pattern = "hello";
    
    printf("Training on '%s' 100 times...\n", pattern);
    for (int i = 0; i < 100; i++) {
        melvin_m_universal_input_write(mfile, (uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    size_t node_count = melvin_m_get_node_count(mfile);
    printf("Graph has %zu nodes\n\n", node_count);
    
    printf("Attempting to access internal graph structure...\n");
    printf("(This test requires direct memory access - may not work)\n\n");
    
    // Try to access internal structure
    // This is hacky but needed for debugging
    MFile *internal_mfile = (MFile*)mfile;
    if (internal_mfile && internal_mfile->graph) {
        Graph *graph = internal_mfile->graph;
        
        printf("Found %zu nodes in graph:\n\n", graph->node_count);
        
        int byte_nodes = 0;
        int hierarchy_nodes = 0;
        
        for (size_t i = 0; i < graph->node_count && i < 50; i++) {
            Node *node = graph->nodes[i];
            if (!node) continue;
            
            printf("Node %2zu: abstraction=%u, size=%zu, payload='", 
                   i, node->abstraction_level, node->payload_size);
            
            for (size_t j = 0; j < node->payload_size && j < 20; j++) {
                if (node->payload[j] >= 32 && node->payload[j] < 127) {
                    printf("%c", node->payload[j]);
                } else {
                    printf("\\x%02x", node->payload[j]);
                }
            }
            printf("'\n");
            
            if (node->abstraction_level == 0) {
                byte_nodes++;
            } else {
                hierarchy_nodes++;
            }
        }
        
        printf("\nSummary:\n");
        printf("  Byte nodes (abstraction=0): %d\n", byte_nodes);
        printf("  Hierarchy nodes (abstraction>0): %d\n", hierarchy_nodes);
        
        if (hierarchy_nodes > 0) {
            printf("\n✓ Hierarchies exist!\n");
            printf("  Now checking if 'hello' hierarchy exists...\n\n");
            
            int found_hello = 0;
            for (size_t i = 0; i < graph->node_count; i++) {
                Node *node = graph->nodes[i];
                if (!node || node->abstraction_level == 0) continue;
                if (node->payload_size != 5) continue;
                
                if (memcmp(node->payload, "hello", 5) == 0) {
                    found_hello = 1;
                    printf("  ✓ Found 'hello' hierarchy at node %zu!\n", i);
                    break;
                }
            }
            
            if (!found_hello) {
                printf("  ✗ 'hello' hierarchy not found\n");
                printf("  Hierarchies are being created but not the full pattern\n");
            }
        } else {
            printf("\n✗ No hierarchies created!\n");
        }
    } else {
        printf("Failed to access internal structure\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

