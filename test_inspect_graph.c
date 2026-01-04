#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "melvin.h"

// Access internal structures (hack for debugging)
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct Graph Graph;

struct Node {
    uint8_t *payload;
    size_t payload_size;
    size_t payload_capacity;
    uint32_t abstraction_level;
    float weight;
    float bias;
    float state;
    uint32_t total_activations;
    uint32_t completion_count;
    float activation_strength;
    Edge **outgoing_edges;
    size_t outgoing_count;
    size_t outgoing_capacity;
    Edge **incoming_edges;
    size_t incoming_count;
    size_t incoming_capacity;
    // ... more fields ...
};

struct Graph {
    Node **nodes;
    size_t node_count;
    size_t node_capacity;
    // ... more fields ...
};

struct MelvinMFile {
    Graph *graph;
    // ... more fields ...
};

int main() {
    printf("=== Graph Structure Inspector ===\n\n");
    
    const char *test_file = "test_inspect.m";
    remove(test_file);
    
    MelvinMFile *mfile = melvin_m_create(test_file);
    if (!mfile) {
        printf("Failed to create .m file\n");
        return 1;
    }
    
    // Train on "hello" 200 times
    const char *pattern = "hello";
    printf("Training on '%s' 200 times...\n", pattern);
    
    for (int i = 0; i < 200; i++) {
        melvin_m_universal_input_write(mfile, (const uint8_t*)pattern, strlen(pattern));
        melvin_m_process_input(mfile);
        melvin_m_universal_input_clear(mfile);
        melvin_m_universal_output_clear(mfile);
    }
    
    printf("Training complete: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile),
           melvin_m_get_edge_count(mfile));
    
    // Access internal graph structure
    Graph *graph = mfile->graph;
    
    printf("=== Byte Nodes (abstraction_level = 0) ===\n");
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (node && node->abstraction_level == 0 && node->payload_size > 0) {
            printf("Node %zu: '", i);
            for (size_t j = 0; j < node->payload_size; j++) {
                if (node->payload[j] >= 32 && node->payload[j] < 127) {
                    printf("%c", node->payload[j]);
                } else {
                    printf("\\x%02x", node->payload[j]);
                }
            }
            printf("' (weight: %.2f, activations: %u, outgoing: %zu)\n", 
                   node->weight, node->total_activations, node->outgoing_count);
        }
    }
    
    printf("\n=== Hierarchy Nodes (abstraction_level > 0) ===\n");
    int hierarchy_count = 0;
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (node && node->abstraction_level > 0 && node->payload_size > 0) {
            hierarchy_count++;
            printf("Hierarchy %d (level %u): '", hierarchy_count, node->abstraction_level);
            for (size_t j = 0; j < node->payload_size; j++) {
                if (node->payload[j] >= 32 && node->payload[j] < 127) {
                    printf("%c", node->payload[j]);
                } else {
                    printf("\\x%02x", node->payload[j]);
                }
            }
            printf("' (%zu bytes, weight: %.2f, activations: %u)\n", 
                   node->payload_size, node->weight, node->total_activations);
        }
    }
    
    if (hierarchy_count == 0) {
        printf("(No hierarchies found)\n");
    }
    
    printf("\n=== Edges from 'l' node ===\n");
    Node *l_node = NULL;
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (node && node->abstraction_level == 0 && node->payload_size == 1 && node->payload[0] == 'l') {
            l_node = node;
            break;
        }
    }
    
    if (l_node) {
        printf("Found 'l' node with %zu outgoing edges:\n", l_node->outgoing_count);
        for (size_t i = 0; i < l_node->outgoing_count; i++) {
            Edge *edge = l_node->outgoing_edges[i];
            if (edge && edge->to_node && edge->to_node->payload_size > 0) {
                printf("  l → '");
                for (size_t j = 0; j < edge->to_node->payload_size; j++) {
                    if (edge->to_node->payload[j] >= 32 && edge->to_node->payload[j] < 127) {
                        printf("%c", edge->to_node->payload[j]);
                    } else {
                        printf("\\x%02x", edge->to_node->payload[j]);
                    }
                }
                printf("' (weight: %.2f)\n", edge->weight);
            }
        }
    } else {
        printf("'l' node not found!\n");
    }
    
    melvin_m_close(mfile);
    remove(test_file);
    
    return 0;
}

