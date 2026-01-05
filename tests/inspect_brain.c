/* Simple brain inspector - shows edge weights and hierarchies */
#include "melvin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <brain.m>\n", argv[0]);
        return 1;
    }
    
    MelvinMFile *mfile = melvin_m_load(argv[1]);
    if (!mfile) {
        fprintf(stderr, "Failed to load brain file\n");
        return 1;
    }
    
    Graph *graph = mfile->graph;
    if (!graph) {
        fprintf(stderr, "No graph loaded\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    printf("=== BRAIN ANALYSIS ===\n");
    printf("Nodes: %zu\n", graph->node_count);
    printf("Edges: %zu\n", graph->edge_count);
    printf("Max abstraction level: %u\n\n", graph->max_abstraction_level);
    
    // Find 'o' node and show its outgoing edges
    Node *o_node = NULL;
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *n = graph->nodes[i];
        if (n && n->payload_size == 1 && n->payload[0] == 'o') {
            o_node = n;
            break;
        }
    }
    
    if (o_node) {
        printf("=== 'o' NODE EDGES (Signal vs Noise) ===\n");
        printf("Node weight: %.2f\n", o_node->weight);
        printf("Outgoing edges: %zu\n", o_node->outgoing_count);
        
        for (size_t i = 0; i < o_node->outgoing_count; i++) {
            Edge *e = o_node->outgoing_edges[i];
            if (e && e->to_node && e->to_node->payload_size > 0) {
                uint8_t byte = e->to_node->payload[0];
                char label[32] = {0};
                if (byte >= 32 && byte < 127) {
                    snprintf(label, sizeof(label), "'%c'", byte);
                } else {
                    snprintf(label, sizeof(label), "0x%02x", byte);
                }
                printf("  Edge[%zu]: o -> %s, weight=%.2f, timer=%.2f\n", 
                       i, label, e->weight, e->inactivity_timer);
            }
        }
        printf("\n");
    }
    
    // Show hierarchies
    printf("=== HIERARCHIES (Level > 0) ===\n");
    int found_hierarchies = 0;
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *n = graph->nodes[i];
        if (n && n->abstraction_level > 0 && n->payload_size > 0) {
            found_hierarchies = 1;
            printf("  Level %u, %zu bytes: \"", n->abstraction_level, n->payload_size);
            for (size_t j = 0; j < n->payload_size && j < 50; j++) {
                if (n->payload[j] >= 32 && n->payload[j] < 127) {
                    printf("%c", n->payload[j]);
                } else {
                    printf("\\x%02x", n->payload[j]);
                }
            }
            printf("\", weight=%.2f\n", n->weight);
        }
    }
    if (!found_hierarchies) {
        printf("  (No hierarchies found)\n");
    }
    printf("\n");
    
    // Show all nodes with their weights
    printf("=== ALL NODES (Top 20 by weight) ===\n");
    // Simple sort by weight (bubble sort for small n)
    Node **sorted = malloc(graph->node_count * sizeof(Node*));
    for (size_t i = 0; i < graph->node_count; i++) {
        sorted[i] = graph->nodes[i];
    }
    for (size_t i = 0; i < graph->node_count && i < 20; i++) {
        for (size_t j = i + 1; j < graph->node_count; j++) {
            if (sorted[j] && sorted[i] && sorted[j]->weight > sorted[i]->weight) {
                Node *tmp = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = tmp;
            }
        }
    }
    
    for (size_t i = 0; i < graph->node_count && i < 20; i++) {
        Node *n = sorted[i];
        if (!n || n->payload_size == 0) continue;
        printf("  ");
        for (size_t j = 0; j < n->payload_size && j < 20; j++) {
            if (n->payload[j] >= 32 && n->payload[j] < 127) {
                printf("%c", n->payload[j]);
            } else {
                printf("?");
            }
        }
        printf(": weight=%.2f, level=%u, outgoing=%zu\n", 
               n->weight, n->abstraction_level, n->outgoing_count);
    }
    free(sorted);
    
    melvin_m_close(mfile);
    return 0;
}

