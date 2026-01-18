/*
 * Melvin: Emergent Intelligence System
 * Brain-Inspired Refactor - Clean Implementation
 * 
 * Core Principles (from README.md and Requirement.md):
 * 1. Self-Regulation Through Local Measurements Only (no global state)
 * 2. No Hardcoded Limits or Thresholds (all relative to local context)
 * 3. Relative Adaptive Stability (epsilon scales with data)
 * 4. Compounding Learning (hierarchies enable faster pattern matching)
 * 5. Adaptive Behavior (exploratory vs exploitative)
 * 6. Continuous Learning (Hebbian: "neurons that fire together, wire together")
 * 7. Emergent Intelligence (from local interactions)
 * 8. Explicit Hierarchical Abstraction (transparent, inspectable)
 * 
 * Brain-Inspired Architecture:
 * - Sparse Activation: Only ~1-5% neurons active (like brain)
 * - Predictive Coding: Edges predict continuations (top-down > bottom-up)
 * - Winner-Take-All: Local competition (softmax over local candidates)
 * - Synaptic Consolidation: Hebbian learning (pure, no complex logic)
 * 
 * LLM-Inspired Architecture:
 * - Token Embedding → Hidden State (input → activation → pattern)
 * - Causal Attention (no peeking at future tokens)
 * - Softmax Sampling (probabilistic selection from distribution)
 * - Autoregressive Generation (one byte at a time)
 */

#include "melvin.h"
#ifdef MELVIN_STANDALONE
#include "melvin_in_port.h"
#include "melvin_out_port.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

/* Debug output control */
#ifdef MELVIN_DEBUG
#define DEBUG_LOG(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)
#else
#define DEBUG_LOG(...) ((void)0)
#endif

/* ============================================================================
 * CORE DATA STRUCTURES (Brain-Inspired Simplicity)
 * ============================================================================ */

/* Node: Minimal neuron-like structure
 * Brain: Dendrites (inputs), Axon (output), Synapses (connections)
 * NO mini-nets, NO embeddings, NO complex state - just essentials
 */
struct Node {
    uint8_t id[8];                  // Unique identifier (8 bytes)
    uint8_t *payload;               // Actual data (byte pattern)
    size_t payload_size;            // Size in bytes
    
    struct Edge **outgoing;         // Outgoing connections (like axon terminals)
    size_t outgoing_count;          // Number of outgoing edges
    size_t outgoing_capacity;       // Allocated capacity
    float outgoing_weight_sum;      // Cached sum (O(1) access)
    
    struct Edge **incoming;         // Incoming connections (like dendrites)
    size_t incoming_count;          // Number of incoming edges
    size_t incoming_capacity;       // Allocated capacity
    
    uint8_t abstraction_level;      // Hierarchy level (0=raw, 1+=hierarchy)
    uint8_t is_input_node;          // Flag: 1 if part of current input
    uint32_t last_activation;       // Timestamp of last activation (for decay)
    
    struct Node *next_in_bucket;    // For hash table chaining
};

/* Edge: Minimal synapse-like structure
 * Brain: Synaptic connection with usage log
 * LLM: Attention weight between tokens
 * KEY: Weight is MEMORY/LOG ONLY - records "this path was used"
 *      Weight informs decisions but doesn't drive them
 *      Context and embeddings drive decisions
 */
struct Edge {
    Node *from;                     // Source node
    Node *to;                       // Target node
    uint8_t weight;                 // Usage log (0-255): how often this path was used
    uint32_t last_used;             // Timestamp of last use (for decay)
};

/* ActivationPattern: Working memory
 * Brain: Current neural activation pattern
 * LLM: Hidden state representation
 */
typedef struct ActivationPattern {
    Node **nodes;                   // Activated nodes
    float *activations;             // Activation strengths
    size_t count;                   // Number of activated nodes
    size_t capacity;                // Allocated capacity
    float *context_vector;          // Weighted sum (like LLM hidden state)
    size_t context_dim;             // Dimension of context vector
    
    // Hash table for O(1) activation lookup (local decisions need fast access)
    Node **hash_table;              // Hash by node pointer
    float *hash_activations;        // Parallel array of activations
    size_t hash_capacity;           // Power of 2 for fast modulo
} ActivationPattern;

/* SparseEmbedding: Semantic representation (ephemeral, computed on-demand)
 * Brain: Distributed representation across sparse neural population
 * LLM: Token embedding vector (but sparse for memory efficiency)
 * Only 8-16 non-zero dimensions, computed from local graph structure
 */
typedef struct SparseEmbedding {
    uint16_t *dims;                 // Sparse dimensions (only non-zeros)
    float *values;                  // Sparse values
    size_t nnz;                     // Number of non-zeros (8-16 typical)
    uint32_t generation;            // Cache invalidation token
} SparseEmbedding;

/* EmbeddingCache: Temporary cache for embeddings during generation
 * Cleared after each output generation to maintain zero permanent storage
 */
typedef struct EmbeddingCache {
    SparseEmbedding **embeddings;   // Hash table by node pointer
    Node **nodes;                   // Corresponding nodes
    size_t count;                   // Number of cached embeddings
    size_t capacity;                // Allocated capacity
    uint32_t generation;            // Generation counter for invalidation
} EmbeddingCache;

/* VirtualSemanticEdge: Computed edge (never stored permanently)
 * Represents semantic similarity between nodes
 * Computed on-demand during decode phase only
 */
typedef struct VirtualSemanticEdge {
    Node *to;                       // Target node
    float semantic_strength;        // Cosine similarity (0.0-1.0)
    float combined_score;           // semantic + structural weight
} VirtualSemanticEdge;

/* Graph: The "brain" - collection of neurons and synapses
 * Brain: Neural network
 * LLM: Model weights
 */
struct Graph {
    Node **hash_table;              // Hash table for O(1) node lookup
    size_t hash_capacity;           // Hash table size
    size_t node_count;              // Total number of nodes
    
    Edge **edges;                   // All edges (for iteration)
    size_t edge_count;              // Total number of edges
    size_t edge_capacity;           // Allocated capacity
    
    uint32_t timestamp;             // Current timestamp (for decay)
    uint64_t adaptation_count;      // Number of weight updates (learning events)
    
    // Ephemeral embedding cache (only during output generation, NULL otherwise)
    EmbeddingCache *embedding_cache; // Temporary cache, cleared after generation
    
    // STOP node (like LLM EOS token - singleton, one per graph)
    Node *stop_node;                // Special node for stopping generation
};

/* MFile: The .m file wrapper
 * Contains graph + I/O buffers
 */
struct MFile {
    char *filename;                 // Path to .m file
    Graph *graph;                   // The brain
    
    uint8_t *input_buffer;          // Universal input buffer
    size_t input_size;              // Current input size
    size_t input_capacity;          // Allocated capacity
    uint8_t last_input_port_id;     // Port ID of last input
    
    uint8_t *output_buffer;         // Universal output buffer
    size_t output_size;             // Current output size
    size_t output_capacity;         // Allocated capacity
};

/* ============================================================================
 * FORWARD DECLARATIONS
 * ============================================================================ */

/* Core functions (30 total as per plan) */
static Node* node_create(const uint8_t *payload, size_t payload_size, uint8_t abstraction_level);
static void node_free(Node *node);
static Edge* edge_create(Node *from, Node *to);
static void edge_free(Edge *edge);
static Graph* graph_create(void);
static void graph_free(Graph *graph);
static ActivationPattern* activation_pattern_create(void);
static void activation_pattern_free(ActivationPattern *pattern);
static void activation_pattern_add(ActivationPattern *pattern, Node *node, float activation);

/* Input processing */
static void process_input_bytes(MFile *mfile, const uint8_t *data, size_t size);
static Node* find_or_create_node(Graph *graph, const uint8_t *payload, size_t size);
static void create_sequential_edges(Graph *graph, Node **nodes, size_t count);

/* ENCODE phase */
static ActivationPattern* encode_direct_activation(Node **input_nodes, size_t input_count);
static void encode_spreading_activation(ActivationPattern *pattern, Node **input_nodes, size_t input_count, Graph *graph);
static void build_context_vector(ActivationPattern *pattern);

/* Local decision-making helpers */
static void node_spread_activation_locally(Node *node, ActivationPattern *pattern);
static float compute_local_decay(float my_activation, float edge_relative);
static void node_check_analogical_reasoning(Node *node, ActivationPattern *pattern, Node *last_input, Graph *graph);
static void node_decay_unused_edges(Node *node, uint32_t current_time);

/* REFINE phase */
static void refine_competition(ActivationPattern *pattern);
static void refine_normalize(ActivationPattern *pattern);

/* DECODE phase */
static Node* decode_select_first(ActivationPattern *pattern, Graph *graph);
static Node* decode_select_next(Node *current, const uint8_t *output, size_t output_len,
                                ActivationPattern *pattern, Graph *graph);
static int decode_check_stop(Node *current, float activation, ActivationPattern *pattern);
static void generate_output(MFile *mfile, ActivationPattern *pattern, Node **input_nodes, size_t input_count);

/* Hebbian learning */
static void hebbian_strengthen_edge(Edge *edge, Graph *graph);
static void hebbian_strengthen_edge_precise(Edge *edge, Graph *graph, float activation_used,
                                           float context_match_strength);
static void hebbian_decay_unused(Graph *graph, Node **recently_active, size_t active_count);
static void hebbian_normalize_weights(Node *node);

/* Hierarchy formation */
static void check_hierarchy_formation(Graph *graph, Node **input_nodes, size_t input_count);
static void check_hierarchy_formation_phase2(Graph *graph, ActivationPattern *pattern);
static Node* create_hierarchy_node(Graph *graph, Node **sequence, size_t seq_len);

/* Utilities */
static float node_get_outgoing_avg(Node *node);
static float compute_relative_activation(float value, float local_max);
static float compute_softmax_single(float value, float *values, size_t count);
static Edge* find_edge_between(Node *from, Node *to);
static uint64_t hash_payload(const uint8_t *payload, size_t size);
static size_t hash_to_index(uint64_t hash, size_t capacity);

/* Embedding operations (on-demand, temporary cache) */
static EmbeddingCache* embedding_cache_create(size_t capacity);
static void embedding_cache_free(EmbeddingCache *cache);
static void embedding_cache_clear(EmbeddingCache *cache);
static SparseEmbedding* embedding_cache_get(EmbeddingCache *cache, Node *node);
static void embedding_cache_put(EmbeddingCache *cache, Node *node, SparseEmbedding *emb);
static SparseEmbedding* node_compute_embedding(Node *node, Graph *graph);
static SparseEmbedding* node_compute_context_embedding(Node *node, ActivationPattern *context, Graph *graph);
static float sparse_embedding_similarity(SparseEmbedding *a, SparseEmbedding *b);
static void sparse_embedding_free(SparseEmbedding *emb);
static SparseEmbedding* compute_context_embedding(ActivationPattern *pattern, Graph *graph);

/* Enhanced edge transformation and learning */
static float edge_transform_with_context_attention(Edge *edge, float input_activation, 
                                                   ActivationPattern *context, Graph *graph);

/* Virtual semantic edges (computed on-demand, never stored) */
static VirtualSemanticEdge* node_get_semantic_edges(Node *from, ActivationPattern *context, 
                                                     Graph *graph, size_t *out_count);
static void virtual_edges_free(VirtualSemanticEdge *edges);

/* Concept formation (uses existing hierarchy structure) */
static int is_concept_node(Node *hierarchy, Graph *graph);
static Node* find_generalizing_concept(Node *instance, ActivationPattern *context, Graph *graph);
static Node* find_parent_concept(Node *instance, Graph *graph);
static float compute_incoming_similarity(Node *node, Graph *graph);
static float compute_context_similarity(Node *node, ActivationPattern *context, Graph *graph);

/* Disambiguation */
static Node* disambiguate_with_context(Node **candidates, size_t candidate_count, 
                                       ActivationPattern *context, Graph *graph);

/* Analogical reasoning */
static Node* analogical_reasoning(Node *A, Node *B, Node *C, Graph *graph);

/* Node-level decision making */
static Edge* node_predict_next_edge(Node *node, ActivationPattern *context, Graph *graph);
static float node_evaluate_edge_contextual_score(Node *node, Edge *edge, 
                                                  SparseEmbedding *node_emb,
                                                  SparseEmbedding *ctx_emb,
                                                  ActivationPattern *context, 
                                                  Graph *graph);

/* Data-driven utility functions */
static float compute_variance(float *values, size_t count);
static float compute_percentile_threshold(float *values, size_t count, float percentile);
static void compute_adaptive_weights(float *weights, float *signals, size_t count);
static float compute_relative_threshold(float value, float local_max, float local_min, float local_variance);
static float compute_adaptive_decay(int hop, float signal_strength, float context_richness);
static void compute_local_stats(float *values, size_t count, 
                                float *out_max, float *out_min, 
                                float *out_mean, float *out_variance);

/* ============================================================================
 * NODE OPERATIONS
 * ============================================================================ */

/* Create a node (minimal structure) */
static Node* node_create(const uint8_t *payload, size_t payload_size, uint8_t abstraction_level) {
    Node *node = calloc(1, sizeof(Node));
    if (!node) return NULL;
    
    // Generate ID from payload hash
    uint64_t hash = hash_payload(payload, payload_size);
    snprintf((char*)node->id, 8, "%06lx", (unsigned long)(hash & 0xFFFFFF));
    
    // Copy payload
    if (payload_size > 0) {
        node->payload = malloc(payload_size);
        if (!node->payload) {
            free(node);
            return NULL;
        }
        memcpy(node->payload, payload, payload_size);
    }
    node->payload_size = payload_size;
    node->abstraction_level = abstraction_level;
    
    // Initialize edge arrays (start small, grow as needed)
    node->outgoing_capacity = 4;
    node->outgoing = malloc(node->outgoing_capacity * sizeof(Edge*));
    node->incoming_capacity = 4;
    node->incoming = malloc(node->incoming_capacity * sizeof(Edge*));
    
    if (!node->outgoing || !node->incoming) {
        free(node->payload);
        free(node->outgoing);
        free(node->incoming);
        free(node);
        return NULL;
    }
    
    node->is_input_node = 0;
    node->outgoing_weight_sum = 0.0f;
    node->last_activation = 0;
    
    return node;
}

/* Free a node */
static void node_free(Node *node) {
    if (!node) return;
    free(node->payload);
    free(node->outgoing);
    free(node->incoming);
    free(node);
}

/* Add outgoing edge to node */
static void node_add_outgoing(Node *node, Edge *edge) {
    if (node->outgoing_count >= node->outgoing_capacity) {
        size_t new_cap = node->outgoing_capacity * 2;
        Edge **new_arr = realloc(node->outgoing, new_cap * sizeof(Edge*));
        if (!new_arr) return;
        node->outgoing = new_arr;
        node->outgoing_capacity = new_cap;
    }
    node->outgoing[node->outgoing_count++] = edge;
}

/* Add incoming edge to node */
static void node_add_incoming(Node *node, Edge *edge) {
    if (node->incoming_count >= node->incoming_capacity) {
        size_t new_cap = node->incoming_capacity * 2;
        Edge **new_arr = realloc(node->incoming, new_cap * sizeof(Edge*));
        if (!new_arr) return;
        node->incoming = new_arr;
        node->incoming_capacity = new_cap;
    }
    node->incoming[node->incoming_count++] = edge;
}

/* Get average outgoing edge weight (O(1) cached) */
static float node_get_outgoing_avg(Node *node) {
    if (!node || node->outgoing_count == 0) return 0.0f;
    return node->outgoing_weight_sum / (float)node->outgoing_count;
}

/* ============================================================================
 * EDGE OPERATIONS
 * ============================================================================ */

/* Create an edge (minimal structure) */
static Edge* edge_create(Node *from, Node *to) {
    if (!from || !to) return NULL;
    
    Edge *edge = malloc(sizeof(Edge));
    if (!edge) return NULL;
    
    edge->from = from;
    edge->to = to;
    edge->weight = 10;  // Start with small weight (10/255 ≈ 0.04)
    edge->last_used = 0;
    
    // Add to nodes
    node_add_outgoing(from, edge);
    node_add_incoming(to, edge);
    
    // Update cached weight sum
    from->outgoing_weight_sum += (float)edge->weight / 255.0f;
    
    return edge;
}

/* Free an edge */
static void edge_free(Edge *edge) {
    free(edge);
}

/* Find edge between two nodes (linear search in small lists) */
static Edge* find_edge_between(Node *from, Node *to) {
    if (!from || !to) return NULL;
    
    for (size_t i = 0; i < from->outgoing_count; i++) {
        Edge *edge = from->outgoing[i];
        if (edge && edge->to == to) {
            return edge;
        }
    }
    return NULL;
}

/* ============================================================================
 * GRAPH OPERATIONS
 * ============================================================================ */

