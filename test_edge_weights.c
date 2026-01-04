#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// We need to access internal structures
typedef struct Node Node;
typedef struct Edge Edge;

// Declare the internal structure (matching melvin.c)
struct Edge {
    Node *from_node;
    Node *to_node;
    uint8_t direction;
    uint32_t last_wave_generation;
    float weight;
    float routing_gate;
    long file_offset;
    float inactivity_timer;
    uint8_t marked_for_deletion;
    float cached_similarity;
    uint8_t is_similarity_edge;
};

struct Node {
    uint8_t *payload;
    size_t payload_size;
    size_t payload_capacity;
    
    struct Edge **outgoing_edges;
    size_t outgoing_count;
    size_t outgoing_capacity;
    float outgoing_weight_sum;
    
    struct Edge **incoming_edges;
    size_t incoming_count;
    size_t incoming_capacity;
    float incoming_weight_sum;
    
    // ... other fields we don't need
};

typedef struct MFile MFile;

#include "melvin.h"

int main() {
    printf("=== Edge Weight Analysis ===\n\n");
    
    MelvinMFile *mfile = melvin_m_create("test_weights.m");
    
    // Learn "hello" - creates h→e→l→l→o edges
    printf("Learning 'hello'\n");
    melvin_m_universal_input_write(mfile, (const uint8_t*)"Xhello", 6);
    melvin_m_process_input(mfile);
    
    printf("Graph: %zu nodes, %zu edges\n\n", 
           melvin_m_get_node_count(mfile), 
           melvin_m_get_edge_count(mfile));
    
    // Access internal graph to inspect edges
    // This is a hack for debugging
    MFile *internal = (MFile*)mfile;
    
    printf("Expected edges from 'hello':\n");
    printf("  h→e (1 time)\n");
    printf("  e→l (1 time)\n");
    printf("  l→l (1 time) - first 'l' to second 'l'\n");
    printf("  l→o (1 time) - second 'l' to 'o'\n\n");
    
    printf("Question: Why is l→l stronger than l→o?\n");
    printf("Both should have been activated once in 'hello'\n\n");
    
    printf("Hypothesis:\n");
    printf("1. Maybe l→l gets strengthened multiple times?\n");
    printf("2. Maybe similarity edges are interfering?\n");
    printf("3. Maybe the pattern processing creates extra l→l edges?\n");
    
    melvin_m_close(mfile);
    return 0;
}