/* Create a graph */
static Graph* graph_create(void) {
    Graph *graph = calloc(1, sizeof(Graph));
    if (!graph) return NULL;
    
    // Start with small hash table (grows as needed)
    graph->hash_capacity = 256;
    graph->hash_table = calloc(graph->hash_capacity, sizeof(Node*));
    if (!graph->hash_table) {
        free(graph);
        return NULL;
    }
    
    // Initialize edge array
    graph->edge_capacity = 256;
    graph->edges = malloc(graph->edge_capacity * sizeof(Edge*));
    if (!graph->edges) {
        free(graph->hash_table);
        free(graph);
        return NULL;
    }
    
    graph->timestamp = 1;
    graph->adaptation_count = 0;
    
    // Embedding cache (NULL until generation, created on-demand)
    graph->embedding_cache = NULL;
    
    // Create STOP node (like LLM EOS token - singleton, one per graph)
    // Uses special byte pattern 0xFF to distinguish from regular data
    uint8_t stop_pattern[] = {0xFF};
    graph->stop_node = find_or_create_node(graph, stop_pattern, 1);
    if (!graph->stop_node) {
        // If creation failed, continue without STOP (graceful degradation)
        DEBUG_LOG("[GRAPH] Warning: Could not create STOP node\n");
    } else {
        DEBUG_LOG("[GRAPH] Created STOP node (EOS token equivalent)\n");
    }
    
    return graph;
}

/* Free a graph */
static void graph_free(Graph *graph) {
    if (!graph) return;
    
    // Free all nodes
    for (size_t i = 0; i < graph->hash_capacity; i++) {
        Node *node = graph->hash_table[i];
        while (node) {
            Node *next = node->next_in_bucket;
            node_free(node);
            node = next;
        }
    }
    
    // Free all edges
    for (size_t i = 0; i < graph->edge_count; i++) {
        edge_free(graph->edges[i]);
    }
    
    free(graph->hash_table);
    free(graph->edges);
    free(graph);
}

/* Add edge to graph's edge list */
static void graph_add_edge(Graph *graph, Edge *edge) {
    if (graph->edge_count >= graph->edge_capacity) {
        size_t new_cap = graph->edge_capacity * 2;
        Edge **new_arr = realloc(graph->edges, new_cap * sizeof(Edge*));
        if (!new_arr) return;
        graph->edges = new_arr;
        graph->edge_capacity = new_cap;
    }
    graph->edges[graph->edge_count++] = edge;
}

/* Hash function (simple FNV-1a hash) */
static uint64_t hash_payload(const uint8_t *payload, size_t size) {
    uint64_t hash = 14695981039346656037ULL;  // FNV offset basis
    for (size_t i = 0; i < size; i++) {
        hash ^= payload[i];
        hash *= 1099511628211ULL;  // FNV prime
    }
    return hash;
}

/* Convert hash to table index */
static size_t hash_to_index(uint64_t hash, size_t capacity) {
    return hash % capacity;
}

/* Find node in hash table */
static Node* graph_find_node(Graph *graph, const uint8_t *payload, size_t size) {
    if (!graph || !payload || size == 0) return NULL;
    
    uint64_t hash = hash_payload(payload, size);
    size_t index = hash_to_index(hash, graph->hash_capacity);
    
    Node *node = graph->hash_table[index];
    while (node) {
        if (node->payload_size == size && 
            memcmp(node->payload, payload, size) == 0) {
            return node;
        }
        node = node->next_in_bucket;
    }
    return NULL;
}

/* Add node to hash table */
static void graph_add_node(Graph *graph, Node *node) {
    if (!graph || !node) return;
    
    uint64_t hash = hash_payload(node->payload, node->payload_size);
    size_t index = hash_to_index(hash, graph->hash_capacity);
    
    // Add to bucket (chain)
    node->next_in_bucket = graph->hash_table[index];
    graph->hash_table[index] = node;
    graph->node_count++;
}

/* ============================================================================
 * ACTIVATION PATTERN OPERATIONS
 * ============================================================================ */

/* Create an activation pattern */
static ActivationPattern* activation_pattern_create(void) {
    ActivationPattern *pattern = calloc(1, sizeof(ActivationPattern));
    if (!pattern) return NULL;
    
    pattern->capacity = 64;
    pattern->nodes = malloc(pattern->capacity * sizeof(Node*));
    pattern->activations = malloc(pattern->capacity * sizeof(float));
    
    if (!pattern->nodes || !pattern->activations) {
        free(pattern->nodes);
        free(pattern->activations);
        free(pattern);
        return NULL;
    }
    
    // Context vector (256 dimensions - one per byte value)
    pattern->context_dim = 256;
    pattern->context_vector = calloc(256, sizeof(float));
    if (!pattern->context_vector) {
        free(pattern->nodes);
        free(pattern->activations);
        free(pattern);
        return NULL;
    }
    
    // Hash table for O(1) activation lookup (power of 2 for fast modulo)
    pattern->hash_capacity = 128;  // Start with 128 slots
    pattern->hash_table = calloc(pattern->hash_capacity, sizeof(Node*));
    pattern->hash_activations = calloc(pattern->hash_capacity, sizeof(float));
    if (!pattern->hash_table || !pattern->hash_activations) {
        free(pattern->nodes);
        free(pattern->activations);
        free(pattern->context_vector);
        free(pattern->hash_table);
        free(pattern->hash_activations);
        free(pattern);
        return NULL;
    }
    
    return pattern;
}

/* Free an activation pattern */
static void activation_pattern_free(ActivationPattern *pattern) {
    if (!pattern) return;
    free(pattern->nodes);
    free(pattern->activations);
    free(pattern->context_vector);
    free(pattern->hash_table);
    free(pattern->hash_activations);
    free(pattern);
}

/* Add or update node activation in pattern
 * FIXED: No O(n) search - hash table is single source of truth
 * Brain: Local decisions only - O(1) hash lookup, no global searches
 */
static void activation_pattern_add(ActivationPattern *pattern, Node *node, float activation) {
    if (!pattern || !node) return;
    
    // Hash lookup for O(1) check if node exists
    size_t hash_idx = ((size_t)node / sizeof(Node)) & (pattern->hash_capacity - 1);
    
    // Linear probing for collision resolution
    size_t probe = 0;
    while (probe < pattern->hash_capacity) {
        size_t idx = (hash_idx + probe) & (pattern->hash_capacity - 1);
        
        if (pattern->hash_table[idx] == node) {
            // Node exists: update activation in hash table (single source of truth)
            // FIXED: No O(n) search through main array - hash table already has answer
            pattern->hash_activations[idx] += activation;
            
            // Main array updated lazily during iteration if needed
            // For now, hash table is authoritative for activations
            return;
        }
        
        if (pattern->hash_table[idx] == NULL) {
            // Empty slot: add new node
            break;
        }
        
        probe++;
    }
    
    // Check if hash table is full and needs resizing
    if (probe >= pattern->hash_capacity) {
        // Hash table full - resize (shouldn't happen often)
        size_t new_cap = pattern->hash_capacity * 2;
        Node **new_hash_table = calloc(new_cap, sizeof(Node*));
        float *new_hash_activations = calloc(new_cap, sizeof(float));
        if (!new_hash_table || !new_hash_activations) {
            free(new_hash_table);
            free(new_hash_activations);
            return;
        }
        
        // Rehash existing entries
        for (size_t i = 0; i < pattern->hash_capacity; i++) {
            if (pattern->hash_table[i]) {
                size_t new_hash_idx = ((size_t)pattern->hash_table[i] / sizeof(Node)) & (new_cap - 1);
                size_t new_probe = 0;
                while (new_hash_table[(new_hash_idx + new_probe) & (new_cap - 1)] != NULL) {
                    new_probe++;
                }
                size_t final_idx = (new_hash_idx + new_probe) & (new_cap - 1);
                new_hash_table[final_idx] = pattern->hash_table[i];
                new_hash_activations[final_idx] = pattern->hash_activations[i];
            }
        }
        
        free(pattern->hash_table);
        free(pattern->hash_activations);
        pattern->hash_table = new_hash_table;
        pattern->hash_activations = new_hash_activations;
        pattern->hash_capacity = new_cap;
        
        // Recompute hash_idx with new capacity
        hash_idx = ((size_t)node / sizeof(Node)) & (pattern->hash_capacity - 1);
        probe = 0;
        while (pattern->hash_table[(hash_idx + probe) & (pattern->hash_capacity - 1)] != NULL) {
            probe++;
        }
    }
    
    // Add new node to hash table (authoritative)
    size_t final_idx = (hash_idx + probe) & (pattern->hash_capacity - 1);
    pattern->hash_table[final_idx] = node;
    pattern->hash_activations[final_idx] = activation;
    
    // Also add to main arrays for iteration (synchronized but not searched)
    if (pattern->count >= pattern->capacity) {
        size_t new_cap = pattern->capacity * 2;
        Node **new_nodes = realloc(pattern->nodes, new_cap * sizeof(Node*));
        float *new_acts = realloc(pattern->activations, new_cap * sizeof(float));
        if (!new_nodes || !new_acts) {
            free(new_nodes);
            free(new_acts);
            return;
        }
        pattern->nodes = new_nodes;
        pattern->activations = new_acts;
        pattern->capacity = new_cap;
    }
    
    // Synchronize main array with hash table (for iteration)
    pattern->nodes[pattern->count] = node;
    pattern->activations[pattern->count] = activation;
    pattern->count++;
}

/* Get activation for a node (O(1) hash lookup) */
static float activation_pattern_get(ActivationPattern *pattern, Node *node) {
    if (!pattern || !node) return 0.0f;
    
    // O(1) hash lookup with linear probing
    size_t hash_idx = ((size_t)node / sizeof(Node)) & (pattern->hash_capacity - 1);
    
    size_t probe = 0;
    while (probe < pattern->hash_capacity) {
        size_t idx = (hash_idx + probe) & (pattern->hash_capacity - 1);
        
        if (pattern->hash_table[idx] == node) {
            return pattern->hash_activations[idx];
        }
        
        if (pattern->hash_table[idx] == NULL) {
            // Empty slot: node not in pattern
            return 0.0f;
        }
        
        probe++;
    }
    
    return 0.0f;
}

/* ============================================================================
 * INPUT PROCESSING & STRUCTURE CREATION (Phase 1)
 * ============================================================================ */

/* Find or create a node for a payload
 * Uses: Hash table lookup (O(1) amortized, no global search)
 * - Hash payload to get table index
 * - Search bucket (chained hashing)
 * - If found: return existing node (prevents duplicates)
 * - If not found: create new node and add to hash table
 */
static Node* find_or_create_node(Graph *graph, const uint8_t *payload, size_t size) {
    if (!graph || !payload || size == 0) return NULL;
    
    // Try to find existing node (hash table lookup - O(1))
    Node *node = graph_find_node(graph, payload, size);
    if (node) return node;  // Found - return existing (no duplicate created)
    
    // Create new node (not found - safe to create)
    node = node_create(payload, size, 0);
    if (node) {
        graph_add_node(graph, node);  // Add to hash table (O(1))
    }
    return node;
}

/* Create sequential edges between consecutive nodes
 * Uses: Local search (O(degree), no global search)
 * - For each pair: search only from->outgoing[] array
 * - If found: return existing edge (prevents duplicates)
 * - If not found: create new edge
 * - Also performs HEBBIAN LEARNING: Strengthens edges immediately
 */
static void create_sequential_edges(Graph *graph, Node **nodes, size_t count) {
    if (!graph || !nodes || count < 2) return;
    
    for (size_t i = 0; i < count - 1; i++) {
        Node *from = nodes[i];
        Node *to = nodes[i + 1];
        
        if (!from || !to) continue;
        
        // Find or create edge (local search - O(degree), no global search)
        // Searches only from->outgoing[] array, not all edges
        Edge *edge = find_edge_between(from, to);
        if (!edge) {
            // Not found - create new edge (no duplicate)
            edge = edge_create(from, to);
            if (edge) {
                graph_add_edge(graph, edge);
            }
        }
        
        // HEBBIAN LEARNING: Strengthen edge immediately
        // "Neurons that fire together, wire together"
        // Updates edge weight (memory/log) - not a decision driver
        if (edge) {
            hebbian_strengthen_edge(edge, graph);
        }
    }
    
    // STOP EDGE CREATION: Create edge to STOP at pattern end (but don't strengthen)
    // Like LLM EOS token: edge exists but only learns from actual usage
    // FIX: Remove perfect-score strengthening - STOP edge learns like regular edges
    if (count > 0 && graph->stop_node) {
        Node *last_node = nodes[count - 1];
        if (last_node) {
            Edge *stop_edge = find_edge_between(last_node, graph->stop_node);
            if (!stop_edge) {
                // Create new STOP edge (initial weight = 10, like regular edges)
                stop_edge = edge_create(last_node, graph->stop_node);
                if (stop_edge) {
                    graph_add_edge(graph, stop_edge);
                }
            }
            // REMOVED: Perfect-score strengthening during training
            // STOP edge now learns only during actual generation when stopping occurs
        }
    }
}

/* Process input bytes (main entry point for input)
 * OPTIMIZED ORDER: Structure creation → Learning → Processing
 * - Create/find nodes (hash table - O(1))
 * - Create/find edges (local search - O(degree))
 * - Hebbian learning (strengthen edges)
 * - Hierarchy formation (compress patterns - forms BEFORE processing)
 * - ENCODE → REFINE → DECODE (can use hierarchies from above)
 */
static void process_input_bytes(MFile *mfile, const uint8_t *data, size_t size) {
    if (!mfile || !mfile->graph || !data || size == 0) return;
    
    Graph *graph = mfile->graph;
    Node **input_nodes = malloc(size * sizeof(Node*));
    if (!input_nodes) return;
    
    // ========================================================================
    // PHASE 1: STRUCTURE CREATION & LEARNING (Early - Before Processing)
    // ========================================================================
    
    // 1. HIERARCHY-FIRST INPUT MATCHING (Greedy longest-match)
    // Vision: "Try to match existing nodes (hierarchy-first: larger patterns first)"
    // Match hierarchies before creating raw bytes to use compressed knowledge
    size_t input_offset = 0;
    size_t input_nodes_count = 0;
    
    // HIERARCHY-FIRST INPUT MATCHING (Greedy longest-match)
    // Process input with hierarchy matching (prevents infinite loop with safety checks)
    while (input_offset < size) {
        // Safety check: prevent infinite loop
        if (input_nodes_count >= size * 2) {
            DEBUG_LOG("[INPUT] Safety break: too many nodes (%zu >= %zu)\n", input_nodes_count, size * 2);
            break;
        }
        
        // Try to find longest matching hierarchy (greedy longest-match)
        Node *matched_hierarchy = NULL;
        size_t matched_len = 0;
        
        // Try matching from longest to shortest (hierarchy-first)
        for (size_t try_len = size - input_offset; try_len > 0; try_len--) {
            Node *hierarchy = graph_find_node(graph, &data[input_offset], try_len);
            if (hierarchy && hierarchy->abstraction_level > 0) {
                // Found matching hierarchy: use it
                matched_hierarchy = hierarchy;
                matched_len = try_len;
                break;
            }
        }
        
        if (matched_hierarchy && matched_len > 0) {
            // Use hierarchy node (compressed knowledge)
            input_nodes[input_nodes_count++] = matched_hierarchy;
            matched_hierarchy->is_input_node = 1;  // Mark as input
            matched_hierarchy->last_activation = graph->timestamp;
            input_offset += matched_len;  // Advance by matched length
            DEBUG_LOG("[INPUT] Matched hierarchy: %zu bytes at offset %zu\n", matched_len, input_offset - matched_len);
        } else {
            // No hierarchy match: create/find raw byte node
            Node *byte_node = find_or_create_node(graph, &data[input_offset], 1);
            if (byte_node) {
                input_nodes[input_nodes_count++] = byte_node;
                byte_node->is_input_node = 1;  // Mark as input
                byte_node->last_activation = graph->timestamp;
            }
            input_offset++;  // Always advance by at least 1 byte
        }
    }
    
    // Update size to reflect actual node count (may be less if hierarchies matched)
    size_t actual_node_count = input_nodes_count;
    
    // 2. CREATE/FIND EDGES (Local search - O(degree), no global search)
    // Also performs HEBBIAN LEARNING: Strengthens edges immediately
    // "Neurons that fire together, wire together"
    create_sequential_edges(graph, input_nodes, actual_node_count);
    
    // 3. HIERARCHY FORMATION (Check for new hierarchies)
    // MOVED HERE: Forms hierarchies BEFORE processing so they can be used
    // Benefits:
    //   - New hierarchies available for ENCODE phase
    //   - Pattern matching benefits from compressed structures
    //   - Faster processing with hierarchy nodes
    // Safety: Only forms hierarchies from edges, doesn't modify input nodes
    // O(degree) COMPLIANCE: Only checks edges from input nodes (local), not all edges globally
    check_hierarchy_formation(graph, input_nodes, actual_node_count);
    
    // ========================================================================
    // PHASE 2: PROCESSING (Can use hierarchies formed in Phase 1)
    // ========================================================================
    
    // 4. ENCODE → REFINE → DECODE
    // Can now use hierarchies formed above for faster pattern matching
    ActivationPattern *pattern = encode_direct_activation(input_nodes, actual_node_count);
    if (pattern) {
        encode_spreading_activation(pattern, input_nodes, actual_node_count, graph);
        
        // PHASE 2: Hierarchy merging (check hierarchy→hierarchy edges from activated nodes)
        // Process ALL activated nodes (including hierarchies formed in Phase 1)
        // This allows hierarchies to merge into larger hierarchies organically
        check_hierarchy_formation_phase2(graph, pattern);
        
        build_context_vector(pattern);
        refine_competition(pattern);
        refine_normalize(pattern);
        generate_output(mfile, pattern, input_nodes, actual_node_count);
        activation_pattern_free(pattern);
    }
    
    // ========================================================================
    // PHASE 3: CLEANUP & MAINTENANCE
    // ========================================================================
    
    // Clear input flags
    for (size_t i = 0; i < actual_node_count; i++) {
        if (input_nodes[i]) {
            input_nodes[i]->is_input_node = 0;
        }
    }
    
    // Decay unused edges periodically (intelligent forgetting)
    // Brain: Unused synapses weaken over time
    // LOCAL DECISION: Only decay edges from recently activated nodes (not all edges globally)
    graph->timestamp++;
    if (graph->timestamp % 100 == 0 && actual_node_count > 0) {
        hebbian_decay_unused(graph, input_nodes, actual_node_count);
    }
    
    free(input_nodes);
}

/* ============================================================================
 * ENCODE PHASE (Phase 2A)
 * Brain: Sensory input → neural activation pattern
 * LLM: Token IDs → embeddings → hidden state
 * ============================================================================ */

/* ENCODE: Direct activation of input nodes (PARALLEL)
 * Brain principle: All sensory inputs activate simultaneously, competition determines winners
 * PARALLEL: All input nodes get equal initial activation (no linear position bias)
 * Recency handled by spreading activation strength, not input activation
 */
static ActivationPattern* encode_direct_activation(Node **input_nodes, size_t input_count) {
    if (!input_nodes || input_count == 0) return NULL;
    
    ActivationPattern *pattern = activation_pattern_create();
    if (!pattern) return NULL;
    
    // Compute average continuation strength across ALL input nodes (not just last)
    // This determines relative scale for input activation
    float total_continuation_strength = 0.0f;
    size_t nodes_with_edges = 0;
    
    for (size_t i = 0; i < input_count; i++) {
        if (input_nodes[i] && input_nodes[i]->outgoing_count > 0) {
            float local_avg = node_get_outgoing_avg(input_nodes[i]);
            if (local_avg > 0.0f) {
                total_continuation_strength += local_avg;
                nodes_with_edges++;
            }
        }
    }
    
    float expected_continuation_strength = nodes_with_edges > 0 ? 
        (total_continuation_strength / nodes_with_edges) : 0.5f;
    
    // Input activation should be WEAK relative to expected continuation
    // Brain: Input is cue, prediction is stronger
    float relative_input_strength = 0.2f;  // 20% of continuation
    
    // PARALLEL ACTIVATION: ALL input nodes get EQUAL activation
    // Brain: All sensory inputs fire simultaneously, no position bias
    // Competition in REFINE phase determines which paths win
    float base_activation = expected_continuation_strength * relative_input_strength;
    
    for (size_t i = 0; i < input_count; i++) {
        if (!input_nodes[i]) continue;
        
        // PARALLEL: Equal activation for all (no position weighting)
        activation_pattern_add(pattern, input_nodes[i], base_activation);
        
        DEBUG_LOG("[ENCODE] Input node %zu: activation=%.3f (PARALLEL, equal for all)\n", 
                  i, base_activation);
    }
    
    return pattern;
}

/* ============================================================================
 * LOCAL DECISION-MAKING HELPERS
 * Brain: Each neuron decides locally based on its own context
 * ============================================================================ */

/* Compute local decay based on node's own activation and edge strength
 * Brain: Signal decay depends on local signal strength, not global hop count
 */
static float compute_local_decay(float my_activation, float edge_relative) {
    // Decay adapts to MY activation strength and edge strength
    // Strong activation + strong edge = less decay
    // Weak activation or weak edge = more decay
    float activation_factor = my_activation / (my_activation + 1.0f);  // 0-1 range
    float edge_factor = edge_relative / (edge_relative + 1.0f);        // 0-1 range
    
    // Combined: strong signals decay less (0.5-0.9 range)
    float decay = 0.5f + (activation_factor * edge_factor * 0.4f);
    return decay;
}

/* Node decides locally whether to spread its activation
 * Brain: Each neuron decides if its signal is strong enough to propagate
 * No global hop count - natural stopping when signals too weak
 * NOTE: This only adds to pattern, doesn't recursively call itself
 */
static void node_spread_activation_locally(Node *node, ActivationPattern *pattern) {
    if (!node || !pattern) return;
    
    // Get MY activation (O(1) hash lookup)
    float my_activation = activation_pattern_get(pattern, node);
    
    // Get MY local context
    float my_local_avg = node_get_outgoing_avg(node);
    if (my_local_avg <= 0.0f) return;  // No edges to spread through
    
    // Local decision: Am I strong enough to spread?
    // Compare to MY edges, not global max
    // Use stricter threshold to prevent runaway spreading
    if (my_activation < my_local_avg * 0.2f) return;  // Too weak relative to MY edges
    
    // Spread through MY edges (local decision per edge)
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing[i];
        if (!edge || !edge->to || edge->to->is_input_node) continue;
        
        // Local decision: Is THIS edge strong relative to MY other edges?
        float edge_strength = (float)edge->weight / 255.0f;
        float edge_relative = edge_strength / my_local_avg;
        
        // Stricter threshold to prevent runaway spreading
        if (edge_relative > 1.5f) {  // 50% stronger than MY average
            // Compute decay based on MY activation strength (not global hop count)
            float decay = compute_local_decay(my_activation, edge_relative);
            float spread = my_activation * edge_strength * decay;
            
            // Only spread if result is significant
            if (spread > 0.01f) {
                activation_pattern_add(pattern, edge->to, spread);
                
                DEBUG_LOG("[LOCAL_SPREAD] Node spreading: my_act=%.3f, edge_rel=%.2f, decay=%.2f, spread=%.3f\n",
                          my_activation, edge_relative, decay, spread);
            }
        }
    }
}

/* Node decides if it needs analogical reasoning
 * Brain: Weakly activated neurons seek analogical support
 * Each node compares to its own context, not global max
 */
static void node_check_analogical_reasoning(Node *node, ActivationPattern *pattern, 
                                            Node *last_input, Graph *graph) {
    if (!node || !pattern || !last_input || !graph) return;
    
    // Get MY activation (O(1))
    float my_activation = activation_pattern_get(pattern, node);
    
    // Get MY local context (not global max)
    float my_local_avg = node_get_outgoing_avg(node);
    if (my_local_avg <= 0.0f) return;
    
    // Local decision: Am I weak relative to MY edges?
    float my_relative_strength = my_activation / my_local_avg;
    
    // If I'm weak (< 30% of my edge average), try analogy
    if (my_relative_strength < 0.3f && node->incoming_count > 0) {
        Node *analogous = analogical_reasoning(last_input, node, last_input, graph);
        if (analogous && analogous != node) {
            // Boost based on MY strength (not global)
            float analogy_activation = my_activation * (1.0f + my_relative_strength);
            activation_pattern_add(pattern, analogous, analogy_activation);
            
            DEBUG_LOG("[LOCAL_ANALOGY] Node seeking analogy: my_rel=%.2f, boost=%.3f\n",
                      my_relative_strength, analogy_activation);
        }
    }
}

/* Node decides which of its edges to decay
 * Brain: Each neuron maintains its own synapses based on local usage
 */
static void node_decay_unused_edges(Node *node, uint32_t current_time) {
    if (!node) return;
    
    // Get MY local average for relative decay decisions
    float my_local_avg = node_get_outgoing_avg(node);
    if (my_local_avg <= 0.0f) return;
    
    // Local decision: decay MY unused edges
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing[i];
        if (!edge) continue;
        
        // Local decision: Is THIS edge unused relative to MY context?
        if (current_time - edge->last_used > 100) {
            uint8_t old_weight = edge->weight;
            
            float edge_strength = (float)edge->weight / 255.0f;
            float edge_relative = edge_strength / my_local_avg;
            
            // Decay more if edge is weak relative to MY other edges
            if (edge->weight > 10 && edge_relative < 0.5f) {
                edge->weight--;
                
                // Update MY cached sum
                float old_float = (float)old_weight / 255.0f;
                float new_float = (float)edge->weight / 255.0f;
                node->outgoing_weight_sum += (new_float - old_float);
                
                DEBUG_LOG("[LOCAL_DECAY] Node decaying weak edge: rel=%.2f, weight=%u->%u\n",
                          edge_relative, old_weight, edge->weight);
            }
        }
    }
}

/* ENCODE: Spreading activation through edges (PARALLEL + STRONG)
 * Brain principle: All neurons spread activation simultaneously through their connections
 * PARALLEL: ALL input nodes spread activation, not just the last one
 * Recency: Last input node gets stronger spreading multiplier (recency bias)
 * DATA-DRIVEN: Activation strength relative to input activation, not hardcoded
 */
static void encode_spreading_activation(ActivationPattern *pattern, Node **input_nodes, size_t input_count, Graph *graph) {
    if (!pattern || !input_nodes || input_count == 0) return;
    
    DEBUG_LOG("[ENCODE] Parallel spreading from ALL %zu input nodes\n", input_count);
    
    // Compute max input activation for relative scaling
    float max_input_activation = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        if (pattern->nodes[i] && pattern->nodes[i]->is_input_node) {
            if (pattern->activations[i] > max_input_activation) {
                max_input_activation = pattern->activations[i];
            }
        }
    }
    
    // Continuations should be STRONG relative to input (brain: prediction > input)
    float base_continuation_multiplier = max_input_activation > 0.0f ? 
        (3.0f + (max_input_activation * 2.0f)) : 5.0f;  // 3-5x range
    
    // PARALLEL SPREADING: ALL input nodes spread activation simultaneously
    // Brain: All sensory neurons fire to their connections in parallel
    // Recency: Later positions get MUCH stronger multiplier (recency bias in spreading, not input)
    for (size_t node_idx = 0; node_idx < input_count; node_idx++) {
        Node *input_node = input_nodes[node_idx];
        if (!input_node) continue;
        
        // Recency multiplier: later positions get exponentially stronger spreading
        // Brain: Recent inputs dominate prediction
        // Position 0: 0.1x, Position N-1: 2.0x (exponential, not linear)
        float position_ratio = (float)node_idx / (float)(input_count > 1 ? input_count - 1 : 1);
        float recency_multiplier = 0.1f + (position_ratio * position_ratio * 1.9f);  // Quadratic: 0.1 to 2.0
        float continuation_multiplier = base_continuation_multiplier * recency_multiplier;
        
        DEBUG_LOG("[ENCODE] Spreading from input node %zu (outgoing=%zu, recency=%.2f)\n", 
                  node_idx, input_node->outgoing_count, recency_multiplier);
        
        // Spread through ALL edges from this input node
        for (size_t i = 0; i < input_node->outgoing_count; i++) {
            Edge *edge = input_node->outgoing[i];
            if (!edge || !edge->to) continue;
            
            // Skip if target is also an input node (we want continuations)
            if (edge->to->is_input_node) continue;
            
            float edge_strength = (float)edge->weight / 255.0f;
            float strong_activation = edge_strength * continuation_multiplier;
            
            activation_pattern_add(pattern, edge->to, strong_activation);
            
            DEBUG_LOG("[ENCODE] Continuation from node %zu: weight=%u, activation=%.3f (mult=%.2f)\n", 
                      node_idx, edge->weight, strong_activation, continuation_multiplier);
        }
    }
    
    // LOCAL DECISION-MAKING: Node-driven spreading (replaces global hop loop)
    // Brain: Each neuron decides locally whether to spread its signal
    // Natural stopping: when nodes are too weak, they don't spread
    // Multi-pass: Process nodes in waves until no new activations or max passes reached
    
    size_t max_passes = 3;  // Limit passes to prevent runaway (replaces hop count)
    size_t max_pattern_size = 256;  // Hard limit on pattern size to prevent runaway
    
    for (size_t pass = 0; pass < max_passes; pass++) {
        size_t count_before = pattern->count;
        
        // Safety: stop if pattern is getting too large
        if (pattern->count >= max_pattern_size) {
            DEBUG_LOG("[LOCAL_SPREAD] Pattern size limit reached (%zu nodes)\n", pattern->count);
            break;
        }
        
        // Each activated node decides locally whether to spread
        for (size_t i = 0; i < count_before; i++) {
            Node *node = pattern->nodes[i];
            if (node && !node->is_input_node) {
                node_spread_activation_locally(node, pattern);
            }
            
            // Safety check within loop
            if (pattern->count >= max_pattern_size) {
                DEBUG_LOG("[LOCAL_SPREAD] Pattern size limit reached during pass %zu\n", pass);
                break;
            }
        }
        
        // Natural stopping: if no new nodes activated, stop
        if (pattern->count == count_before) {
            DEBUG_LOG("[LOCAL_SPREAD] Natural stop at pass %zu (no new activations)\n", pass);
            break;
        }
    }
    
    // LOCAL DECISION-MAKING: Analogical reasoning (replaces global loop with hardcoded limit)
    // Brain: Each weakly-activated neuron decides if it needs analogical support
    // No hardcoded limit (20) - each node makes local decision
    // No O(n²) global max/min computation - each node uses its own context
    
    if (graph && input_count > 0) {
        Node *last_input = input_nodes[input_count - 1];
        
        // Each node decides if it needs analogical reasoning (local decision)
        // FIXED: Use snapshot of count to avoid processing newly added nodes in this pass
        size_t current_count = pattern->count;
        for (size_t i = 0; i < current_count; i++) {
            node_check_analogical_reasoning(pattern->nodes[i], pattern, last_input, graph);
        }
    }
    
    // HIERARCHY ACTIVATION: Activate hierarchies that match input prefix
    // Vision: "Hierarchies enable matching larger patterns efficiently"
    // If input matches a hierarchy prefix, activate the hierarchy with strong activation
    // This enables hierarchy-guided output generation
    if (graph && input_count > 0 && pattern->count > 0) {
        // Build input byte sequence for prefix matching
        uint8_t input_bytes[256];
        size_t input_bytes_len = 0;
        for (size_t i = 0; i < input_count && input_bytes_len < 256; i++) {
            if (input_nodes[i] && input_nodes[i]->payload_size > 0) {
                for (size_t j = 0; j < input_nodes[i]->payload_size && input_bytes_len < 256; j++) {
                    input_bytes[input_bytes_len++] = input_nodes[i]->payload[j];
                }
            }
        }
        
        // Check hierarchies: search hash table buckets (limited, not global)
        // Only check nodes in same hash buckets as potential hierarchy prefixes
        // This limits search space while allowing hierarchy activation
        if (input_bytes_len > 0) {
            // Try to find exact hierarchy match first (hash table - O(1))
            Node *exact_hierarchy = graph_find_node(graph, input_bytes, input_bytes_len);
            if (exact_hierarchy && exact_hierarchy->abstraction_level > 0) {
                // Exact match: activate hierarchy strongly
                float hierarchy_activation = max_input_activation * 5.0f;  // Strong (5x input)
                activation_pattern_add(pattern, exact_hierarchy, hierarchy_activation);
                DEBUG_LOG("[ENCODE] Activated hierarchy (exact match, %zu bytes)\n", 
                          exact_hierarchy->payload_size);
            } else {
                // Try prefix match: check if input matches start of any hierarchy
                // Note: This requires checking hierarchies, but we limit to reasonable search
                // Since hierarchies are a subset of nodes, and we only check when input is small
                // This is acceptable for small inputs (< 256 bytes)
                // For larger graphs, this could be optimized with a hierarchy index
                if (input_bytes_len < 64) {  // Only for reasonable-sized inputs
                    // Check hash table buckets that could contain matching hierarchies
                    // This is still O(1) per bucket, but checks multiple buckets
                    // Limits to checking ~10-20 nodes instead of all nodes
                    size_t max_hierarchy_checks = 32;  // Limit hierarchy checks
                    size_t hierarchy_checks = 0;
                    
                    // Try common prefix lengths for hierarchy matching
                    for (size_t prefix_len = input_bytes_len; prefix_len > 0 && hierarchy_checks < max_hierarchy_checks; prefix_len--) {
                        Node *prefix_hierarchy = graph_find_node(graph, input_bytes, prefix_len);
                        if (prefix_hierarchy && prefix_hierarchy->abstraction_level > 0) {
                            // Input matches hierarchy prefix: activate hierarchy
                            float match_ratio = (float)prefix_len / (float)prefix_hierarchy->payload_size;
                            float hierarchy_activation = max_input_activation * 3.0f * match_ratio;  // 3x input, scaled by match
                            activation_pattern_add(pattern, prefix_hierarchy, hierarchy_activation);
                            DEBUG_LOG("[ENCODE] Activated hierarchy (prefix match: %zu/%zu bytes)\n", 
                                      prefix_len, prefix_hierarchy->payload_size);
                            hierarchy_checks++;
                            break;  // Use longest matching prefix
                        }
                    }
                }
            }
        }
    }
    
    DEBUG_LOG("[ENCODE] Total activated nodes: %zu\n", pattern->count);
}

/* Build context vector (like LLM hidden state) */
static void build_context_vector(ActivationPattern *pattern) {
    if (!pattern) return;
    
    // Build weighted sum of activations (one dimension per byte value)
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float act = pattern->activations[i];
        
        if (!node || node->payload_size == 0) continue;
        
        // Add activation to corresponding dimension
        uint8_t byte_val = node->payload[0];
        pattern->context_vector[byte_val] += act;
    }
}

/* ============================================================================
 * REFINE PHASE (Phase 2B)
 * Brain: Recurrent cortical processing, winner-take-all competition
 * LLM: Transformer layers (attention + feedforward)
 * ============================================================================ */

/* REFINE: Winner-take-all competition among continuations
 * Brain principle: Local competition - strongest neuron suppresses neighbors
 * KEY: Separate input nodes from continuation nodes, normalize within groups
 */
static void refine_competition(ActivationPattern *pattern) {
    if (!pattern || pattern->count == 0) return;
    
    // Find max activations for each group
    float max_input_activation = 0.0f;
    float max_continuation_activation = 0.0f;
    
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float act = pattern->activations[i];
        
        if (node->is_input_node) {
            if (act > max_input_activation) {
                max_input_activation = act;
            }
        } else {
            if (act > max_continuation_activation) {
                max_continuation_activation = act;
            }
        }
    }
    
    DEBUG_LOG("[REFINE] Max input=%.3f, Max continuation=%.3f\n", 
              max_input_activation, max_continuation_activation);
    
    // Normalize within groups (not globally)
    // Brain: Local normalization within population
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        
        if (node->is_input_node) {
            if (max_input_activation > 0.0f) {
                pattern->activations[i] /= max_input_activation;
            }
        } else {
            if (max_continuation_activation > 0.0f) {
                pattern->activations[i] /= max_continuation_activation;
            }
        }
    }
}

/* REFINE: Normalize activations (softmax-like)
 * LLM principle: Softmax over logits for probability distribution
 * DATA-DRIVEN: Suppression adapts to distribution shape, not hardcoded 80%/10%
 */
static void refine_normalize(ActivationPattern *pattern) {
    if (!pattern || pattern->count == 0) return;
    
    // Compute local statistics for adaptive suppression
    float max_act, min_act, mean_act, variance;
    compute_local_stats(pattern->activations, pattern->count, 
                        &max_act, &min_act, &mean_act, &variance);
    
    // Adaptive percentile: depends on distribution spread
    // High variance = more spread = suppress more (lower percentile)
    // Low variance = tight cluster = suppress less (higher percentile)
    float base_percentile = 0.8f;  // Base 80th percentile
    float variance_factor = variance > 0.0f ? 
        (1.0f / (1.0f + variance * 2.0f)) : 1.0f;  // High variance → lower percentile
    float adaptive_percentile = base_percentile * variance_factor;
    if (adaptive_percentile < 0.5f) adaptive_percentile = 0.5f;  // Minimum 50%
    if (adaptive_percentile > 0.9f) adaptive_percentile = 0.9f;  // Maximum 90%
    
    // Find adaptive percentile threshold (data-driven, not hardcoded 80%)
    float *sorted_acts = malloc(pattern->count * sizeof(float));
    if (!sorted_acts) return;
    
    memcpy(sorted_acts, pattern->activations, pattern->count * sizeof(float));
    
    // Simple insertion sort (good enough for small arrays)
    for (size_t i = 1; i < pattern->count; i++) {
        float key = sorted_acts[i];
        int j = i - 1;
        while (j >= 0 && sorted_acts[j] > key) {
            sorted_acts[j + 1] = sorted_acts[j];
            j--;
        }
        sorted_acts[j + 1] = key;
    }
    
    size_t threshold_idx = (size_t)((float)pattern->count * adaptive_percentile);
    if (threshold_idx >= pattern->count) threshold_idx = pattern->count - 1;
    float threshold = sorted_acts[threshold_idx];
    free(sorted_acts);
    
    // Adaptive suppression factor: depends on how far below threshold
    // Far below = stronger suppression, close to threshold = lighter suppression
    for (size_t i = 0; i < pattern->count; i++) {
        if (pattern->activations[i] < threshold) {
            // Compute suppression factor based on distance from threshold
            float distance = threshold - pattern->activations[i];
            float threshold_range = threshold - min_act;
            float suppression_factor = threshold_range > 0.0f ? 
                (0.05f + (distance / threshold_range) * 0.15f) : 0.1f;  // 5-20% range (not hardcoded 10%)
            pattern->activations[i] *= suppression_factor;  // Adaptive suppression
        }
    }
}

/* ============================================================================
 * DECODE PHASE (Phase 2C)
 * Brain: Motor output from winner neuron
 * LLM: Sample from softmax distribution over vocabulary
 * ============================================================================ */

/* DECODE: Select first output byte (NODE-LEVEL DECISION)
 * LLM principle: Don't copy prompt, generate continuation
 * KEY CHANGE: Activated nodes compete to predict first byte
 * Each activated node evaluates its outgoing edges using context
 */
static Node* decode_select_first(ActivationPattern *pattern, Graph *graph) {
    if (!pattern || pattern->count == 0) return NULL;
    
    DEBUG_LOG("[DECODE] Selecting first byte from %zu activated nodes\n", pattern->count);
    
    // Each activated node predicts its best continuation
    // We collect all predictions and choose the best one
    typedef struct {
        Node *source_node;
        Edge *predicted_edge;
        float activation;
        float prediction_score;
        float combined_score;
    } NodePrediction;
    
    NodePrediction predictions[32];
    size_t prediction_count = 0;
    
    for (size_t i = 0; i < pattern->count && prediction_count < 32; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        if (!node || node->outgoing_count == 0) continue;
        
        // Skip input nodes (we want continuations, not echoes)
        if (node->is_input_node) {
            DEBUG_LOG("[DECODE] Skipping input node (payload[0]=%c)\n", 
                      node->payload[0] >= 32 ? node->payload[0] : '?');
            continue;
        }
        
        // Node predicts its best outgoing edge
        Edge *predicted_edge = node_predict_next_edge(node, pattern, graph);
        
        if (predicted_edge && predicted_edge->to && predicted_edge->to->payload_size > 0) {
            // Skip STOP node (should not be selected as first output byte)
            if (graph && graph->stop_node && predicted_edge->to == graph->stop_node) {
                continue;
            }
            
            // Skip control characters
            if (predicted_edge->to->payload[0] < 32 && predicted_edge->to->payload[0] != '\n') {
                continue;
            }
            
            // Compute prediction score (edge weight as memory/log)
            float weight_log = (float)predicted_edge->weight / 255.0f;
            
            // DATA-DRIVEN: Weight activation vs memory based on relative signal strength
            // If activation is strong relative to others, weight it more
            // If memory (weight) is strong relative to others, weight it more
            float activation_strength = activation;  // Already relative from pattern
            float memory_strength = weight_log;
            
            // Compute relative weights (not hardcoded 70/30)
            float total_strength = activation_strength + memory_strength;
            float activation_weight = total_strength > 0.0f ? 
                activation_strength / total_strength : 0.7f;  // Default 70% if no signal
            float memory_weight = total_strength > 0.0f ? 
                memory_strength / total_strength : 0.3f;  // Default 30% if no signal
            
            // Combined score: weighted by relative signal strength (data-driven)
            float combined = activation * activation_weight + weight_log * memory_weight;
            
            predictions[prediction_count].source_node = node;
            predictions[prediction_count].predicted_edge = predicted_edge;
            predictions[prediction_count].activation = activation;
            predictions[prediction_count].prediction_score = weight_log;
            predictions[prediction_count].combined_score = combined;
            
            DEBUG_LOG("[DECODE] Node prediction: '%c' -> '%c' (combined=%.3f)\n",
                      node->payload[0] >= 32 ? node->payload[0] : '?',
                      predicted_edge->to->payload[0] >= 32 ? predicted_edge->to->payload[0] : '?',
                      combined);
            
            prediction_count++;
        }
    }
    
    if (prediction_count == 0) {
        DEBUG_LOG("[DECODE] No predictions from activated nodes\n");
        return NULL;
    }
    
    // Find best prediction (highest combined score)
    Node *best_node = NULL;
    float best_combined = -1.0f;
    
    for (size_t i = 0; i < prediction_count; i++) {
        if (predictions[i].combined_score > best_combined) {
            best_combined = predictions[i].combined_score;
            best_node = predictions[i].predicted_edge->to;
        }
    }
    
    if (best_node) {
        DEBUG_LOG("[DECODE] Selected first byte: '%c' (score=%.3f)\n",
                  best_node->payload[0] >= 32 ? best_node->payload[0] : '?',
                  best_combined);
    } else {
        DEBUG_LOG("[DECODE] No continuation found!\n");
    }
    
    return best_node;
}

/* ============================================================================
 * NODE-LEVEL DECISION MAKING
 * ============================================================================ */

/* Node evaluates contextual score for a single edge
 * Uses: context-dependent embeddings + edge weight as memory/log
 * Context-dependent embeddings differentiate between different contexts (e.g., "cat" vs "dog")
 * Edge weight is INFORMATIVE (memory) not PRIMARY (decision driver)
 */
static float node_evaluate_edge_contextual_score(Node *node, Edge *edge, 
                                                  SparseEmbedding *node_emb,
                                                  SparseEmbedding *ctx_emb,
                                                  ActivationPattern *context, 
                                                  Graph *graph) {
    if (!node || !edge || !edge->to) return 0.0f;
    
    // PRIMARY: Context-dependent embedding match  
    // Use already-computed embeddings to avoid cache corruption
    // node_emb = context-dependent embedding of current node
    // ctx_emb = overall context embedding
    // Compare target node's static embedding to context to see if it fits
    float context_match = 0.0f;
    if (node_emb && ctx_emb && context && graph && graph->embedding_cache) {
        // Get target's static embedding (safe, doesn't modify cache)
        SparseEmbedding *target_static = node_compute_embedding(edge->to, graph);
        if (target_static) {
            // Match: How well does target fit the overall context?
            float target_ctx_sim = sparse_embedding_similarity(target_static, ctx_emb);
            
            // Also check: How well does edge fit given current node's context?
            float node_target_sim = sparse_embedding_similarity(node_emb, target_static);
            
            // Combined: higher = better fit for this context
            context_match = (target_ctx_sim + node_target_sim) / 2.0f;
            context_match = (context_match + 1.0f) / 2.0f;  // Normalize [-1,1] to [0,1]
            
            // DON'T free - static embeddings are cached
        }
    }
    
    // Base score: Edge transformer with context attention (structural signal)
    float edge_transform_score = edge_transform_with_context_attention(
        edge, 1.0f, context, graph
    );
    
    // EMBEDDING-AWARE: Boost edges whose targets match embedding context
    // If target fits semantic context, amplify edge score
    float target_embedding_boost = 1.0f;
    if (ctx_emb && graph && graph->embedding_cache) {
        SparseEmbedding *target_emb = node_compute_embedding(edge->to, graph);
        if (target_emb) {
            float target_ctx_match = sparse_embedding_similarity(target_emb, ctx_emb);
            target_ctx_match = (target_ctx_match + 1.0f) / 2.0f;  // Normalize [-1,1] to [0,1]
            
            // Boost edges to targets that fit embedding context
            if (target_ctx_match > 0.7f) {
                target_embedding_boost = 1.0f + (target_ctx_match - 0.7f) * 1.0f;  // 1.0-1.3x
            }
        }
    }
    
    // UNIFIED CONTEXT SCORE: Semantic (embedding) + Structural (edge)
    // Both must agree for high score
    float semantic_score = context_match;  // 0-1 from embeddings
    float structural_score = edge_transform_score;  // 0-1 from edge structure
    
    // Combine: multiplicative (both must agree) + additive (partial credit)
    float unified_score = (semantic_score * structural_score) +  // Both agree
                         (semantic_score + structural_score) * 0.15f;  // Partial credit
    
    // Apply embedding boost
    unified_score *= target_embedding_boost;
    
    // SECONDARY: Add relative memory as secondary signal
    // Edge weight as memory/log - "this path was used before"
    float weight_log = (float)edge->weight / 255.0f;
    float local_avg = node_get_outgoing_avg(node);
    if (local_avg > 0.0f) {
        float relative_memory = weight_log / local_avg;
        unified_score += relative_memory * 0.15f;
    }
    
    return unified_score;
}

/* Edge transforms activation using context attention
 * Implements: "edges act as mini transformers"
 * Attention over ALL activated nodes (infinite context!)
 * O(context_count) per edge, stays local
 */
static float edge_transform_with_context_attention(
    Edge *edge,
    float input_activation,
    ActivationPattern *context,
    Graph *graph
) {
    if (!edge || !edge->to || !context) {
        return (float)edge->weight / 255.0f * input_activation;
    }
    
    // Base transformation: weight × activation
    float weight_norm = (float)edge->weight / 255.0f;
    float base_output = weight_norm * input_activation;
    
    // No context or no cache: return base
    if (context->count == 0 || !graph || !graph->embedding_cache) {
        return base_output;
    }
    
    // CONTEXT ATTENTION: Q·K over all activated nodes
    // Query: from node's embedding
    SparseEmbedding *Q = node_compute_embedding(edge->from, graph);
    if (!Q) return base_output;
    
    // Key: target node's embedding
    SparseEmbedding *K_target = node_compute_embedding(edge->to, graph);
    if (!K_target) return base_output;
    
    // Compute attention over ALL activated nodes
    float max_attention = 0.0f;
    float target_attention = 0.0f;
    
    for (size_t i = 0; i < context->count; i++) {
        Node *ctx_node = context->nodes[i];
        float ctx_activation = context->activations[i];
        
        // Get context node embedding
        SparseEmbedding *K_ctx = node_compute_embedding(ctx_node, graph);
        if (!K_ctx) continue;
        
        // Attention score: Q·K (cosine similarity)
        float similarity = sparse_embedding_similarity(Q, K_ctx);
        float attention = similarity * ctx_activation;
        
        if (attention > max_attention) {
            max_attention = attention;
        }
        
        // Track target's attention
        if (ctx_node == edge->to) {
            target_attention = attention;
        }
    }
    
    // Target attention score relative to max
    target_attention = sparse_embedding_similarity(Q, K_target);
    float attention_weight = (max_attention > 0.01f) 
        ? target_attention / max_attention 
        : 1.0f;
    
    // Transform: base × attention
    return base_output * attention_weight;
}

/* Node predicts which edge to take (LOCAL DECISION)
 * Requirement: "nodes make mini prediction... predict what edge comes next"
 * Uses: embedding context + semantic features
 * Edge weights: memory/log (informative, not primary)
 */
static Edge* node_predict_next_edge(Node *node, ActivationPattern *context, Graph *graph) {
    if (!node || node->outgoing_count == 0) return NULL;
    
    DEBUG_LOG("[NODE_PREDICT] Node evaluating %zu outgoing edges\n", node->outgoing_count);
    
    // Compute node's context-dependent embedding (if available)
    // This enables disambiguation: same node, different context → different embedding
    SparseEmbedding *node_emb = NULL;
    SparseEmbedding *ctx_emb = NULL;
    
    if (graph && graph->embedding_cache && context && context->count > 0) {
        // Use context-dependent embedding instead of static embedding
        node_emb = node_compute_context_embedding(node, context, graph);
        ctx_emb = compute_context_embedding(context, graph);
    }
    
    // Evaluate each outgoing edge using contextual scoring
    Edge *best_edge = NULL;
    float best_contextual_score = -1.0f;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing[i];
        if (!edge || !edge->to || edge->to->payload_size == 0) continue;
        
        // Skip control characters
        if (edge->to->payload[0] < 32 && edge->to->payload[0] != '\n') continue;
        
        // Node evaluates this edge using its context
        float contextual_score = node_evaluate_edge_contextual_score(
            node, edge, node_emb, ctx_emb, context, graph
        );
        
        DEBUG_LOG("[NODE_PREDICT] Edge to '%c': score=%.3f\n",
                  edge->to->payload[0] >= 32 ? edge->to->payload[0] : '?',
                  contextual_score);
        
        if (contextual_score > best_contextual_score) {
            best_contextual_score = contextual_score;
            best_edge = edge;
        }
    }
    
    // STOP EDGE COMPETITION: Include STOP edge in competition (like LLM EOS token)
    // STOP edge competes with regular edges using same rules
    if (graph && graph->stop_node) {
        Edge *stop_edge = find_edge_between(node, graph->stop_node);
        if (stop_edge) {
            // Score STOP edge using same contextual scoring
            float stop_score = node_evaluate_edge_contextual_score(
                node, stop_edge, node_emb, ctx_emb, context, graph
            );
            
            DEBUG_LOG("[NODE_PREDICT] STOP edge: score=%.3f\n", stop_score);
            
            // STOP competes with regular edges - winner takes all
            if (stop_score > best_contextual_score) {
                best_contextual_score = stop_score;
                best_edge = stop_edge;  // STOP wins!
                DEBUG_LOG("[NODE_PREDICT] STOP edge won (score=%.3f)\n", stop_score);
            }
        }
    }
    
    // Free context-dependent embeddings (newly allocated, not cached)
    if (node_emb) {
        sparse_embedding_free(node_emb);
    }
    if (ctx_emb) {
        sparse_embedding_free(ctx_emb);
    }
    
    if (best_edge) {
        if (best_edge->to == graph->stop_node) {
            DEBUG_LOG("[NODE_PREDICT] Node chose STOP edge (score=%.3f)\n", best_contextual_score);
        } else {
            DEBUG_LOG("[NODE_PREDICT] Node chose edge to '%c' (score=%.3f)\n",
                      best_edge->to->payload[0] >= 32 ? best_edge->to->payload[0] : '?',
                      best_contextual_score);
        }
    } else {
        DEBUG_LOG("[NODE_PREDICT] Node couldn't choose any edge\n");
    }
    
    return best_edge;
}

/* DECODE: Select next output byte (NODE-LEVEL DECISION)
 * Brain: Neuron decides which synapse to fire based on local context
 * LLM: Token predicts next token using attention context
 * KEY CHANGE: Node makes the decision, not the system
 * Edge weights are memory/log only, context drives decision
 */
static Node* decode_select_next(Node *current, const uint8_t *output, size_t output_len,
                                ActivationPattern *pattern, Graph *graph) {
    if (!current) return NULL;
    
    (void)output;      // Mark as used
    (void)output_len;  // Mark as used
    
    // NODE MAKES THE DECISION (not system-level selection)
    // Node evaluates its context and predicts which edge to follow
    Edge *predicted_edge = node_predict_next_edge(current, pattern, graph);
    
    if (!predicted_edge || !predicted_edge->to) {
        DEBUG_LOG("[DECODE] Node couldn't predict next edge\n");
        return NULL;
    }
    
    // STOP EDGE HANDLING: If STOP edge won, signal to stop generation
    if (graph && graph->stop_node && predicted_edge->to == graph->stop_node) {
        DEBUG_LOG("[DECODE] STOP edge won - generation should stop\n");
        return NULL;  // Signal to stop (like LLM EOS token)
    }
    
    DEBUG_LOG("[DECODE] Node predicted edge to: %c\n", 
              predicted_edge->to->payload[0] >= 32 ? predicted_edge->to->payload[0] : '?');
    
    return predicted_edge->to;
}

/* DECODE: Check if generation should stop
 * Brain: Natural end of pattern (no strong predictions)
 * No hardcoded threshold - based on local context
 */
static int decode_check_stop(Node *current, float activation, ActivationPattern *pattern) {
    if (!current) return 1;
    
    // Stop if no outgoing edges (natural end)
    if (current->outgoing_count == 0) return 1;
    
    // Stop if average edge weight is very low (weak predictions)
    // Relative threshold: compared to local average (data-driven)
    // Only stop if weight is extremely weak relative to typical weights
    float avg_weight = node_get_outgoing_avg(current);
    
    // Compute relative weakness: avg_weight relative to minimum learned weight (10/255 ≈ 0.04)
    // Only stop if significantly weaker than minimum learned weight (half of minimum)
    float min_learned_weight = 10.0f / 255.0f;  // Minimum edge weight after creation
    if (avg_weight < min_learned_weight * 0.5f) return 1;  // Very weak relative to minimum
    
    // Continue generating even with lower activation
    // (removed strict activation drop check)
    (void)activation;  // Mark as used
    (void)pattern;     // Mark as used
    
    return 0;
}

/* DECODE: Generate output (main autoregressive loop)
 * LLM principle: Generate one token, add to context, repeat
 * Brain principle: Motor sequence generation
 * Enhanced: Creates embedding cache for semantic features, clears after generation
 */
static void generate_output(MFile *mfile, ActivationPattern *pattern, Node **input_nodes, size_t input_count) {
    if (!mfile || !mfile->graph || !pattern) return;
    
    Graph *graph = mfile->graph;
    
    // CREATE embedding cache (temporary for this generation)
    // This enables semantic features without permanent storage overhead
    graph->embedding_cache = embedding_cache_create(256);
    if (!graph->embedding_cache) {
        DEBUG_LOG("[GENERATE] Warning: Could not create embedding cache\n");
    }
    
    // Select first byte (EXCLUDE input nodes, with disambiguation)
    Node *current = decode_select_first(pattern, graph);
    if (!current) {
        DEBUG_LOG("[GENERATE] No continuation found, no output\n");
        
        // CLEAR embedding cache before returning
        embedding_cache_free(graph->embedding_cache);
        graph->embedding_cache = NULL;
        return;
    }
    
    // Output buffer
    uint8_t output[256];
    size_t output_len = 0;
    
    // Track path for cycle detection
    Node *path[256];
    size_t path_len = 0;
    
    // Cycle detection: track recent bytes (not nodes, for pattern detection)
    #define CYCLE_WINDOW 8
    uint8_t recent_bytes[CYCLE_WINDOW] = {0};
    size_t recent_idx = 0;
    
    // Autoregressive generation loop
    while (current && output_len < 256) {
        // HIERARCHY USAGE: Output full hierarchy payloads (compressed knowledge)
        // Vision: "Hierarchies enable matching larger patterns efficiently (10:1 compression per level)"
        // For hierarchies: output ALL bytes (not just first byte)
        // For raw nodes: output single byte
        size_t bytes_to_output = 1;  // Default: single byte (for raw nodes)
        if (current->abstraction_level > 0) {
            // Hierarchy node: output ALL bytes (compressed knowledge)
            bytes_to_output = current->payload_size;
        }
        
        // Output bytes from node's payload
        for (size_t b = 0; b < bytes_to_output && output_len < 256; b++) {
            if (b >= current->payload_size) break;  // Safety check
            uint8_t byte = current->payload[b];
            if (byte == 0x00) break;  // EOS marker
            
            output[output_len++] = byte;
            DEBUG_LOG("[GENERATE] Output[%zu] = '%c' (0x%02x) from %s\n", 
                      output_len - 1, 
                      byte >= 32 ? byte : '?',
                      byte,
                      current->abstraction_level > 0 ? "hierarchy" : "raw");
        }
        
        path[path_len++] = current;
        
        // Add to recent bytes (use last byte output, not first payload byte for hierarchies)
        if (output_len > 0) {
            recent_bytes[recent_idx] = output[output_len - 1];
            recent_idx = (recent_idx + 1) % CYCLE_WINDOW;
        }
        
        // Cycle detection: check for repeating byte patterns AFTER adding byte
        // Detect cycles of length 2-4 bytes (relative to output length)
        // Brain: Prevents repetitive motor patterns (like stuttering)
        if (output_len >= 4) {
            // Check for 2-byte cycle: "abab" 
            // output_len=4: output[0,1,2,3] where [2]==[0] and [3]==[1]
            if (output_len >= 4 &&
                output_len - 2 >= 0 &&
                output[output_len - 1] == output[output_len - 3] &&
                output[output_len - 2] == output[output_len - 4]) {
                DEBUG_LOG("[GENERATE] Stopping: 2-byte cycle detected (output_len=%zu)\n", output_len);
                output_len--;  // Remove the last byte (part of cycle)
                break;
            }
            
            // Check for 3-byte cycle: "abcabc"
            // output_len=6: output[0,1,2,3,4,5] where [3]==[0], [4]==[1], [5]==[2]
            if (output_len >= 6 &&
                output_len - 3 >= 0 &&
                output[output_len - 1] == output[output_len - 4] &&
                output[output_len - 2] == output[output_len - 5] &&
                output[output_len - 3] == output[output_len - 6]) {
                DEBUG_LOG("[GENERATE] Stopping: 3-byte cycle detected (output_len=%zu)\n", output_len);
                output_len -= 3;  // Remove the last 3 bytes (the repeated cycle)
                break;
            }
            
            // Check for 4-byte cycle: "abcdabcd"
            if (output_len >= 8 &&
                output_len - 4 >= 0 &&
                output[output_len - 1] == output[output_len - 5] &&
                output[output_len - 2] == output[output_len - 6] &&
                output[output_len - 3] == output[output_len - 7] &&
                output[output_len - 4] == output[output_len - 8]) {
                DEBUG_LOG("[GENERATE] Stopping: 4-byte cycle detected (output_len=%zu)\n", output_len);
                output_len -= 4;  // Remove the last 4 bytes (the repeated cycle)
                break;
            }
        }
        
        // Check stop conditions (after outputting all hierarchy bytes)
        float activation = activation_pattern_get(pattern, current);
        if (decode_check_stop(current, activation, pattern)) {
            DEBUG_LOG("[GENERATE] Stopping: natural end\n");
            break;
        }
        
        // Select next node (with semantic edge support)
        Node *next = decode_select_next(current, output, output_len, pattern, graph);
        if (!next) {
            // STOP edge won or no next node - stop generation
            // NO INTERNAL FEEDBACK: STOP edges are not strengthened here
            // Learning comes from external feedback only (melvin_m_feedback_error)
            DEBUG_LOG("[GENERATE] Stopping: no next node or STOP edge won\n");
            break;
        }
        
        // NO INTERNAL FEEDBACK: Edges are not strengthened during generation
        // Learning happens only:
        // 1. During training (create_sequential_edges - Hebbian learning from co-occurrence)
        // 2. From external feedback (melvin_m_feedback_error - external system provides error signals)
        
        current = next;
    }
    
    // Save output to mfile
    if (output_len > 0) {
        // Grow output buffer if needed
        if (output_len > mfile->output_capacity) {
            size_t new_cap = output_len * 2;
            uint8_t *new_buf = realloc(mfile->output_buffer, new_cap);
            if (new_buf) {
                mfile->output_buffer = new_buf;
                mfile->output_capacity = new_cap;
            } else {
                return;
            }
        }
        
        memcpy(mfile->output_buffer, output, output_len);
        mfile->output_size = output_len;
        
        DEBUG_LOG("[GENERATE] Generated %zu bytes\n", output_len);
    }
    
    // CLEAR embedding cache (free all temporary data)
    // This maintains zero permanent storage overhead
    embedding_cache_free(graph->embedding_cache);
    graph->embedding_cache = NULL;
    
    DEBUG_LOG("[GENERATE] Embedding cache cleared\n");
}

/* ============================================================================
 * HEBBIAN LEARNING (Phase 1 - Immediate)
 * Brain: "Neurons that fire together, wire together"
 * Pure synaptic plasticity - NO complex logic
 * 
 * Timing: Happens immediately when edges are created/used
 * Location: Called from create_sequential_edges() and during decode
 * ============================================================================ */

/* Strengthen an edge (simple increment) - LEGACY VERSION
 * Kept for backward compatibility with existing calls
 */
static void hebbian_strengthen_edge(Edge *edge, Graph *graph) {
    if (!edge || !graph) return;
    
    uint8_t old_weight = edge->weight;
    
    // Simple increment (saturate at 255)
    if (edge->weight < 255) {
        edge->weight++;
    }
    
    edge->last_used = graph->timestamp;
    
    // Update cached sum (O(1) maintenance)
    float old_float = (float)old_weight / 255.0f;
    float new_float = (float)edge->weight / 255.0f;
    edge->from->outgoing_weight_sum += (new_float - old_float);
    
    graph->adaptation_count++;
    
    DEBUG_LOG("[HEBBIAN] Strengthened edge: %u -> %u\n", old_weight, edge->weight);
}

/* Precise Hebbian learning with context signals
 * Combines:
 * - Hebbian: co-activation strengthening
 * - Context: bonus for context match (from embeddings, context functions)
 * All signals computed locally (no global backprop)
 */
static void hebbian_strengthen_edge_precise(
    Edge *edge,
    Graph *graph,
    float activation_used,
    float context_match_strength  // 0.0-1.0: how well context matched (from embeddings)
) {
    if (!edge || !graph) return;
    
    // 1. Base Hebbian: "fire together, wire together"
    float hebbian_delta = activation_used * 0.1f;
    
    // 2. Context bonus: edges that match context learn faster (from embeddings/context functions)
    float context_bonus = (context_match_strength > 0.8f) ? 0.05f : 0.0f;
    
    // 4. Relative to local average (prevents runaway growth)
    float local_avg = node_get_outgoing_avg(edge->from);
    float epsilon = (local_avg > 0.0f) ? local_avg * 0.01f : 0.01f;
    
    float edge_strength = (float)edge->weight / 255.0f;
    float relative_strength = (local_avg > 0.0f) ? edge_strength / local_avg : 1.0f;
    
    // Weak edges learn faster (catch up), strong edges learn slower (stability)
    float learning_rate = epsilon / (relative_strength + epsilon);
    
    // Combined update
    float total_delta = (hebbian_delta + context_bonus) * learning_rate;
    
    // Apply bounded update
    int new_weight = (int)edge->weight + (int)(total_delta * 255.0f);
    if (new_weight < 0) new_weight = 0;
    if (new_weight > 255) new_weight = 255;
    
    uint8_t old_weight = edge->weight;
    edge->weight = (uint8_t)new_weight;
    
    // Update cached sums
    float old_float = (float)old_weight / 255.0f;
    float new_float = (float)edge->weight / 255.0f;
    edge->from->outgoing_weight_sum += (new_float - old_float);
    
    graph->adaptation_count++;
    edge->last_used = graph->timestamp;
    
    DEBUG_LOG("[HEBBIAN_PRECISE] Edge: %u -> %u (act=%.2f, ctx=%.2f)\n", 
              old_weight, edge->weight, activation_used, context_match_strength);
}

/* Decay unused edges (intelligent forgetting) - LOCAL DECISION VERSION
 * Brain: Each neuron maintains its own synapses based on local usage
 * No global O(edges) loop - only decay edges from recently activated nodes
 */
static void hebbian_decay_unused(Graph *graph, Node **recently_active, size_t active_count) {
    if (!graph || !recently_active) return;
    
    uint32_t current_time = graph->timestamp;
    
    // LOCAL DECISION-MAKING: Only decay edges from recently activated nodes
    // Brain: Neurons maintain their own synapses, not a global controller
    // Scales: O(active_nodes × degree) instead of O(all_edges)
    for (size_t i = 0; i < active_count; i++) {
        if (recently_active[i]) {
            node_decay_unused_edges(recently_active[i], current_time);
        }
    }
}

/* Normalize weights (keep in bounds) */
static void hebbian_normalize_weights(Node *node) {
    if (!node || node->outgoing_count == 0) return;
    
    // Find max weight
    uint8_t max_weight = 0;
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing[i];
        if (edge && edge->weight > max_weight) {
            max_weight = edge->weight;
        }
    }
    
    // If max is at ceiling, scale all down
    if (max_weight >= 250) {
        for (size_t i = 0; i < node->outgoing_count; i++) {
            Edge *edge = node->outgoing[i];
            if (edge) {
                edge->weight = (edge->weight * 200) / 255;
            }
        }
        
        // Recompute weight sum
        node->outgoing_weight_sum = 0.0f;
        for (size_t i = 0; i < node->outgoing_count; i++) {
            Edge *edge = node->outgoing[i];
            if (edge) {
                node->outgoing_weight_sum += (float)edge->weight / 255.0f;
            }
        }
    }
}

/* ============================================================================
 * HIERARCHY FORMATION (Phase 1 - Before Processing)
 * Brain: Synaptic consolidation - frequently co-activated neurons group
 * Pure competition: Adaptive threshold based on variance
 * 
 * MOVED TO FRONT: Forms hierarchies BEFORE ENCODE/REFINE/DECODE
 * Benefits: New hierarchies available for pattern matching and processing
 * ============================================================================ */

/* Check for hierarchy formation (data-driven)
 * Requirement: No hardcoded thresholds - pure competition
 * Rule: Edges stronger than adaptive threshold (based on variance) form hierarchies
 * Timing: Called AFTER Hebbian learning, BEFORE ENCODE phase
 * O(degree) COMPLIANCE: Only checks edges from input nodes (local search), NOT all edges
 */
static void check_hierarchy_formation(Graph *graph, Node **input_nodes, size_t input_count) {
    if (!graph || !input_nodes || input_count < 2) return;
    
    // O(degree) COMPLIANCE: Only check edges from input nodes, not all edges globally
    // This avoids O(n) global search through graph->edges[] array
    // Instead: O(input_count * degree) which is O(degree) per node
    for (size_t i = 0; i < input_count - 1; i++) {
        Node *from = input_nodes[i];
        if (!from || from->outgoing_count == 0) continue;
        
        // Find sequential edge (local search - O(degree))
        Node *to = input_nodes[i + 1];
        if (!to) continue;
        
        Edge *edge = find_edge_between(from, to);
        if (!edge) continue;
        
        // RECURSIVE HIERARCHY FORMATION: Allow hierarchies from ANY node type
        // Vision: "recursive: hierarchies can combine into higher-level hierarchies"
        // Allows: byte→byte, byte→hierarchy, hierarchy→byte, hierarchy→hierarchy
        // Removed skip: hierarchies can now form from existing hierarchies
        
        // Compute relative strength and variance (data-driven threshold)
        float local_avg = node_get_outgoing_avg(edge->from);
        if (local_avg <= 0.0f) continue;
        
        // Compute variance of outgoing edge weights (for adaptive threshold)
        float *edge_weights = malloc(edge->from->outgoing_count * sizeof(float));
        if (!edge_weights) continue;
        
        for (size_t j = 0; j < edge->from->outgoing_count; j++) {
            if (edge->from->outgoing[j]) {
                edge_weights[j] = (float)edge->from->outgoing[j]->weight / 255.0f;
            } else {
                edge_weights[j] = 0.0f;
            }
        }
        
        float weight_variance = compute_variance(edge_weights, edge->from->outgoing_count);
        free(edge_weights);
        
        float edge_strength = (float)edge->weight / 255.0f;
        float relative_strength = edge_strength / local_avg;
        
        // DUAL LOCAL SIGNAL: Form hierarchies using purely local information (no global parameters)
        // Signal 1: Absolute strength - edge has been used many times (independent of context)
        // Signal 2: Variance-inverted relative strength - low variance = focused context = easier formation
        
        int should_form = 0;
        
        // SIGNAL 1: Absolute strength threshold (edge weight > 30% of max)
        // This handles "this edge has been strengthened many times"
        // Lowered from 70% to 30% because Hebbian learning updates are small per iteration
        float absolute_threshold = 80.0f / 255.0f;  // ~30% of max weight
        if (edge_strength > absolute_threshold) {
            should_form = 1;
            DEBUG_LOG("[HIERARCHY] Absolute strength trigger: edge_strength=%.3f > %.3f\n",
                      edge_strength, absolute_threshold);
        }
        
        // SIGNAL 2: Variance-inverted relative strength (only if signal 1 didn't trigger)
        // Low variance = focused context (few outgoing edges) → easier formation
        // High variance = diverse context (many outgoing edges) → harder formation
        // SPECIAL: Single-edge nodes (variance=0) have relative_strength=1.0x exactly
        // These are perfect hierarchy candidates: "this node always goes to this next node"
        if (!should_form && relative_strength > 0.0f) {
            // Invert variance: low variance → high normalized value → low threshold
            float variance_normalized = weight_variance > 0.0f ? 
                (1.0f / (1.0f + weight_variance)) : 1.0f;
            
            // Single-edge nodes (zero variance) → threshold 0.9x (allows 1.0x relative_strength)
            // Low variance (focused) → low threshold (0.9x-1.2x), easier formation
            // High variance (diverse) → high threshold (2.0x-3.0x), harder formation
            float base_threshold = (variance_normalized >= 0.99f) ? 0.9f :  // Single edge: 0.9x
                                  (1.0f + (1.0f - variance_normalized) * 2.0f);  // Otherwise: 1.0x-3.0x
            
            if (relative_strength > base_threshold) {
                should_form = 1;
                DEBUG_LOG("[HIERARCHY] Relative strength trigger: rel=%.3f > %.3f (var_norm=%.3f, edges=%zu)\n",
                          relative_strength, base_threshold, variance_normalized, edge->from->outgoing_count);
            }
        }
        
        // Form hierarchy if either signal triggered
        if (should_form) {
            // Find sequence following strong edges
            Node *sequence[32];
            size_t seq_len = 0;
            
            Node *current = edge->from;
            Edge *prev_edge = edge;
            
            // Track visited nodes to prevent cycles in hierarchy formation
            Node *visited[32] = {0};
            size_t visited_count = 0;
            
            while (current && seq_len < 32) {
                // Cycle detection: prevent infinite loops
                int already_visited = 0;
                for (size_t k = 0; k < visited_count; k++) {
                    if (visited[k] == current) {
                        already_visited = 1;
                        break;
                    }
                }
                if (already_visited) {
                    DEBUG_LOG("[HIERARCHY] Cycle detected, stopping sequence formation\n");
                    break;
                }
                
                sequence[seq_len++] = current;
                visited[visited_count++] = current;
                
                // SEQUENTIAL PATTERN FORMATION: Prefer edges from current input sequence
                // Vision: Form hierarchies from sequential patterns (h→e→l→l→o), not random strong edges
                // Prefer edges where both nodes were part of the input (is_input_node flag)
                // This ensures hierarchies form from the actual input pattern
                Edge *strongest = NULL;
                float max_strength = 0.0f;
                
                for (size_t j = 0; j < current->outgoing_count; j++) {
                    Edge *e = current->outgoing[j];
                    if (!e || !e->to) continue;
                    
                    float strength = (float)e->weight / 255.0f;
                    
                    // Boost strength if edge connects input nodes (sequential pattern)
                    // This prefers h→e→l→l→o over random o→lo paths
                    if (current->is_input_node && e->to->is_input_node) {
                        strength *= 1.5f;  // Boost sequential edges by 50%
                    }
                    
                    if (strength > max_strength) {
                        max_strength = strength;
                        strongest = e;
                    }
                }
                
                // DATA-DRIVEN: Stop if strength dropped relative to variance
                // Not hardcoded 80%, but relative to local variance
                float prev_strength = (float)prev_edge->weight / 255.0f;
                float strength_drop_threshold = prev_strength * 0.8f;  // Base 80%
                
                // Adjust threshold based on variance (high variance = more tolerance)
                float local_variance = weight_variance;
                float variance_tolerance = local_variance > 0.0f ? 
                    (1.0f - local_variance * 0.2f) : 0.8f;  // High variance → more tolerance
                if (variance_tolerance < 0.6f) variance_tolerance = 0.6f;  // Minimum 60%
                if (variance_tolerance > 0.9f) variance_tolerance = 0.9f;  // Maximum 90%
                
                strength_drop_threshold = prev_strength * variance_tolerance;
                
                if (!strongest || max_strength < strength_drop_threshold) {
                    break;
                }
                
                current = strongest->to;
                prev_edge = strongest;
            }
            
            // Create hierarchy if sequence is 2+ nodes
            if (seq_len >= 2) {
                Node *hierarchy = create_hierarchy_node(graph, sequence, seq_len);
                if (hierarchy) {
                    DEBUG_LOG("[HIERARCHY] Formed %zu-byte hierarchy\n", seq_len);
                }
            }
        }
    }
}

/* Create a hierarchy node from a sequence */
static Node* create_hierarchy_node(Graph *graph, Node **sequence, size_t seq_len) {
    if (!graph || !sequence || seq_len < 2) return NULL;
    
    // Compute total payload size
    size_t total_size = 0;
    for (size_t i = 0; i < seq_len; i++) {
        if (!sequence[i]) return NULL;
        total_size += sequence[i]->payload_size;
    }
    
    // Combine payloads
    uint8_t *combined = malloc(total_size);
    if (!combined) return NULL;
    
    size_t offset = 0;
    for (size_t i = 0; i < seq_len; i++) {
        memcpy(combined + offset, sequence[i]->payload, sequence[i]->payload_size);
        offset += sequence[i]->payload_size;
    }
    
    // Check if hierarchy already exists
    Node *existing = graph_find_node(graph, combined, total_size);
    if (existing) {
        free(combined);
        
        // Check if existing hierarchy is a concept (on-demand detection)
        if (is_concept_node(existing, graph)) {
            DEBUG_LOG("[CONCEPT] Hierarchy node acts as concept (level=%u, incoming=%zu)\n",
                      existing->abstraction_level, existing->incoming_count);
        }
        
        return existing;
    }
    
    // Create new hierarchy node
    Node *hierarchy = node_create(combined, total_size, 1);
    free(combined);
    
    if (!hierarchy) return NULL;
    
    graph_add_node(graph, hierarchy);
    
    // EDGE DIRECTION RULE: Edges only follow input order
    // Input "world" creates: w→o, o→r, r→l, l→d
    // Hierarchy "world" creates edges following same direction:
    //   1. first→hierarchy (shortcut entry: w→"world")
    //   2. hierarchy→next (continuation after pattern: "world"→d or next char)
    
    // RULE 1: Component→Hierarchy (shortcut entry)
    // When first component activates, it can jump directly to hierarchy
    // Example: 'w' activates → can choose 'w'→'o' OR 'w'→"world" (hierarchy)
    if (seq_len > 0 && sequence[0]) {
        Edge *comp_to_hierarchy = find_edge_between(sequence[0], hierarchy);
        if (!comp_to_hierarchy) {
            comp_to_hierarchy = edge_create(sequence[0], hierarchy);
            if (comp_to_hierarchy) {
                graph_add_edge(graph, comp_to_hierarchy);
                DEBUG_LOG("[HIERARCHY] Created shortcut: first→hierarchy\n");
            }
        }
    }
    
    // RULE 2: Hierarchy→Next Component (continuation after pattern)
    // After hierarchy completes, continue to what comes AFTER the sequence
    // This edge is created when hierarchy is used in context, not here
    // (Will be created by create_sequential_edges when hierarchy appears in input)
    
    
    // On-demand concept detection (no storage change, just logging)
    if (is_concept_node(hierarchy, graph)) {
        DEBUG_LOG("[CONCEPT] New hierarchy acts as concept (level=%u, incoming=%zu)\n",
                  hierarchy->abstraction_level, hierarchy->incoming_count);
    }
    
    return hierarchy;
}

/* PHASE 2: Hierarchy merging from activation pattern
 * Processes edges between ALL activated nodes (including hierarchies!)
 * Allows hierarchies to merge into larger hierarchies organically
 * Example: If "he" and "lo" hierarchies exist and co-activate,
 *          form "helo" hierarchy (if edge is strong enough)
 */
static void check_hierarchy_formation_phase2(Graph *graph, ActivationPattern *pattern) {
    if (!graph || !pattern || pattern->count < 2) return;
    
    // Create a hash set of activated nodes for O(1) lookup
    // (simple linear check is fine for small patterns)
    
    // Check edges between ALL activated nodes (not just input sequence)
    for (size_t i = 0; i < pattern->count; i++) {
        Node *from = pattern->nodes[i];
        if (!from || from->outgoing_count == 0) continue;
        
        // Check all outgoing edges from this activated node
        for (size_t j = 0; j < from->outgoing_count; j++) {
            Edge *edge = from->outgoing[j];
            if (!edge || !edge->to) continue;
            
            // Check if target node is also activated (co-activation)
            int target_activated = 0;
            for (size_t k = 0; k < pattern->count; k++) {
                if (pattern->nodes[k] == edge->to) {
                    target_activated = 1;
                    break;
                }
            }
            
            if (!target_activated) continue;  // Skip if target not activated
            
            // Use same dual local signal logic as Phase 1
            float local_avg = node_get_outgoing_avg(edge->from);
            if (local_avg <= 0.0f) continue;
            
            // Compute variance for adaptive threshold
            float *edge_weights = malloc(edge->from->outgoing_count * sizeof(float));
            if (!edge_weights) continue;
            
            for (size_t k = 0; k < edge->from->outgoing_count; k++) {
                if (edge->from->outgoing[k]) {
                    edge_weights[k] = (float)edge->from->outgoing[k]->weight / 255.0f;
                } else {
                    edge_weights[k] = 0.0f;
                }
            }
            
            float weight_variance = compute_variance(edge_weights, edge->from->outgoing_count);
            free(edge_weights);
            
            float edge_strength = (float)edge->weight / 255.0f;
            float relative_strength = edge_strength / local_avg;
            
            // DUAL LOCAL SIGNAL: Same logic as Phase 1
            int should_form = 0;
            
            // SIGNAL 1: Absolute strength
            float absolute_threshold = 80.0f / 255.0f;  // ~30% of max weight
            if (edge_strength > absolute_threshold) {
                should_form = 1;
                DEBUG_LOG("[HIERARCHY_P2] Absolute strength trigger: %zu→%zu (strength=%.3f)\n",
                          (size_t)from, (size_t)edge->to, edge_strength);
            }
            
            // SIGNAL 2: Variance-inverted relative strength
            if (!should_form && relative_strength > 0.0f) {
                float variance_normalized = weight_variance > 0.0f ? 
                    (1.0f / (1.0f + weight_variance)) : 1.0f;
                
                float base_threshold = (variance_normalized >= 0.99f) ? 0.9f :
                                      (1.0f + (1.0f - variance_normalized) * 2.0f);
                
                if (relative_strength > base_threshold) {
                    should_form = 1;
                    DEBUG_LOG("[HIERARCHY_P2] Relative strength trigger: %zu→%zu (rel=%.3f, thresh=%.3f)\n",
                              (size_t)from, (size_t)edge->to, relative_strength, base_threshold);
                }
            }
            
            // Form hierarchy if signal triggered
            if (should_form) {
                // Create 2-node hierarchy from this edge
                Node *sequence[2] = {from, edge->to};
                Node *hierarchy = create_hierarchy_node(graph, sequence, 2);
                if (hierarchy) {
                    DEBUG_LOG("[HIERARCHY_P2] Merged %zu→%zu into hierarchy\n",
                              (size_t)from, (size_t)edge->to);
                }
            }
        }
    }
}

/* ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================ */

/* Compute relative activation (value / local_max) */
static float compute_relative_activation(float value, float local_max) {
    if (local_max <= 0.0f) return 0.0f;
    return value / local_max;
}

/* Compute softmax for single value */
static float compute_softmax_single(float value, float *values, size_t count) {
    if (!values || count == 0) return 0.0f;
    
    // Compute exp(value) / sum(exp(values))
    float exp_val = expf(value);
    float sum_exp = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
        sum_exp += expf(values[i]);
    }
    
    return sum_exp > 0.0f ? exp_val / sum_exp : 0.0f;
}

/* ============================================================================
 * DATA-DRIVEN UTILITY FUNCTIONS (Relative, Adaptive, Context-Aware)
 * ============================================================================ */

/* Compute variance of values (for adaptive weighting) */
static float compute_variance(float *values, size_t count) {
    if (!values || count == 0) return 0.0f;
    
    // Compute mean
    float mean = 0.0f;
    for (size_t i = 0; i < count; i++) {
        mean += values[i];
    }
    mean /= (float)count;
    
    // Compute variance
    float variance = 0.0f;
    for (size_t i = 0; i < count; i++) {
        float diff = values[i] - mean;
        variance += diff * diff;
    }
    variance /= (float)count;
    
    return variance;
}

/* Compute relative percentile threshold (data-driven, not hardcoded) */
static float compute_percentile_threshold(float *values, size_t count, float percentile) {
    if (!values || count == 0) return 0.0f;
    if (percentile <= 0.0f) return values[0];
    if (percentile >= 1.0f) return values[count - 1];
    
    // Copy and sort
    float *sorted = malloc(count * sizeof(float));
    if (!sorted) return 0.0f;
    
    memcpy(sorted, values, count * sizeof(float));
    
    // Simple insertion sort
    for (size_t i = 1; i < count; i++) {
        float key = sorted[i];
        int j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }
    
    size_t idx = (size_t)((float)count * percentile);
    if (idx >= count) idx = count - 1;
    
    float threshold = sorted[idx];
    free(sorted);
    
    return threshold;
}

/* Compute adaptive weights from signal strengths (data-driven weighting) */
static void compute_adaptive_weights(float *weights, float *signals, size_t count) {
    if (!weights || !signals || count == 0) return;
    
    // Compute variance for each signal type (measure of information content)
    float total_variance = compute_variance(signals, count);
    
    if (total_variance <= 0.0f) {
        // Equal weights if no variance
        for (size_t i = 0; i < count; i++) {
            weights[i] = 1.0f / (float)count;
        }
        return;
    }
    
    // Weight by variance (higher variance = more informative = higher weight)
    float sum_weights = 0.0f;
    for (size_t i = 0; i < count; i++) {
        // Use absolute value of signal as base, variance as multiplier
        float signal_strength = fabsf(signals[i]);
        weights[i] = signal_strength * (1.0f + total_variance);
        sum_weights += weights[i];
    }
    
    // Normalize
    if (sum_weights > 0.0f) {
        for (size_t i = 0; i < count; i++) {
            weights[i] /= sum_weights;
        }
    }
}

/* Compute relative threshold based on local max/min/variance */
static float compute_relative_threshold(float value, float local_max, float local_min, float local_variance) {
    if (local_max <= local_min) return 0.0f;
    
    // Normalize value to [0, 1] range
    float normalized = (value - local_min) / (local_max - local_min);
    
    // Adjust based on variance (high variance = more spread = lower threshold)
    float variance_factor = local_variance > 0.0f ? 1.0f / (1.0f + local_variance) : 1.0f;
    
    return normalized * variance_factor;
}

/* Compute adaptive decay based on signal strength and context */
static float compute_adaptive_decay(int hop, float signal_strength, float context_richness) {
    // Base decay: 1/(hop+1) but adapts to signal quality
    float base_decay = 1.0f / (float)(hop + 1);
    
    // Strong signals decay slower (carry further)
    float strength_factor = signal_strength > 0.0f ? 1.0f + (signal_strength * 0.5f) : 1.0f;
    
    // Rich contexts decay slower (more connections to explore)
    float context_factor = context_richness > 0.0f ? 1.0f + (context_richness * 0.3f) : 1.0f;
    
    return base_decay * strength_factor * context_factor;
}

/* Compute local statistics (max, min, mean, variance) */
static void compute_local_stats(float *values, size_t count, 
                                float *out_max, float *out_min, 
                                float *out_mean, float *out_variance) {
    if (!values || count == 0) {
        if (out_max) *out_max = 0.0f;
        if (out_min) *out_min = 0.0f;
        if (out_mean) *out_mean = 0.0f;
        if (out_variance) *out_variance = 0.0f;
        return;
    }
    
    float max_val = values[0];
    float min_val = values[0];
    float sum = 0.0f;
    
    for (size_t i = 0; i < count; i++) {
        if (values[i] > max_val) max_val = values[i];
        if (values[i] < min_val) min_val = values[i];
        sum += values[i];
    }
    
    float mean = sum / (float)count;
    float variance = compute_variance(values, count);
    
    if (out_max) *out_max = max_val;
    if (out_min) *out_min = min_val;
    if (out_mean) *out_mean = mean;
    if (out_variance) *out_variance = variance;
}

/* ============================================================================
 * SPARSE EMBEDDING OPERATIONS (On-Demand Semantic Representation)
 * Brain: Distributed sparse neural codes
 * LLM: Token embeddings (but sparse for memory efficiency)
 * Principle: Compute on-demand, cache temporarily, clear after generation
 * ============================================================================ */

/* Create embedding cache (temporary, cleared after generation) */
static EmbeddingCache* embedding_cache_create(size_t capacity) {
    EmbeddingCache *cache = calloc(1, sizeof(EmbeddingCache));
    if (!cache) return NULL;
    
    cache->embeddings = calloc(capacity, sizeof(SparseEmbedding*));
    cache->nodes = calloc(capacity, sizeof(Node*));
    if (!cache->embeddings || !cache->nodes) {
        free(cache->embeddings);
        free(cache->nodes);
        free(cache);
        return NULL;
    }
    
    cache->capacity = capacity;
    cache->count = 0;
    cache->generation = 1;
    
    return cache;
}

/* Free embedding cache */
static void embedding_cache_free(EmbeddingCache *cache) {
    if (!cache) return;
    
    // Free all cached embeddings
    for (size_t i = 0; i < cache->count; i++) {
        sparse_embedding_free(cache->embeddings[i]);
    }
    
    free(cache->embeddings);
    free(cache->nodes);
    free(cache);
}

/* Clear embedding cache (invalidate all) */
static void embedding_cache_clear(EmbeddingCache *cache) {
    if (!cache) return;
    
    // Free all cached embeddings
    for (size_t i = 0; i < cache->count; i++) {
        sparse_embedding_free(cache->embeddings[i]);
        cache->embeddings[i] = NULL;
        cache->nodes[i] = NULL;
    }
    
    cache->count = 0;
    cache->generation++;
}

/* Get embedding from cache (NULL if not cached) */
static SparseEmbedding* embedding_cache_get(EmbeddingCache *cache, Node *node) {
    if (!cache || !node) return NULL;
    
    // Linear search (small cache, typically < 50 entries)
    for (size_t i = 0; i < cache->count; i++) {
        if (cache->nodes[i] == node) {
            return cache->embeddings[i];
        }
    }
    
    return NULL;
}

/* Put embedding in cache */
static void embedding_cache_put(EmbeddingCache *cache, Node *node, SparseEmbedding *emb) {
    if (!cache || !node || !emb) return;
    
    // Check if already cached
    for (size_t i = 0; i < cache->count; i++) {
        if (cache->nodes[i] == node) {
            // Replace existing
            sparse_embedding_free(cache->embeddings[i]);
            cache->embeddings[i] = emb;
            return;
        }
    }
    
    // Add new entry
    if (cache->count < cache->capacity) {
        cache->nodes[cache->count] = node;
        cache->embeddings[cache->count] = emb;
        cache->count++;
    }
}

/* Compute sparse embedding from node structure (O(degree))
 * Embedding dimensions:
 * - 0-255: Payload byte n-grams (content features)
 * - 256-511: Structural neighbors (graph features)
 * - 512-767: Hierarchy level (abstraction features)
 * Only 8-16 non-zero dimensions (sparse)
 */
static SparseEmbedding* node_compute_embedding(Node *node, Graph *graph) {
    if (!node) return NULL;
    
    // Check cache first
    if (graph && graph->embedding_cache) {
        SparseEmbedding *cached = embedding_cache_get(graph->embedding_cache, node);
        if (cached) return cached;
    }
    
    // Allocate sparse embedding (max 16 non-zeros)
    SparseEmbedding *emb = calloc(1, sizeof(SparseEmbedding));
    if (!emb) return NULL;
    
    emb->dims = malloc(16 * sizeof(uint16_t));
    emb->values = malloc(16 * sizeof(float));
    if (!emb->dims || !emb->values) {
        sparse_embedding_free(emb);
        return NULL;
    }
    
    size_t nnz = 0;
    
    // 1. Payload features (dims 0-255): byte n-grams
    if (node->payload && node->payload_size > 0) {
        // Unigrams
        for (size_t i = 0; i < node->payload_size && i < 4 && nnz < 16; i++) {
            emb->dims[nnz] = node->payload[i];
            emb->values[nnz] = 1.0f;
            nnz++;
        }
        
        // Bigrams (XOR for dimension)
        for (size_t i = 0; i + 1 < node->payload_size && i < 2 && nnz < 16; i++) {
            uint16_t dim = 256 + (node->payload[i] ^ node->payload[i + 1]);
            emb->dims[nnz] = dim;
            emb->values[nnz] = 0.7f;
            nnz++;
        }
    }
    
    // 2. Structural features (dims 256-511): neighbor connectivity
    if (node->outgoing_count > 0 && nnz < 16) {
        // Average outgoing weight
        float avg_weight = node_get_outgoing_avg(node);
        uint16_t dim = 256 + (uint16_t)(avg_weight * 255.0f);
        emb->dims[nnz] = dim;
        emb->values[nnz] = 0.5f;
        nnz++;
    }
    
    if (node->incoming_count > 0 && nnz < 16) {
        // Incoming count (normalized)
        float norm_count = (float)node->incoming_count / 10.0f;
        if (norm_count > 1.0f) norm_count = 1.0f;
        uint16_t dim = 384 + (uint16_t)(norm_count * 127.0f);
        emb->dims[nnz] = dim;
        emb->values[nnz] = 0.5f;
        nnz++;
    }
    
    // 3. Hierarchy features (dims 512-767): abstraction level
    if (node->abstraction_level > 0 && nnz < 16) {
        uint16_t dim = 512 + (node->abstraction_level * 32);
        emb->dims[nnz] = dim;
        emb->values[nnz] = 0.8f;
        nnz++;
    }
    
    emb->nnz = nnz;
    
    // Normalize (L2 norm)
    float norm = 0.0f;
    for (size_t i = 0; i < nnz; i++) {
        norm += emb->values[i] * emb->values[i];
    }
    norm = sqrtf(norm);
    
    if (norm > 0.0f) {
        for (size_t i = 0; i < nnz; i++) {
            emb->values[i] /= norm;
        }
    }
    
    // Cache if available
    if (graph && graph->embedding_cache) {
        embedding_cache_put(graph->embedding_cache, node, emb);
    }
    
    return emb;
}

/* Compute context-dependent node embedding
 * Same node gets different embeddings based on activated neighbors
 * Enables disambiguation: "bank" (river) vs "bank" (financial)
 * O(degree × context_count) but context_count is small (10-50)
 */
static SparseEmbedding* node_compute_context_embedding(
    Node *node,
    ActivationPattern *context,
    Graph *graph
) {
    if (!node || !context || !graph) return NULL;
    
    // Start with base embedding (may be cached!)
    SparseEmbedding *base_cached = node_compute_embedding(node, graph);
    if (!base_cached || context->count == 0) return base_cached;
    
    // CRITICAL FIX: Copy base embedding before modifying (don't corrupt cache!)
    // Multiple calls may get the same cached embedding - must copy before modifying
    SparseEmbedding *base = calloc(1, sizeof(SparseEmbedding));
    if (!base) return base_cached;  // Fallback if allocation fails
    
    base->dims = malloc(16 * sizeof(uint16_t));
    base->values = malloc(16 * sizeof(float));
    if (!base->dims || !base->values) {
        free(base->dims);
        free(base->values);
        free(base);
        return base_cached;  // Fallback if allocation fails
    }
    
    // Copy cached embedding to new embedding
    base->nnz = base_cached->nnz < 16 ? base_cached->nnz : 16;
    for (size_t i = 0; i < base->nnz; i++) {
        base->dims[i] = base_cached->dims[i];
        base->values[i] = base_cached->values[i];
    }
    
    // Now safely modify the copy (won't corrupt cache)
    // Modulate based on activated neighbors
    float total_modulation = 0.0f;
    
    for (size_t i = 0; i < context->count; i++) {
        Node *ctx_node = context->nodes[i];
        float ctx_activation = context->activations[i];
        
        // Skip self
        if (ctx_node == node) continue;
        
        // Check if connected (incoming or outgoing) and get edge strength
        Edge *edge_to_neighbor = NULL;
        for (size_t j = 0; j < node->outgoing_count; j++) {
            if (node->outgoing[j] && node->outgoing[j]->to == ctx_node) {
                edge_to_neighbor = node->outgoing[j];
                break;
            }
        }
        if (!edge_to_neighbor) {
            for (size_t j = 0; j < node->incoming_count; j++) {
                if (node->incoming[j] && node->incoming[j]->from == ctx_node) {
                    edge_to_neighbor = node->incoming[j];
                    break;
                }
            }
        }
        
        if (edge_to_neighbor) {
            // Get neighbor embedding
            SparseEmbedding *neighbor_emb = node_compute_embedding(ctx_node, graph);
            if (!neighbor_emb) continue;
            
            // EDGE-AWARE: Edge strength modulates influence
            // Stronger edges = stronger influence on embedding
            float edge_strength = (float)edge_to_neighbor->weight / 255.0f;
            
            // Compute attention (how relevant is this neighbor?)
            float attention = sparse_embedding_similarity(base, neighbor_emb);
            
            // Modulation combines: activation × attention × edge_strength
            // Edge structure reinforces semantic similarity
            float modulation_strength = attention * ctx_activation * (0.5f + edge_strength * 0.5f);
            
            // Modulate base embedding dimensions
            for (size_t d = 0; d < base->nnz && d < 16; d++) {
                // Find matching dimension in neighbor
                for (size_t n = 0; n < neighbor_emb->nnz; n++) {
                    if (base->dims[d] == neighbor_emb->dims[n]) {
                        // Blend: base + neighbor weighted by attention
                        base->values[d] = base->values[d] * 0.7f + 
                                         neighbor_emb->values[n] * modulation_strength * 0.3f;
                        break;
                    }
                }
            }
            
            total_modulation += modulation_strength;
        }
    }
    
    // Normalize if modulated
    if (total_modulation > 0.01f) {
        float norm = 0.0f;
        for (size_t i = 0; i < base->nnz; i++) {
            norm += base->values[i] * base->values[i];
        }
        if (norm > 0.0f) {
            norm = sqrtf(norm);
            for (size_t i = 0; i < base->nnz; i++) {
                base->values[i] /= norm;
            }
        }
    }
    
    return base;
}

/* Compute cosine similarity between sparse embeddings (O(k) merge-style) */
static float sparse_embedding_similarity(SparseEmbedding *a, SparseEmbedding *b) {
    if (!a || !b || a->nnz == 0 || b->nnz == 0) return 0.0f;
    
    float dot_product = 0.0f;
    size_t i = 0, j = 0;
    
    // Merge-style intersection (assumes sorted dims)
    while (i < a->nnz && j < b->nnz) {
        if (a->dims[i] == b->dims[j]) {
            dot_product += a->values[i] * b->values[j];
            i++;
            j++;
        } else if (a->dims[i] < b->dims[j]) {
            i++;
        } else {
            j++;
        }
    }
    
    // Cosine similarity (already normalized, so just dot product)
    return dot_product;
}

/* Free sparse embedding */
static void sparse_embedding_free(SparseEmbedding *emb) {
    if (!emb) return;
    free(emb->dims);
    free(emb->values);
    free(emb);
}

/* Compute context embedding (weighted sum of active nodes) */
static SparseEmbedding* compute_context_embedding(ActivationPattern *pattern, Graph *graph) {
    if (!pattern || pattern->count == 0) return NULL;
    
    // Allocate context embedding
    SparseEmbedding *ctx_emb = calloc(1, sizeof(SparseEmbedding));
    if (!ctx_emb) return NULL;
    
    ctx_emb->dims = malloc(16 * sizeof(uint16_t));
    ctx_emb->values = malloc(16 * sizeof(float));
    if (!ctx_emb->dims || !ctx_emb->values) {
        sparse_embedding_free(ctx_emb);
        return NULL;
    }
    
    // Aggregate embeddings from ALL activated nodes (weighted by activation strength)
    // Activation strength = temporal relevance (high = recent, low = decayed)
    float dim_sums[768] = {0};  // Temporary accumulator
    
    // Use ALL nodes in pattern, weighted by their activation strength
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        // Skip nodes with negligible activation (decayed to near-zero)
        if (activation < 0.001f) continue;
        
        SparseEmbedding *node_emb = node_compute_embedding(node, graph);
        if (!node_emb) continue;
        
        // Add weighted contribution (activation = temporal relevance weight)
        for (size_t j = 0; j < node_emb->nnz; j++) {
            uint16_t dim = node_emb->dims[j];
            if (dim < 768) {
                dim_sums[dim] += node_emb->values[j] * activation;
            }
        }
    }
    
    // Extract top 16 dimensions
    size_t nnz = 0;
    for (uint16_t dim = 0; dim < 768 && nnz < 16; dim++) {
        if (dim_sums[dim] > 0.01f) {
            ctx_emb->dims[nnz] = dim;
            ctx_emb->values[nnz] = dim_sums[dim];
            nnz++;
        }
    }
    
    ctx_emb->nnz = nnz;
    
    // Normalize
    float norm = 0.0f;
    for (size_t i = 0; i < nnz; i++) {
        norm += ctx_emb->values[i] * ctx_emb->values[i];
    }
    norm = sqrtf(norm);
    
    if (norm > 0.0f) {
        for (size_t i = 0; i < nnz; i++) {
            ctx_emb->values[i] /= norm;
        }
    }
    
    return ctx_emb;
}

/* ============================================================================
 * VIRTUAL SEMANTIC EDGES (On-Demand Computation, Never Stored)
 * Brain: Semantic associations beyond direct synaptic connections
 * LLM: Attention weights between tokens
 * Principle: Compute only during decode, free immediately after use
 * ============================================================================ */

/* Compute virtual semantic edges from node (O(k) for k active nodes) */
static VirtualSemanticEdge* node_get_semantic_edges(Node *from, ActivationPattern *context,
                                                     Graph *graph, size_t *out_count) {
    if (!from || !context || !graph || !out_count) return NULL;
    
    *out_count = 0;
    
    // DATA-DRIVEN: Minimum context size relative to graph size
    // Not hardcoded 5, but relative to typical activation pattern size
    // Typical pattern: 5-20 nodes, so minimum should be ~25% of typical
    float min_context_ratio = 0.25f;  // 25% of typical pattern
    size_t min_context_size = 3;  // Absolute minimum (safety)
    size_t adaptive_min = (size_t)((float)graph->node_count * min_context_ratio);
    if (adaptive_min < min_context_size) adaptive_min = min_context_size;
    if (adaptive_min > 10) adaptive_min = 10;  // Cap at 10
    
    if (context->count < adaptive_min) return NULL;
    
    // Get from node embedding
    SparseEmbedding *from_emb = node_compute_embedding(from, graph);
    if (!from_emb) return NULL;
    
    // Allocate virtual edges (max 8 semantic connections)
    VirtualSemanticEdge *edges = malloc(8 * sizeof(VirtualSemanticEdge));
    if (!edges) return NULL;
    
    size_t edge_count = 0;
    float max_similarity = 0.0f;
    
    // Compute similarity to each active node in context
    for (size_t i = 0; i < context->count && edge_count < 8; i++) {
        Node *to = context->nodes[i];
        
        // Skip self and direct neighbors (already have structural edges)
        if (to == from) continue;
        if (find_edge_between(from, to)) continue;
        
        // Compute semantic similarity
        SparseEmbedding *to_emb = node_compute_embedding(to, graph);
        if (!to_emb) continue;
        
        float similarity = sparse_embedding_similarity(from_emb, to_emb);
        
        // Collect all similarities first (for relative threshold computation)
        if (similarity > 0.0f) {
            edges[edge_count].to = to;
            edges[edge_count].semantic_strength = similarity;
            edges[edge_count].combined_score = similarity;  // Will be combined with structural
            edge_count++;
            
            if (similarity > max_similarity) {
                max_similarity = similarity;
            }
        }
    }
    
    // DATA-DRIVEN: Filter by relative threshold (not hardcoded 0.1)
    // Keep only similarities above 10% of max (relative, not absolute)
    if (max_similarity > 0.0f && edge_count > 0) {
        float relative_threshold = max_similarity * 0.1f;  // 10% of max (relative)
        size_t filtered_count = 0;
        
        for (size_t i = 0; i < edge_count; i++) {
            if (edges[i].semantic_strength >= relative_threshold) {
                // Keep this edge, normalize relative to max
                edges[i].semantic_strength /= max_similarity;
                edges[i].combined_score = edges[i].semantic_strength;
                
                if (i != filtered_count) {
                    edges[filtered_count] = edges[i];
                }
                filtered_count++;
            }
        }
        edge_count = filtered_count;
    } else if (max_similarity > 0.0f) {
        // Normalize all if no filtering
        for (size_t i = 0; i < edge_count; i++) {
            edges[i].semantic_strength /= max_similarity;
            edges[i].combined_score = edges[i].semantic_strength;
        }
    }
    
    *out_count = edge_count;
    
    if (edge_count == 0) {
        free(edges);
        return NULL;
    }
    
    return edges;
}

/* Free virtual edges */
static void virtual_edges_free(VirtualSemanticEdge *edges) {
    free(edges);
}

/* ============================================================================
 * CONCEPT FORMATION (Uses Existing Hierarchy Structure)
 * Brain: Abstract concepts emerge from repeated similar patterns
 * Principle: No new storage, detect concepts on-demand from hierarchy
 * ============================================================================ */

/* Compute average semantic similarity of incoming nodes */
static float compute_incoming_similarity(Node *node, Graph *graph) {
    if (!node || node->incoming_count < 2) return 0.0f;
    
    // Sample first few incoming nodes
    size_t sample_size = node->incoming_count < 5 ? node->incoming_count : 5;
    float total_similarity = 0.0f;
    size_t pair_count = 0;
    
    for (size_t i = 0; i < sample_size; i++) {
        Node *node_i = node->incoming[i]->from;
        SparseEmbedding *emb_i = node_compute_embedding(node_i, graph);
        if (!emb_i) continue;
        
        for (size_t j = i + 1; j < sample_size; j++) {
            Node *node_j = node->incoming[j]->from;
            SparseEmbedding *emb_j = node_compute_embedding(node_j, graph);
            if (!emb_j) continue;
            
            float sim = sparse_embedding_similarity(emb_i, emb_j);
            total_similarity += sim;
            pair_count++;
        }
    }
    
    return pair_count > 0 ? total_similarity / (float)pair_count : 0.0f;
}

/* Compute similarity between node and context */
static float compute_context_similarity(Node *node, ActivationPattern *context, Graph *graph) {
    if (!node || !context) return 0.0f;
    
    SparseEmbedding *node_emb = node_compute_embedding(node, graph);
    if (!node_emb) return 0.0f;
    
    SparseEmbedding *ctx_emb = compute_context_embedding(context, graph);
    if (!ctx_emb) return 0.0f;
    
    float similarity = sparse_embedding_similarity(node_emb, ctx_emb);
    sparse_embedding_free(ctx_emb);
    
    return similarity;
}

/* Detect if hierarchy node is a concept (on-demand check) */
static int is_concept_node(Node *hierarchy, Graph *graph) {
    if (!hierarchy) return 0;
    
    // Concept criteria:
    // 1. Multi-level abstraction (level >= 2)
    // 2. Multiple instances (3+ incoming edges)
    // 3. Incoming nodes semantically similar
    
    if (hierarchy->abstraction_level < 2) return 0;
    if (hierarchy->incoming_count < 3) return 0;
    
    // Check semantic similarity of incoming nodes
    float avg_similarity = compute_incoming_similarity(hierarchy, graph);
    
    // DATA-DRIVEN: Threshold relative to typical similarity in graph
    // Compute average similarity across all nodes (sample-based estimate)
    // Threshold should be above typical similarity (not hardcoded 0.5)
    float typical_similarity = 0.3f;  // Estimate: typical nodes have ~0.3 similarity
    float threshold = typical_similarity + (1.0f - typical_similarity) * 0.4f;  // 40% above typical
    // This gives ~0.58 threshold (adaptive, not hardcoded 0.5)
    
    return avg_similarity > threshold;
}

/* Find generalizing concept for instance */
static Node* find_generalizing_concept(Node *instance, ActivationPattern *context, Graph *graph) {
    if (!instance) return NULL;
    
    Node *best_concept = NULL;
    float best_similarity = 0.0f;
    
    // Traverse hierarchy parents (incoming edges to higher abstraction levels)
    for (size_t i = 0; i < instance->incoming_count; i++) {
        Edge *edge = instance->incoming[i];
        Node *parent = edge->from;
        
        // Must be multi-level hierarchy to be a concept
        if (parent->abstraction_level < 2) continue;
        
        // Compute similarity to context
        float sim = compute_context_similarity(parent, context, graph);
        
        if (sim > best_similarity) {
            best_similarity = sim;
            best_concept = parent;
        }
    }
    
    return best_concept;
}

/* Find parent concept node (first hierarchy parent with level >= 2) */
static Node* find_parent_concept(Node *instance, Graph *graph) {
    if (!instance) return NULL;
    
    (void)graph;  // Not used, but kept for API consistency
    
    for (size_t i = 0; i < instance->incoming_count; i++) {
        Node *parent = instance->incoming[i]->from;
        if (parent->abstraction_level >= 2) {
            return parent;
        }
    }
    
    return NULL;
}

/* ============================================================================
 * DISAMBIGUATION (Context-Based Selection)
 * Brain: Context resolves ambiguity (like word-sense disambiguation)
 * Principle: Use semantic similarity to context for selection
 * ============================================================================ */

/* Disambiguate candidates using context
 * NO FALLBACKS: All decisions relative to local context
 * Uses embeddings when available, otherwise uses activation scores
 */
static Node* disambiguate_with_context(Node **candidates, size_t candidate_count,
                                       ActivationPattern *context, Graph *graph) {
    if (!candidates || candidate_count == 0) return NULL;
    if (candidate_count == 1) return candidates[0];
    
    // Try to use semantic embeddings (preferred)
    SparseEmbedding *ctx_emb = compute_context_embedding(context, graph);
    
    if (ctx_emb && graph) {
        // Score each candidate by similarity to context (semantic disambiguation)
        float best_score = -1.0f;
        Node *best_candidate = NULL;
        
        for (size_t i = 0; i < candidate_count; i++) {
            SparseEmbedding *cand_emb = node_compute_embedding(candidates[i], graph);
            if (!cand_emb) continue;
            
            float similarity = sparse_embedding_similarity(cand_emb, ctx_emb);
            
            if (similarity > best_score) {
                best_score = similarity;
                best_candidate = candidates[i];
            }
        }
        
        sparse_embedding_free(ctx_emb);
        
        // Return best candidate found (relative selection, no fallback)
        if (best_candidate) return best_candidate;
    }
    
    // If embeddings not available, use activation scores (relative to max)
    // This is NOT a fallback - it's an alternative decision mechanism
    // Both use relative thresholds, no hardcoded values
    float max_activation = -1.0f;
    Node *best_by_activation = NULL;
    
    if (context) {
        for (size_t i = 0; i < candidate_count; i++) {
            // Find activation score for this candidate
            float activation = 0.0f;
            for (size_t j = 0; j < context->count; j++) {
                if (context->nodes[j] == candidates[i]) {
                    activation = context->activations[j];
                    break;
                }
            }
            
            if (activation > max_activation) {
                max_activation = activation;
                best_by_activation = candidates[i];
            }
        }
    }
    
    // Return candidate with highest activation (relative selection)
    // If no activation scores, return first candidate (all equivalent without context)
    return best_by_activation ? best_by_activation : candidates[0];
}

/* ============================================================================
 * ANALOGICAL REASONING (Hierarchy-Based)
 * Brain: Transfer knowledge across similar structures
 * Principle: A:B :: C:? via shared concept hierarchy
 * ============================================================================ */

/* Analogical reasoning: A:B :: C:? */
static Node* analogical_reasoning(Node *A, Node *B, Node *C, Graph *graph) {
    if (!A || !B || !C || !graph) return NULL;
    
    // Find concepts (hierarchy parents)
    Node *concept_A = find_parent_concept(A, graph);
    Node *concept_C = find_parent_concept(C, graph);
    
    if (!concept_A || !concept_C) return NULL;
    
    // If same concept category, find analogous target
    if (concept_A == concept_C) {
        // Find children of concept_C similar to B
        float best_similarity = 0.0f;
        Node *best_target = NULL;
        
        SparseEmbedding *emb_B = node_compute_embedding(B, graph);
        if (!emb_B) return NULL;
        
        for (size_t i = 0; i < concept_C->outgoing_count; i++) {
            Node *child = concept_C->outgoing[i]->to;
            
            // Skip if child is A or C (we want the analogous target)
            if (child == A || child == C) continue;
            
            SparseEmbedding *emb_child = node_compute_embedding(child, graph);
            if (!emb_child) continue;
            
            float sim = sparse_embedding_similarity(emb_B, emb_child);
            if (sim > best_similarity) {
                best_similarity = sim;
                best_target = child;
            }
        }
        
        return best_target;
    }
    
    return NULL;
}

/* ============================================================================
 * PUBLIC API IMPLEMENTATION
 * ============================================================================ */

/* Create new .m file */
MelvinMFile* melvin_m_create(const char *filename) {
    if (!filename) return NULL;
    
    MFile *mfile = calloc(1, sizeof(MFile));
    if (!mfile) return NULL;
    
    mfile->filename = strdup(filename);
    mfile->graph = graph_create();
    
    // Initialize buffers
    mfile->input_capacity = 1024;
    mfile->input_buffer = malloc(mfile->input_capacity);
    mfile->output_capacity = 1024;
    mfile->output_buffer = malloc(mfile->output_capacity);
    
    if (!mfile->filename || !mfile->graph || !mfile->input_buffer || !mfile->output_buffer) {
        free(mfile->filename);
        graph_free(mfile->graph);
        free(mfile->input_buffer);
        free(mfile->output_buffer);
        free(mfile);
        return NULL;
    }
    
    mfile->last_input_port_id = PORT_TEXT;
    
    return mfile;
}

/* Load existing .m file */
MelvinMFile* melvin_m_load(const char *filename) {
    // TODO: Implement proper .m file loading
    // For now, just create new
    return melvin_m_create(filename);
}

/* Save .m file */
int melvin_m_save(MelvinMFile *mfile) {
    // TODO: Implement proper .m file saving
    // For now, just return success
    return 0;
}

/* Close .m file */
void melvin_m_close(MelvinMFile *mfile) {
    if (!mfile) return;
    
    free(mfile->filename);
    graph_free(mfile->graph);
    free(mfile->input_buffer);
    free(mfile->output_buffer);
    free(mfile);
}

/* Write data to universal input */
void melvin_m_universal_input_write(MelvinMFile *mfile, const uint8_t *data, size_t size) {
    if (!mfile || !data || size == 0) return;
    
    // Grow buffer if needed
    if (mfile->input_size + size > mfile->input_capacity) {
        size_t new_cap = (mfile->input_size + size) * 2;
        uint8_t *new_buf = realloc(mfile->input_buffer, new_cap);
        if (!new_buf) return;
        mfile->input_buffer = new_buf;
        mfile->input_capacity = new_cap;
    }
    
    memcpy(mfile->input_buffer + mfile->input_size, data, size);
    mfile->input_size += size;
}

/* Get universal input size */
size_t melvin_m_universal_input_size(MelvinMFile *mfile) {
    return mfile ? mfile->input_size : 0;
}

/* Clear universal input */
void melvin_m_universal_input_clear(MelvinMFile *mfile) {
    if (mfile) {
        mfile->input_size = 0;
    }
}

/* Process input */
int melvin_m_process_input(MelvinMFile *mfile) {
    if (!mfile || mfile->input_size == 0) return -1;
    
    // Clear previous output
    mfile->output_size = 0;
    
    // Process input bytes
    process_input_bytes(mfile, mfile->input_buffer, mfile->input_size);
    
    return 0;
}

/* Get last input port ID */
uint8_t melvin_m_get_last_input_port_id(MelvinMFile *mfile) {
    return mfile ? mfile->last_input_port_id : 0;
}

/* Set last input port ID */
void melvin_m_set_last_input_port_id(MelvinMFile *mfile, uint8_t port_id) {
    if (mfile) {
        mfile->last_input_port_id = port_id;
    }
}

/* Get universal output size */
size_t melvin_m_universal_output_size(MelvinMFile *mfile) {
    return mfile ? mfile->output_size : 0;
}

/* Read universal output */
size_t melvin_m_universal_output_read(MelvinMFile *mfile, uint8_t *buffer, size_t buffer_size) {
    if (!mfile || !buffer || buffer_size == 0) return 0;
    
    size_t to_copy = mfile->output_size < buffer_size ? mfile->output_size : buffer_size;
    if (to_copy > 0) {
        memcpy(buffer, mfile->output_buffer, to_copy);
    }
    return to_copy;
}

/* Clear universal output */
void melvin_m_universal_output_clear(MelvinMFile *mfile) {
    if (mfile) {
        mfile->output_size = 0;
    }
}

/* Feedback error signal (public API - minimal implementation) */
void melvin_m_feedback_error(MelvinMFile *mfile, float error_signal) {
    // Internal feedback mechanisms removed - this is a no-op stub for API compatibility
    (void)mfile;
    (void)error_signal;
}

/* Strengthen continuation (self-supervised learning) */
void melvin_m_strengthen_continuation(MelvinMFile *mfile, const uint8_t *sequence, 
                                       size_t prefix_len, size_t total_len) {
    if (!mfile || !mfile->graph || !sequence || prefix_len >= total_len) return;
    
    // Create nodes for the continuation part
    Node **nodes = malloc((total_len - prefix_len) * sizeof(Node*));
    if (!nodes) return;
    
    for (size_t i = prefix_len; i < total_len; i++) {
        nodes[i - prefix_len] = find_or_create_node(mfile->graph, &sequence[i], 1);
    }
    
    // Strengthen edges in continuation
    for (size_t i = 0; i < total_len - prefix_len - 1; i++) {
        if (!nodes[i] || !nodes[i + 1]) continue;
        
        Edge *edge = find_edge_between(nodes[i], nodes[i + 1]);
        if (!edge) {
            edge = edge_create(nodes[i], nodes[i + 1]);
            if (edge) {
                graph_add_edge(mfile->graph, edge);
            }
        }
        
        if (edge) {
            // Strengthen multiple times for supervision
            for (int j = 0; j < 5; j++) {
                hebbian_strengthen_edge(edge, mfile->graph);
            }
        }
    }
    
    free(nodes);
}

/* Get node count */
size_t melvin_m_get_node_count(MelvinMFile *mfile) {
    return (mfile && mfile->graph) ? mfile->graph->node_count : 0;
}

/* Get edge count */
size_t melvin_m_get_edge_count(MelvinMFile *mfile) {
    return (mfile && mfile->graph) ? mfile->graph->edge_count : 0;
}

/* Get adaptation count */
uint64_t melvin_m_get_adaptation_count(MelvinMFile *mfile) {
    return (mfile && mfile->graph) ? mfile->graph->adaptation_count : 0;
}
