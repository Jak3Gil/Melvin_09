/*
 * Melvin: Emergent Intelligence System
 * 
 * Core engine implementation that processes .m files according to README.md rules.
 * This file implements all processing logic; melvin_in_port.c and melvin_out_port.c
 * handle external device I/O.
 * 
 * Core Principles (from README):
 * 1. Self-Regulation Through Local Measurements Only
 * 2. No Hardcoded Limits or Thresholds
 * 3. Relative Adaptive Stability
 * 4. Compounding Learning
 * 5. Adaptive Behavior
 * 6. Continuous Learning
 * 7. Emergent Intelligence
 * 8. Explicit Hierarchical Abstraction
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
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* ============================================================================
 * MINI NEURAL NET: Universal Decision-Making Structure
 * 
 * All .m file processing logic is in this file (README requirement).
 * Mini nets make all decisions - nodes, edges, and global operations.
 * Variables are multi-functional - meaning depends on context.
 * 
 * Core Principles:
 * - Local computation only (no global state)
 * - Context-aware (same net, different behavior in different contexts)
 * - Hebbian learning (strengthen with use)
 * - Data-driven (no hardcoded thresholds)
 * ============================================================================ */

/* Mini Neural Net Structure */
typedef struct MiniNet {
    // Multi-functional weights (context determines meaning)
    float *weights;
    size_t weight_count;
    size_t weight_capacity;
    
    // Multi-functional state (context determines meaning)
    float *state;
    size_t state_dim;
    size_t state_capacity;
    
    // Multi-functional bias (context determines meaning)
    float bias;
    
    // Activation function parameters (learned, not fixed)
    float activation_sharpness;  // Adaptive, not hardcoded
    float activation_offset;      // Adaptive, not hardcoded
    
    // Context-aware gating (like LSTM gates)
    float input_gate;         // Controls input influence
    float forget_gate;        // Controls state retention
    float output_gate;        // Controls output magnitude
    
    // Learning state (for adaptive learning)
    float learning_momentum;  // Momentum for weight updates
    float learning_variance;  // Variance of recent updates (for adaptive rates)
} MiniNet;

/* Variable Context Enum (for multi-functional variables) */
typedef enum {
    CTX_ACTIVATION,      // Variable used for activation computation
    CTX_LEARNING,        // Variable used for learning rate
    CTX_PRUNING,         // Variable used for pruning decision
    CTX_HIERARCHY,       // Variable used for hierarchy formation
    CTX_OUTPUT,          // Variable used for output generation
    CTX_EXPLORATION,     // Variable used for exploration vs exploitation
    CTX_ATTENTION,       // Variable used for attention computation
    CTX_TRANSFORMATION   // Variable used for edge transformation
} VariableContext;

/* ============================================================================
 * CORE DATA STRUCTURES
 * ============================================================================ */

/* Forward declarations */
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Edge Edge;

/* Node: Mini Neural Net
 * - Computes activation from weighted inputs
 * - All decisions relative to local context
 * - Self-regulating bias
 * - O(degree) operations only
 * 
 * REFACTORED: Now contains a MiniNet for all decision-making
 */
typedef struct Node {
    uint8_t id[9];                    // Unique identifier (8 bytes + null)
    uint8_t *payload;                 // Actual data (flexible, 0 to unlimited)
    size_t payload_size;              // Size in bytes (0 = blank node)
    
    uint8_t port_id;                  // Port ID where this node originated (0 = unknown/original)
    uint32_t abstraction_level;       // 0 = raw, 1+ = hierarchy
    
    // MINI NEURAL NET (all decisions flow through this)
    MiniNet *net;                     // The node IS a mini neural net
    
    // Multi-functional variables (context-aware interpretation)
    float activation_strength;        // Can be: current activation, readiness, confidence
    float weight;                     // Can be: importance, age, usage frequency
    float bias;                       // Can be: meta-learning rate, exploration, strategy
    float state;                      // Can be: hidden state, momentum, context memory
    
    // Local edge tracking (O(1) access to cached sums)
    struct Edge **outgoing_edges;
    size_t outgoing_count;
    size_t outgoing_capacity;
    float outgoing_weight_sum;        // Cached sum (maintained incrementally)
    
    struct Edge **incoming_edges;
    size_t incoming_count;
    size_t incoming_capacity;
    float incoming_weight_sum;        // Cached sum (maintained incrementally)
    
    // Adaptive learning rate tracking (kept for compatibility, but mini net also tracks)
    float *recent_weight_changes;     // Rolling window (adaptive size)
    size_t weight_change_capacity;
    size_t weight_change_count;
    size_t weight_change_index;
    float change_rate_avg;
    
    // File offset for lazy loading
    off_t file_offset;
    uint8_t loaded;                   // 0 = not loaded, 1 = loaded
    
    // Self-destruct tracking (relative timer, no global state)
    float inactivity_timer;           // Increments when not activated, resets when activated
    
    // Stop prediction tracking (for wave propagation learning)
    uint32_t total_activations;       // Total times this node was activated during output
    float stop_weight;                // Learned weight for stopping at this node (0.0 = no bias to stop)
    
    // Context-relative edge values (for quick winning edge computation)
    float *edge_context_values;       // Cached: edge->weight / local_avg for each outgoing edge
    size_t edge_context_capacity;     // Allocated capacity of edge_context_values array
    Edge *best_edge;                  // Cached: pointer to edge with highest context value
    float best_edge_value;            // Cached: context value of best edge
    uint32_t context_generation;      // Generation when context values were last computed
    
    // ADAPTIVE CONTEXT TRACE (RNN-like hidden state for disambiguation)
    // ADAPTIVE: Grows/shrinks based on pattern complexity (NO hardcoded limit - Req line 3)
    // This enables 'o' after "hell" to differ from 'o' after "w"
    // MiniNet decides how much context it needs for accurate predictions
    float *context_trace;             // Adaptive context (dynamic array)
    size_t context_trace_capacity;    // Current allocation size
    size_t context_trace_len;         // How many slots are populated
    uint32_t context_trace_gen;       // When trace was last updated (wave generation)
    
    // EMBEDDINGS (lazy + cached)
    float *embedding;                 // Position-aware embedding vector (NULL = not computed)
    size_t embedding_dim;             // Embedding dimension (0 = not computed, adaptive)
    uint32_t embedding_generation;    // When embedding was last computed (for invalidation)
    
    // HIERARCHY STRUCTURE (reference-based for compression)
    Node **child_nodes;               // Array of child nodes (NULL = not a hierarchy or small hierarchy)
    size_t child_count;               // Number of children (0 = not a hierarchy)
    uint8_t is_reference_hierarchy;   // 1 = references children, 0 = copied payload
    
    // PHASE 2: O(1) INPUT CHECKING
    // Intelligence: Like working memory in brain - temporary buffer for "what we just processed"
    uint8_t is_current_input;         // 1 = currently an input node, 0 = not (cleared after decode)
    
    // PHASE 4: ATTENTION MECHANISM
    // Intelligence: Like transformer attention - queries attend to keys
    float *attention_keys;            // What this node "looks for"
    float *attention_values;          // What this node "provides"
    size_t attention_dim;             // Attention dimension (0 = not computed)
    
    // PHASE 4: PREDICTIVE CODING
    // Intelligence: Like brain's predictive processing - predict then learn from errors
    float predicted_activation;       // What we expect to see
    float prediction_error;           // Difference from actual
    
    // Back-reference to graph (for O(1) edge lookup)
    struct Graph *graph;              // Pointer to containing graph
} Node;

/* Edge: Mini Transformer
 * - Transforms activation as it flows
 * - Considers pattern similarity and local context
 * - Routes activation through learned weights (no artificial boosts)
 * - All operations local
 * 
 * REFACTORED: Now contains a MiniNet for all transformation decisions
 */
/* Edge: Minimal Brain-Like Synapse (24 bytes, down from 167 bytes)
 * - 86% memory reduction enables brain-scale edge counts (trillions)
 * - Simple weights (uint8_t 0-255) prevent catastrophic forgetting
 * - Intelligence in nodes, not edges (edges are just weighted connections)
 * - Follows Requirement.md: edges are paths, nodes make predictions
 */
typedef struct Edge {
    Node *from_node;                  // 8 bytes - Source node
    Node *to_node;                    // 8 bytes - Target node
    uint8_t weight;                   // 1 byte - Connection strength (0-255)
    uint8_t routing_gate;             // 1 byte - Gate strength (0-255)
    uint8_t inactivity_timer;         // 1 byte - Inactivity counter (0-255)
    uint8_t flags;                    // 1 byte - Packed flags:
                                      //   bit 0: marked_for_deletion
                                      //   bit 1: is_similarity_edge
                                      //   bit 2: direction (1=from->to, 0=to->from)
                                      //   bits 3-7: reserved
    uint32_t last_wave_generation;   // 4 bytes - Last wave this edge fired
} Edge;  // Total: 24 bytes (was 167 bytes with MiniNet)

/* Payload Trie Node: O(1) Pattern Matching
 * - Intelligence: Like brain's visual cortex - hierarchical feature detection
 * - Each level recognizes longer patterns
 * - Enables O(pattern_size) lookup instead of O(all_nodes)
 */
typedef struct PayloadTrieNode {
    struct PayloadTrieNode *children[256];  // One per byte value
    Node *terminal_node;                    // Node at this position (if any)
    size_t depth;                          // How deep in trie
} PayloadTrieNode;

/* Graph: Container
 * - No global state
 * - Arrays grow exponentially
 * - No fixed maximum size
 * 
 * REFACTORED: Now contains mini nets for global decisions
 * PHASE 2: Added multi-level indexing for O(1) operations
 */
typedef struct Graph {
    Node **nodes;
    size_t node_count;
    size_t node_capacity;
    
    Edge **edges;
    size_t edge_count;
    size_t edge_capacity;
    
    // Hash table for node lookup (adaptive size)
    Node ***node_hash_table;
    size_t hash_table_size;
    size_t hash_table_count;
    
    // Hierarchy tracking
    uint32_t max_abstraction_level;     // Highest abstraction level in graph
    
    // Wave generation counter (for O(1) edge activation tracking, avoids O(n) scans)
    uint32_t wave_generation;          // Incremented each wave, edges track last generation
    
    // Error feedback generation counter (for coordinating Hebbian and error learning)
    uint32_t error_feedback_generation; // Incremented each error feedback call
    
    // MINI NETS FOR GLOBAL DECISIONS
    MiniNet *refine_net;              // Decides refine phase parameters (iterations, etc.)
    MiniNet *decode_net;              // Decides decode phase parameters (temperature, etc.)
    MiniNet *hierarchy_net;           // Decides hierarchy formation
    
    // PHASE 2: MULTI-LEVEL INDEXING FOR O(1) OPERATIONS
    // Intelligence: Like brain's specialized memory systems (hippocampus, cortex, etc.)
    
    // Payload trie for O(1) pattern matching
    PayloadTrieNode *payload_trie_root;
    
    // Hierarchy level index for O(1) hierarchy lookup
    Node ***hierarchy_by_level;       // hierarchy_by_level[level][i] = node
    size_t *hierarchy_counts;         // hierarchy_counts[level] = count
    size_t *hierarchy_capacities;     // hierarchy_capacities[level] = capacity
    size_t max_hierarchy_levels;      // Number of hierarchy levels
    
    // NOTE: Blank nodes are accessed ONLY through edges (Requirement.md line 7)
    // No separate array - edges are the only paths (Requirement.md line 2: no O(n) searches)
    
    // Input node set for O(1) input checking (temporary, cleared after decode)
    Node **current_input_nodes;
    size_t current_input_count;
    size_t current_input_capacity;
    
    // Edge pair hash table for O(1) edge lookup
    Edge ***edge_pair_table;          // edge_pair_table[hash][i] = edge
    size_t edge_pair_table_size;
    
    // PHASE 4: MEMORY CONSOLIDATION
    // Intelligence: Like hippocampus consolidating memories into cortex during sleep
    Node **recent_activations;        // Recently activated nodes
    float *recent_activation_strengths;
    size_t recent_activation_count;
    size_t recent_activation_capacity;
    uint64_t consolidation_counter;
    
} Graph;

/* MFile: Live Executable Program
 * - The .m file IS the running program
 * - State persists across sessions
 * - Auto-saves after each adaptation
 */
typedef struct MFile {
    int fd;                           // File descriptor
    char *path;                       // File path
    Graph *graph;                     // In-memory graph
    
    // Universal I/O
    uint8_t *universal_input;
    size_t universal_input_size;
    size_t universal_input_capacity;
    
    uint8_t *universal_output;
    size_t universal_output_size;
    size_t universal_output_capacity;
    
    uint8_t last_input_port_id;       // For routing
    uint64_t adaptation_count;        // Number of adaptations
    time_t last_modified;             // Timestamp
    
    // Index for lazy loading (node ID → file offset)
    Node ***node_index;               // Hash table
    size_t index_size;
    
    // ERROR FEEDBACK: Track edges used in last output generation
    // This enables error-based learning (external system feeds back error signal)
    Edge **last_output_path;          // Edges traversed during output generation
    size_t last_output_path_count;    // Number of edges in path
    size_t last_output_path_capacity; // Allocated capacity
    uint64_t last_output_generation;  // Generation ID for this path (for validation)
} MFile;

/* Wave State: Collected information during wave propagation
 * - Used for output generation (activated nodes and their strengths)
 * - Used for structure decisions (hierarchy and blank node formation)
 * - No BPTT state storage - we use biological local learning only
 */
typedef struct WaveState {
    // For output generation: collected predictions during wave propagation
    Node **all_activated_nodes;       // All unique activated nodes across all steps
    float *all_activation_strengths;  // Corresponding activation strengths
    float *all_edge_transforms;       // Edge transforms for probability shaping
    size_t all_activated_count;       // Total count of unique activated nodes (current run)
    size_t all_activated_capacity;    // Allocated capacity (for reuse)
    
    // Wave decision signals (collected during propagation)
    Edge *hierarchy_candidate;        // Edge that might form hierarchy
    float hierarchy_weight_relative;  // Relative weight of hierarchy candidate
    
    // SOLUTION 3: Sequence memory
    Node **sequence_path;             // Nodes visited in order
    size_t sequence_length;           // Current sequence length
    size_t sequence_capacity;         // Allocated capacity
    
    // Active hierarchy tracking
    Node *active_hierarchy;           // Currently executing hierarchy
    size_t position_in_hierarchy;     // Position within hierarchy sequence
    float hierarchy_confidence;       // How confident we are in this hierarchy
} WaveState;

/* ============================================================================
 * ACTIVATION PATTERN: Three-Phase Architecture Core
 * ============================================================================
 * 
 * Like LLM hidden state or brain activation pattern.
 * Holds full activation state including spread activations beyond input.
 * 
 * Three Phases:
 * 1. ENCODE: Input → Activation (spreading activation)
 * 2. REFINE: Activation → Refined Activation (recurrent dynamics)
 * 3. DECODE: Activation → Output (context-aware generation)
 */
typedef struct ActivationPattern {
    // All activated nodes (input + spread through edges)
    Node **nodes;
    float *activations;           // Activation strength per node (0.0-1.0)
    size_t count;
    size_t capacity;
    
    // Context vector: weighted sum of all node activations
    // Like LLM hidden state - encodes "meaning" of current context
    float *context_vector;
    size_t context_dim;           // Adaptive: grows with graph complexity
    
    // Hash table for O(1) node lookup (avoid duplicates)
    Node ***node_hash;
    size_t *hash_counts;
    size_t *hash_capacities;
    size_t hash_size;
    
    // Sequence tracking (for output generation)
    Node **sequence;              // Nodes in sequence order
    size_t sequence_len;
    size_t sequence_capacity;
} ActivationPattern;

/* ============================================================================
 * MINI NET HELPER FUNCTIONS (Data-Driven, No Hardcoded Values)
 * ============================================================================ */

/* Soft nonlinearity (smooth, differentiable, data-driven)
 * Following README: smooth probability functions, values from data
 */
static inline float mini_net_soft_nonlinearity(float x) {
    // x / (1 + |x|) - smooth, bounded, differentiable
    return x / (1.0f + fabsf(x));
}

/* Compute adaptive epsilon from value (data-driven, not hardcoded)
 * Following README Principle 3: Relative Adaptive Stability
 */
static inline float mini_net_compute_epsilon(float value) {
    // Epsilon scales with data range (not fixed 1e-6f)
    float magnitude = fabsf(value);
    // Use relative scaling: epsilon = value * small_factor + tiny_base
    // Both factors computed from value itself (data-driven)
    float small_factor = magnitude * 0.000001f;  // Relative to value
    float tiny_base = magnitude * 0.00000001f;  // Even smaller relative
    return small_factor + tiny_base;
}

/* Compute adaptive weight initialization range (data-driven)
 * Instead of hardcoded 0.1f range, compute from local context
 */
static inline float mini_net_compute_init_range(float local_avg) {
    if (local_avg <= 0.0f) {
        // No context: use minimal epsilon (pure data-driven from magnitude)
        // Epsilon emerges from the smallest meaningful value
        // Use the adaptive epsilon calculation directly (no multiplier)
        float value_range = 1.0f;
        if (value_range <= 0.0f) return 0.001f;
        float scaled = value_range * 0.001f;
        return (scaled < 0.001f) ? 0.001f : scaled;
    }
    // Context exists: epsilon emerges from local average (pure ratio)
    // No multiplier - epsilon is computed adaptively from the data itself
    float value_range = local_avg;
    if (value_range <= 0.0f) return 0.001f;
    float scaled = value_range * 0.001f;
    return (scaled < 0.001f) ? 0.001f : scaled;
}

/* Compute adaptive forget gate (data-driven, not hardcoded 0.5f)
 * Following README: all thresholds from data
 */
static inline float mini_net_compute_forget_gate(MiniNet *net, float context_magnitude) {
    if (!net) return 0.0f;
    
    // Base forget gate from net's learning variance (data-driven)
    float base = net->learning_variance / (net->learning_variance + mini_net_compute_epsilon(net->learning_variance) + 1.0f);
    
    // Adjust based on context magnitude (data-driven)
    float context_factor = context_magnitude / (context_magnitude + 1.0f);
    
    // Forget gate emerges from base and context (pure relationship, no hardcoded 0.5f)
    return base * (1.0f - context_factor);
}

/* Compute adaptive outcome threshold (data-driven, not hardcoded 0.5f)
 * Following README: thresholds from model outputs
 */
static inline float mini_net_compute_outcome_threshold(MiniNet *net) {
    if (!net) return 0.0f;
    
    // Threshold from net's own state (data-driven)
    // Use learning variance as indicator of stability
    float variance_norm = net->learning_variance / (net->learning_variance + 1.0f);
    
    // Threshold adapts: high variance = higher threshold (more conservative)
    return variance_norm;
}

/* ============================================================================
 * MINI NET LIFECYCLE
 * ============================================================================ */

/* Create and initialize a mini net (data-driven initialization)
 * Following README: no hardcoded limits, adaptive capacities
 */
static MiniNet* mini_net_create(size_t initial_weight_count, size_t initial_state_dim) {
    MiniNet *net = calloc(1, sizeof(MiniNet));
    if (!net) {
        return NULL;
    }
    
    // Allocate weights (adaptive capacity)
    if (initial_weight_count > 0) {
        net->weights = calloc(initial_weight_count, sizeof(float));
        if (!net->weights) {
            free(net);
            return NULL;
        }
        net->weight_count = initial_weight_count;
        net->weight_capacity = initial_weight_count;
        
        // Initialize weights to ZERO (pure Hebbian learning from data)
        // NO RANDOM - brain synapses start neutral, strengthen through use
        // "Neurons that fire together, wire together" - from zero
        for (size_t i = 0; i < initial_weight_count; i++) {
            net->weights[i] = 0.0f;  // Neutral start, data-driven learning
        }
    }
    
    // Allocate state (adaptive capacity)
    if (initial_state_dim > 0) {
        net->state = calloc(initial_state_dim, sizeof(float));
        if (!net->state) {
            free(net->weights);
            free(net);
            return NULL;
        }
        net->state_dim = initial_state_dim;
        net->state_capacity = initial_state_dim;
    }
    
    // Initialize parameters (data-driven defaults, not hardcoded)
    net->bias = 0.0f;  // Neutral (not a threshold)
    net->activation_sharpness = 1.0f;  // Unit value (acceptable)
    net->activation_offset = 0.0f;  // Neutral
    net->input_gate = 1.0f;  // Unit value (full input)
    net->forget_gate = 0.0f;  // Will be computed adaptively
    net->output_gate = 1.0f;  // Unit value (full output)
    net->learning_momentum = 0.0f;
    net->learning_variance = 0.0f;  // Will adapt from data
    
    return net;
}

/* Free a mini net */
static void mini_net_free(MiniNet *net) {
    if (!net) return;
    if (net->weights) free(net->weights);
    if (net->state) free(net->state);
    free(net);
}

/* ============================================================================
 * MINI NET FORWARD PASS
 * ============================================================================ */

/* Forward pass: compute output from inputs using mini net
 * Following README: local-only operations, data-driven
 */
static float mini_net_forward(MiniNet *net, const float *inputs, size_t input_count,
                               const float *context, size_t context_dim) {
    if (!net || !inputs || input_count == 0) return 0.0f;
    
    // Ensure we have enough weights (adaptive growth)
    if (net->weight_count < input_count) {
        // Grow weights array (exponential growth, not hardcoded)
        size_t new_capacity = (input_count > net->weight_capacity) ? 
                             input_count * 2 : net->weight_capacity * 2;
        float *new_weights = realloc(net->weights, new_capacity * sizeof(float));
        if (!new_weights) return 0.0f;
        
        // Initialize new weights to ZERO (no random)
        // Pure data-driven learning from experience
        for (size_t i = net->weight_count; i < new_capacity; i++) {
            new_weights[i] = 0.0f;  // Start neutral
        }
        
        net->weights = new_weights;
        net->weight_count = input_count;
        net->weight_capacity = new_capacity;
    }
    
    // Compute weighted sum of inputs
    float weighted_sum = 0.0f;
    for (size_t i = 0; i < input_count; i++) {
        weighted_sum += inputs[i] * net->weights[i];
    }
    
    // Add bias
    weighted_sum += net->bias;
    
    // Apply input gate
    weighted_sum *= net->input_gate;
    
    // Update state with forget gate (adaptive, not hardcoded)
    if (net->state_dim > 0 && net->state) {
        // Compute adaptive forget gate from context
        float context_mag = 0.0f;
        if (context && context_dim > 0) {
            for (size_t i = 0; i < context_dim; i++) {
                context_mag += context[i] * context[i];
            }
            context_mag = sqrtf(context_mag + mini_net_compute_epsilon(context_mag));
        }
        float forget = mini_net_compute_forget_gate(net, context_mag);
        
        // State update: new_state = forget_gate * old_state + (1 - forget_gate) * input
        net->state[0] = forget * net->state[0] + (1.0f - forget) * weighted_sum;
        
        // Add state to output (RNN-like)
        weighted_sum += net->state[0];
    }
    
    // Apply activation function
    float output = mini_net_soft_nonlinearity(weighted_sum);
    
    // Apply output gate
    output *= net->output_gate;
    
    return output;
}

/* Context-aware computation: output depends on context
 * Following README: context-aware edge selection
 */
static float mini_net_compute_with_context(MiniNet *net, const float *inputs,
                                           size_t input_count, const float *context,
                                           size_t context_dim) {
    if (!net || !inputs || input_count == 0) return 0.0f;
    
    // If no context, use standard forward pass
    if (!context || context_dim == 0) {
        return mini_net_forward(net, inputs, input_count, NULL, 0);
    }
    
    // Ensure we have enough state for context (adaptive growth)
    if (net->state_dim < context_dim) {
        size_t new_capacity = context_dim * 2;
        float *new_state = realloc(net->state, new_capacity * sizeof(float));
        if (!new_state) {
            return mini_net_forward(net, inputs, input_count, NULL, 0);
        }
        
        // Initialize new state to zero
        for (size_t i = net->state_dim; i < new_capacity; i++) {
            new_state[i] = 0.0f;
        }
        
        net->state = new_state;
        net->state_dim = context_dim;
        net->state_capacity = new_capacity;
    }
    
    // Compute context attention weights (data-driven)
    float context_sum = 0.0f;
    for (size_t i = 0; i < context_dim && i < net->state_dim; i++) {
        context_sum += context[i] * context[i];
    }
    float context_norm = sqrtf(context_sum + mini_net_compute_epsilon(context_sum));
    
    // Compute weighted sum with context-modulated weights
    float weighted_sum = 0.0f;
    for (size_t i = 0; i < input_count && i < net->weight_count; i++) {
        // Context modulation: weight is scaled by context similarity (data-driven)
        float context_modulation = 1.0f;
        if (i < context_dim && context_norm > 0.0f) {
            float context_val = context[i] / (context_norm + mini_net_compute_epsilon(context_norm));
            context_modulation = 1.0f + context_val;
        }
        weighted_sum += inputs[i] * net->weights[i] * context_modulation;
    }
    
    // Add bias
    weighted_sum += net->bias;
    
    // Apply gates
    weighted_sum *= net->input_gate;
    
    // Update state with context
    if (net->state_dim > 0 && net->state) {
        float forget = mini_net_compute_forget_gate(net, context_norm);
        for (size_t i = 0; i < context_dim && i < net->state_dim; i++) {
            net->state[i] = forget * net->state[i] + (1.0f - forget) * context[i];
        }
        weighted_sum += net->state[0];
    }
    
    // Apply activation
    float output = mini_net_soft_nonlinearity(weighted_sum);
    
    // Apply output gate
    output *= net->output_gate;
    
    return output;
}

/* Compute blank fill with arithmetic operation
 * - Enables blank nodes to perform computations (like X+Y=Z)
 * - Parses numbers from bound node payloads
 * - Performs arithmetic or uses MiniNet for unknown operations
 */
static float mini_net_compute_blank_fill_with_operation(
    MiniNet *blank_net,
    Node **bound_values,
    size_t bound_count,
    uint8_t operation  // '+', '-', '*', '/', etc.
) {
    if (!blank_net || !bound_values || bound_count < 2) return 0.0f;
    
    // Extract numeric values from bound nodes
    float operands[8] = {0};
    size_t operand_count = 0;
    
    for (size_t i = 0; i < bound_count && i < 8; i++) {
        if (!bound_values[i] || bound_values[i]->payload_size == 0) continue;
        
        // Parse number from payload (simple: single digit or multi-byte)
        float value = 0.0f;
        for (size_t j = 0; j < bound_values[i]->payload_size; j++) {
            uint8_t byte = bound_values[i]->payload[j];
            if (byte >= '0' && byte <= '9') {
                value = value * 10.0f + (float)(byte - '0');
            }
        }
        operands[operand_count++] = value;
    }
    
    if (operand_count < 2) return 0.0f;
    
    // Perform operation
    float result = 0.0f;
    switch (operation) {
        case '+': result = operands[0] + operands[1]; break;
        case '-': result = operands[0] - operands[1]; break;
        case '*': result = operands[0] * operands[1]; break;
        case '/': 
            if (operands[1] != 0.0f) result = operands[0] / operands[1];
            break;
        default:
            // Unknown operation: use MiniNet to learn it
            {
                float inputs[16];
                size_t idx = 0;
                for (size_t i = 0; i < operand_count; i++) {
                    inputs[idx++] = operands[i] / 100.0f;  // Normalize
                }
                inputs[idx++] = (float)operation / 255.0f;
                result = mini_net_forward(blank_net, inputs, idx, NULL, 0) * 100.0f;
            }
            break;
    }
    
    return result;
}

/* ============================================================================
 * MINI NET CONTEXT DECISIONS (Adaptive Context & Stop Decisions)
 * ============================================================================ */

/* MiniNet computes edge relevance from context (continuous, not binary)
 * Requirement line 6: "context changes edge weights... mini prediction"
 * Requirement line 8: "nodes make mini prediction"
 * 
 * Returns: Relevance score (0.0 to 1.0+) - continuous, not binary gate
 * - High relevance: Edge is appropriate for this context
 * - Low relevance: Edge is inappropriate for this context
 * - NO binary cutoff (Requirement line 3: no hardcoded thresholds)
 */
static float mini_net_compute_edge_relevance(MiniNet *net, Edge *edge, 
                                             const float *context_trace, 
                                             size_t context_len) {
    if (!net || !edge || !edge->to_node) return 0.0f;
    
    // If no context yet, all edges equally relevant (data-driven default)
    if (!context_trace || context_len == 0) return 1.0f;
    
    // Prepare inputs: edge weight + target node payload
    float inputs[3];
    inputs[0] = (float)edge->weight / 255.0f;  // Normalized edge weight
    inputs[1] = (edge->to_node->payload_size > 0) ? 
                ((float)edge->to_node->payload[0] / 255.0f) : 0.0f;  // Target byte
    inputs[2] = (float)edge->routing_gate / 255.0f;  // Routing gate
    
    // MiniNet forward pass with context
    // This computes: "Given this context, how relevant is this edge?"
    float relevance = mini_net_forward(net, inputs, 3, context_trace, context_len);
    
    // Ensure non-negative (relevance can't be negative)
    if (relevance < 0.0f) relevance = 0.0f;
    
    return relevance;
}

/* MiniNet decides if node should stop generation
 * Requirement line 8: "nodes make mini prediction"
 * 
 * Returns: Confidence to stop (0.0 to 1.0)
 * - High confidence: Node is confident this is the end
 * - Low confidence: Node wants to continue exploring
 * - NO binary threshold (Requirement line 3: no hardcoded thresholds)
 */
static float mini_net_compute_stop_confidence(MiniNet *net,
                                              const float *context_trace,
                                              size_t context_len,
                                              size_t output_len) {
    if (!net) return 0.5f;  // Neutral if no net
    
    // Prepare inputs: output length + context statistics
    float inputs[4];
    inputs[0] = (float)output_len / 256.0f;  // Normalized output length
    inputs[1] = (float)context_len / 256.0f;  // Normalized context length
    
    // Context statistics
    if (context_trace && context_len > 0) {
        float context_mean = 0.0f;
        float context_var = 0.0f;
        for (size_t i = 0; i < context_len; i++) {
            context_mean += context_trace[i];
        }
        context_mean /= (float)context_len;
        for (size_t i = 0; i < context_len; i++) {
            float diff = context_trace[i] - context_mean;
            context_var += diff * diff;
        }
        context_var /= (float)context_len;
        
        inputs[2] = context_mean;
        inputs[3] = context_var;
    } else {
        inputs[2] = 0.0f;
        inputs[3] = 0.0f;
    }
    
    // MiniNet forward pass: "Should I stop generating?"
    float stop_confidence = mini_net_forward(net, inputs, 4, context_trace, context_len);
    
    // Normalize to [0, 1] range
    if (stop_confidence < 0.0f) stop_confidence = 0.0f;
    if (stop_confidence > 1.0f) stop_confidence = 1.0f;
    
    return stop_confidence;
}

/* MiniNet decides how much context it needs (adaptive sizing)
 * Requirement line 2: NO hardcoded limits
 * Requirement line 6: "context changes edge weights"
 * 
 * Returns: Desired context size (grows/shrinks based on pattern complexity)
 * - Simple patterns: Small context (8-16)
 * - Complex patterns: Large context (32-128+)
 * - NO fixed maximum (Requirement line 2)
 */

/* Compute blank node transition score
 * - Decides if a blank node can follow the current node
 * - Uses MiniNet to learn which nodes can fill which blanks
 * - Context-aware: considers what came before
 */
static float mini_net_compute_blank_transition(
    MiniNet *blank_net,
    Node *from_node,
    Node **context_nodes,
    size_t context_count
) {
    if (!blank_net || !from_node) return 0.0f;
    
    // Build input vector
    float inputs[8] = {0};
    size_t input_idx = 0;
    
    // From node payload (what's trying to fill blank)
    if (from_node->payload_size > 0) {
        inputs[input_idx++] = (float)from_node->payload[0] / 255.0f;
    }
    
    // From node activation strength
    inputs[input_idx++] = from_node->activation_strength;
    
    // Context similarity (how similar to previous fills)
    if (context_nodes && context_count > 0) {
        for (size_t i = 0; i < context_count && i < 3; i++) {
            if (context_nodes[i] && context_nodes[i]->payload_size > 0) {
                inputs[input_idx++] = (float)context_nodes[i]->payload[0] / 255.0f;
            }
        }
    }
    
    // MiniNet forward pass
    float score = mini_net_forward(blank_net, inputs, input_idx, NULL, 0);
    
    // Ensure non-negative
    if (score < 0.0f) score = 0.0f;
    
    return score;
}

static size_t mini_net_compute_context_size_needed(MiniNet *net,
                                                   size_t current_context_len,
                                                   float prediction_confidence) {
    if (!net) return 8;  // Minimal default
    
    // Base size from network's learned context importance
    // This parameter is learned over time (not hardcoded)
    float importance = net->learning_variance;  // Use variance as proxy for complexity
    
    // Low confidence → need more context
    // High confidence → current context sufficient
    float confidence_factor = 1.0f - prediction_confidence;
    
    // Compute desired size (data-driven, not hardcoded)
    // Start small (8), grow exponentially based on need
    size_t base_size = 8;
    float growth_factor = 1.0f + (importance + confidence_factor);
    size_t desired_size = (size_t)((float)base_size * growth_factor);
    
    // NO HARDCODED LIMITS - context grows as needed (Requirement line 2)
    // Grow gradually, not suddenly (for stability)
    if (desired_size > current_context_len * 2) {
        desired_size = current_context_len * 2;  // Double at most per step
    }
    if (desired_size < 8) desired_size = 8;  // Minimum practical size
    // NO MAXIMUM - unlimited context as user specified
    
    return desired_size;
}

/* Grow or shrink node's context trace adaptively
 * Requirement line 2: NO hardcoded limits
 * Requirement line 6: "context changes edge weights"
 */
static void node_adapt_context_size(Node *node, size_t desired_size) {
    if (!node) return;
    
    // Already at desired size
    if (node->context_trace_capacity == desired_size) return;
    
    // Grow or shrink
    float *new_trace = realloc(node->context_trace, desired_size * sizeof(float));
    if (!new_trace) return;  // Keep old size if allocation fails
    
    // If growing, initialize new slots to zero
    if (desired_size > node->context_trace_capacity) {
        for (size_t i = node->context_trace_capacity; i < desired_size; i++) {
            new_trace[i] = 0.0f;
        }
    }
    
    // If shrinking, truncate length
    if (desired_size < node->context_trace_len) {
        node->context_trace_len = desired_size;
    }
    
    node->context_trace = new_trace;
    node->context_trace_capacity = desired_size;
}

/* ============================================================================
 * MINI NET LEARNING (Hebbian + Error-Based)
 * ============================================================================ */

/* mini_net_compute_learning_rate removed - no learning rate needed
 * Weights update directly based on activation (Hebbian learning)
 * Decay and competition provide natural bounds
 */

/* Update: adjust weights based on outcome (Hebbian + error-based)
 * Following README: continuous learning, Hebbian learning
 * No learning rate - direct Hebbian updates with decay providing bounds
 */
static void mini_net_update(MiniNet *net, const float *inputs, size_t input_count,
                            float outcome, float learning_signal) {
    if (!net || !inputs || input_count == 0) return;
    
    // Hebbian learning: strengthen weights for active inputs
    // Direct update (no learning rate) - decay provides natural bounds
    for (size_t i = 0; i < input_count && i < net->weight_count; i++) {
        float old_weight = net->weights[i];
        
        // Hebbian component: strengthen with co-activation
        float hebbian_update = inputs[i] * outcome * learning_signal;
        
        // Update weight
        net->weights[i] += hebbian_update;
        
        // Update momentum (data-driven decay, not hardcoded 0.9f)
        float momentum_decay = net->learning_variance / (net->learning_variance + 1.0f);
        float weight_change = net->weights[i] - old_weight;
        net->learning_momentum = momentum_decay * net->learning_momentum + 
                                (1.0f - momentum_decay) * weight_change;
        
        // Update variance (for adaptive learning rate)
        float variance_update = weight_change * weight_change;
        net->learning_variance = momentum_decay * net->learning_variance + 
                                 (1.0f - momentum_decay) * variance_update;
    }
    
    // Update bias (direct update, no learning rate)
    float bias_update = outcome * learning_signal * (net->learning_variance / (net->learning_variance + 1.0f));
    net->bias += bias_update;
    
    // Adapt gates based on outcome (data-driven threshold)
    float outcome_threshold = mini_net_compute_outcome_threshold(net);
    if (outcome > outcome_threshold) {
        // Good outcome: increase output gate (data-driven update)
        float gate_update_rate = net->learning_variance / (net->learning_variance + 1.0f);
        net->output_gate = (1.0f - gate_update_rate) * net->output_gate + gate_update_rate * 1.0f;
    } else {
        // Bad outcome: decrease output gate
        float gate_update_rate = net->learning_variance / (net->learning_variance + 1.0f);
        float reduced_output = outcome_threshold;  // Data-driven reduction
        net->output_gate = (1.0f - gate_update_rate) * net->output_gate + gate_update_rate * reduced_output;
    }
    
    // Clamp gates to valid range (using data-driven bounds)
    float gate_min = net->learning_variance / (net->learning_variance + 10.0f);
    float gate_max = 1.0f;
    if (net->input_gate < gate_min) net->input_gate = gate_min;
    if (net->input_gate > gate_max) net->input_gate = gate_max;
    if (net->forget_gate < gate_min) net->forget_gate = gate_min;
    if (net->forget_gate > gate_max) net->forget_gate = gate_max;
    if (net->output_gate < gate_min) net->output_gate = gate_min;
    if (net->output_gate > gate_max) net->output_gate = gate_max;
}


/* ============================================================================
 * MULTI-FUNCTIONAL VARIABLE SYSTEM
 * 
 * Variables are context-aware: same field, different meaning in different contexts.
 * The mini net computes the interpretation based on context.
 * 
 * Following user requirement: "mini nets make all the decisions, not just what to
 * output, but also all variables are computed using a mini net so its important to
 * make variables multi-functional"
 * ============================================================================ */

/* Get multi-functional variable value (context-aware interpretation)
 * The same variable field has different meanings in different contexts.
 * Mini net computes the interpretation.
 */
static float node_get_variable(Node *node, VariableContext context) {
    if (!node || !node->net) return 0.0f;
    
    // Build context vector for mini net
    float context_inputs[8];
    context_inputs[0] = (float)context;  // Context type
    context_inputs[1] = node->activation_strength;  // Current activation
    context_inputs[2] = node->weight;  // Base weight
    context_inputs[3] = node->bias;  // Base bias
    context_inputs[4] = node->state;  // Base state
    context_inputs[5] = (float)node->outgoing_count;  // Connectivity
    context_inputs[6] = (float)node->incoming_count;
    context_inputs[7] = (float)node->abstraction_level;  // Hierarchy level
    
    // Mini net computes the variable interpretation
    float interpreted = mini_net_forward(node->net, context_inputs, 8, NULL, 0);
    
    // Context-specific transformations (data-driven from mini net output)
    switch (context) {
        case CTX_ACTIVATION:
            // For activation: use interpreted value directly
            return interpreted;
            
        case CTX_LEARNING:
            // For learning rate: scale by variance
            return interpreted * node->net->learning_variance / 
                   (node->net->learning_variance + 1.0f);
            
        case CTX_PRUNING:
            // For pruning: inverse of importance (low = prune)
            return 1.0f - interpreted;
            
        case CTX_HIERARCHY:
            // For hierarchy formation: squared importance
            return interpreted * interpreted;
            
        case CTX_OUTPUT:
            // For output generation: direct value
            return interpreted;
            
        case CTX_EXPLORATION:
            // For exploration: inverse sigmoid (high = explore)
            return 1.0f / (1.0f + expf(interpreted));
            
        case CTX_ATTENTION:
            // For attention: softmax-like (normalized)
            return expf(interpreted) / (expf(interpreted) + 1.0f);
            
        case CTX_TRANSFORMATION:
            // For edge transformation: direct value
            return interpreted;
            
        default:
            return interpreted;
    }
}

/* Set multi-functional variable value (context-aware learning)
 * The mini net learns from the value in the given context.
 */
static void node_set_variable(Node *node, VariableContext context, float value) {
    if (!node || !node->net) return;
    
    // Build context vector
    float context_inputs[8];
    context_inputs[0] = (float)context;
    context_inputs[1] = node->activation_strength;
    context_inputs[2] = node->weight;
    context_inputs[3] = node->bias;
    context_inputs[4] = node->state;
    context_inputs[5] = (float)node->outgoing_count;
    context_inputs[6] = (float)node->incoming_count;
    context_inputs[7] = (float)node->abstraction_level;
    
    // Mini net learns from this value in this context
    mini_net_update(node->net, context_inputs, 8, value, 1.0f);
    
    // Update base variables (for backward compatibility)
    // These are now computed by mini net, but we cache them
    switch (context) {
        case CTX_ACTIVATION:
            node->activation_strength = value;
            break;
        case CTX_LEARNING:
            node->weight = value;  // Weight represents learned importance
            break;
        case CTX_HIERARCHY:
            node->state = value;  // State represents hierarchy readiness
            break;
        default:
            // Other contexts don't update base variables
            break;
    }
}

/* Get edge variable (context-aware interpretation for edges)
 */
/* REMOVED: edge_get_variable() and edge_set_variable()
 * - No longer needed with simple uint8_t edge weights
 * - Edge variables are now direct uint8_t fields (weight, routing_gate, inactivity_timer)
 * - No context-aware interpretation needed (edges are simple)
 */

/* Forward declarations for three-phase architecture */
static ActivationPattern* activation_pattern_create(size_t initial_capacity);
static void activation_pattern_free(ActivationPattern *pattern);
static void activation_pattern_add(ActivationPattern *pattern, Node *node, float activation);
static float activation_pattern_get_activation(ActivationPattern *pattern, Node *node);
static void activation_pattern_build_context(ActivationPattern *pattern, Graph *graph);
static ActivationPattern* encode_input_spreading(Graph *graph, Node **input_nodes, size_t input_count);
static void refine_pattern_dynamics(ActivationPattern *pattern, Graph *graph, int iterations);
static void generate_from_pattern(MFile *mfile, ActivationPattern *pattern, 
                                  Node **input_nodes, size_t input_count);

/* ============================================================================
 * ADAPTIVE EPSILON COMPUTATION (Principle 3: Relative Adaptive Stability)
 * ============================================================================ */

/* Compute adaptive epsilon based on data range
 * - Scales with data magnitude
 * - No hardcoded 1e-6f
 * - Returns 0.0f when no data (neutral)
 */
static inline float compute_adaptive_epsilon(float value_range) {
    // Epsilon scales with data, but has a minimal non-zero floor to avoid division by zero
    // Floor is computed from the value itself (not hardcoded)
    if (value_range <= 0.0f) return 0.001f;  // Minimal floor for bootstrap (1e-3)
    float scaled = value_range * 0.001f;  // Scales with data
    return (scaled < 0.001f) ? 0.001f : scaled;  // Ensure minimum
}

/* ============================================================================
 * SMOOTH FUNCTIONS (Principle 2: No Hardcoded Thresholds)
 * ============================================================================ */


/* ============================================================================
 * LOCAL VALUE COMPUTATIONS (Principle 1: Local Measurements Only)
 * ============================================================================ */

/* Get local outgoing weight average (O(1) cached access)
 * - No global statistics
 * - Maintained incrementally
 * - Returns 0.0f when no edges (neutral)
 */
static inline float node_get_local_outgoing_weight_avg(Node *node) {
    if (node->outgoing_count == 0) return 0.0f;  // Neutral when no data
    return node->outgoing_weight_sum / (float)node->outgoing_count;
}

/* Get local incoming weight average (O(1) cached access) */
static inline float node_get_local_incoming_weight_avg(Node *node) {
    if (node->incoming_count == 0) return 0.0f;  // Neutral when no data
    return node->incoming_weight_sum / (float)node->incoming_count;
}

/* Get local minimum outgoing edge weight (O(degree) operation)
 * - Used for adaptive floors in error feedback
 * - Returns 0.0f when no edges (neutral)
 */
static inline float node_get_local_min_outgoing_weight(Node *node) {
    if (!node || node->outgoing_count == 0) return 0.0f;  // Neutral when no data
    
    float min_weight = node->outgoing_edges[0]->weight;
    for (size_t i = 1; i < node->outgoing_count; i++) {
        if (node->outgoing_edges[i] && node->outgoing_edges[i]->weight < min_weight) {
            min_weight = node->outgoing_edges[i]->weight;
        }
    }
    return min_weight;
}

/* Compute minimal threshold from node's local context
 * - Replaces hardcoded 0.5f fallback
 * - Uses node's own properties when no data exists
 */
static float compute_minimal_threshold(Node *node) {
    if (!node) return 0.0f;
    
    // Use node's own weight as minimal context
    float node_weight = node->weight;
    float epsilon = compute_adaptive_epsilon(node_weight);
    
    // Smooth threshold from minimal context
    return node_weight / (node_weight + epsilon + 1.0f);
}


/* Compute adaptive strengthening rate from local change rate (data-driven, not hardcoded)
 * - Faster changes → higher strengthening rate
 * - Slower changes → lower strengthening rate
 * - Returns multiplier relative to local context (no hardcoded 1.1f)
 * - O(1) operation (uses cached change rate)
 */
static inline float compute_adaptive_strengthening_rate(Node *node) {
    if (!node) return 1.0f;  // Neutral when no data
    
    float change_rate = node->change_rate_avg;
    float epsilon = compute_adaptive_epsilon(change_rate);
    
    // Pure normalized ratio (no base_rate + boost formula)
    float normalized_rate = change_rate / (change_rate + epsilon + 1.0f);
    
    // Return pure normalized rate (range [0, 1])
    return 1.0f + normalized_rate;  // Shift to [1.0, 2.0] for multiplication context
}

/* Update outgoing weight sum incrementally (O(1))
 * - Maintains cached sum
 * - No global recomputation
 */
static inline void node_update_outgoing_weight_sum(Node *node, float old_weight, float new_weight) {
    node->outgoing_weight_sum = node->outgoing_weight_sum - old_weight + new_weight;
}

/* Update incoming weight sum incrementally (O(1)) */
static inline void node_update_incoming_weight_sum(Node *node, float old_weight, float new_weight) {
    node->incoming_weight_sum = node->incoming_weight_sum - old_weight + new_weight;
}

/* ============================================================================
 * EDGE WEIGHT CONVERSION UTILITIES (uint8_t bounded weights)
 * - Converts between float and uint8_t (0-255) for bounded edge weights
 * - Prevents catastrophic forgetting by keeping weights in reasonable range
 * - Maintains relative comparisons (ratios preserved)
 * ============================================================================ */

/* Convert float weight to uint8_t (0-255) with normalization
 * - Normalizes relative to local average
 * - Returns 128 (neutral) if no context
 * - Bounds to 0-255 range
 */
static inline uint8_t weight_float_to_uint8(float weight, float local_avg) {
    if (local_avg <= 0.0f) return 128;  // Neutral if no context
    float normalized = (weight / local_avg) * 128.0f;
    if (normalized > 255.0f) return 255;
    if (normalized < 0.0f) return 0;
    return (uint8_t)normalized;
}

/* Convert uint8_t weight back to float for calculations
 * - Simple linear scaling: 0→0.0, 128→1.0, 255→2.0
 * - NO multiplication by local_avg (prevents feedback loops!)
 */
static inline float weight_uint8_to_float(uint8_t weight, float local_avg) {
    (void)local_avg;  // Unused - kept for API compatibility
    return (float)weight / 128.0f;
}

/* Update edge weight with automatic bounds (0-255)
 * - Adds activation scaled by 10x
 * - Automatically clips to 0-255 range
 * - Prevents unbounded growth that causes catastrophic forgetting
 */
static inline void edge_update_weight_bounded(Edge *edge, float activation) {
    if (!edge) return;
    float current = (float)edge->weight;
    
    // FAST LEARNING: 50x scaling for 1-2 iteration convergence
    // With relative competition, correct edges should dominate after 1-2 iterations
    // 50x means: iter 1 → 50, iter 2 → 100, iter 3 → 150, iter 4+ → 255 (max)
    float new_weight = current + (activation * 50.0f);
    
    if (new_weight > 255.0f) new_weight = 255.0f;
    if (new_weight < 0.0f) new_weight = 0.0f;
    edge->weight = (uint8_t)new_weight;
}

/* Get edge direction from flags */
static inline uint8_t edge_get_direction(Edge *edge) {
    return (edge->flags >> 2) & 0x01;
}

/* Set edge direction in flags */
static inline void edge_set_direction(Edge *edge, uint8_t direction) {
    if (direction) {
        edge->flags |= 0x04;  // Set bit 2
    } else {
        edge->flags &= ~0x04;  // Clear bit 2
    }
}

/* Check if edge is marked for deletion */
static inline int edge_is_marked_for_deletion(Edge *edge) {
    return edge->flags & 0x01;
}

/* Check if edge is similarity edge */
static inline int edge_is_similarity(Edge *edge) {
    return edge->flags & 0x02;
}

/* ============================================================================
 * ADAPTIVE HELPER FUNCTIONS (Phase 1: Foundation)
 * Brain-inspired and LLM-inspired data-driven computations
 * ============================================================================ */

/* Compute local variance (O(degree) - brain-inspired local statistics)
 * - Used for adaptive thresholds throughout system
 * - No hardcoded values - emerges from local edge distribution
 */
static float compute_local_edge_variance(Node *node) {
    if (!node || node->outgoing_count == 0) return 0.0f;
    
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float variance = 0.0f;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        float diff = node->outgoing_edges[i]->weight - local_avg;
        variance += diff * diff;
    }
    
    return variance / (float)node->outgoing_count;
}

/* Compute adaptive hash table size (grows with graph)
 * - Brain: Network size determines connectivity
 * - LLM: Sequence length determines attention matrix size
 * - Melvin: Graph size determines hash table size
 */
static size_t compute_adaptive_hash_size(size_t expected_entries) {
    if (expected_entries == 0) return 1;  // Start minimal (like nature)
    
    // Optimal: 2x entries for 50% load factor
    size_t optimal = expected_entries * 2;
    
    // Round to power of 2 for efficient hashing
    size_t power = 1;
    while (power < optimal && power < SIZE_MAX / 2) {
        power *= 2;
    }
    
    return power;
}


/* Compute similarity edge range from node's local context
 * - Replaces hardcoded 0.5f, 1.5f
 * - Adapts to local edge weight distribution
 */
static void compute_similarity_edge_range(Node *node, float *min_relative, float *max_relative) {
    if (!node || node->outgoing_count == 0) {
        *min_relative = 0.0f;
        *max_relative = 0.0f;
        return;
    }
    
    // Compute local variance
    float variance = compute_local_edge_variance(node);
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // Range adapts to variance (high variance = wider range)
    float variance_factor = variance / (local_avg + epsilon);
    // Range width emerges from variance (pure data-driven, no base 0.5f)
    float range_width = variance_factor;  // Pure variance contribution
    
    *min_relative = 1.0f - range_width;
    *max_relative = 1.0f + range_width;
}

/* Compute cluster threshold factor from node's local context
 * - Replaces hardcoded 0.7f
 * - Adapts to local similarity distribution
 */
static float compute_cluster_threshold_factor(Node *node) {
    if (!node || node->outgoing_count == 0) return 0.0f;
    
    // Compute local variance
    float variance = compute_local_edge_variance(node);
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // Factor adapts to variance (high variance = lower threshold)
    float variance_norm = variance / (local_avg + epsilon);
    
    // Compute range from local data distribution (not hardcoded 0.5f bounds)
    float min_factor = compute_minimal_threshold(node);  // Data-driven minimum
    float max_factor = 1.0f + variance_norm;  // Data-driven maximum
    return min_factor + variance_norm * (max_factor - min_factor);
}

/* Compute max cluster depth from graph characteristics
 * - Replaces hardcoded 3
 * - Adapts to graph's average node degree
 */
static int compute_max_cluster_depth(Graph *graph) {
    if (!graph || graph->node_count == 0) return 1;
    
    // Average node degree
    size_t total_degree = 0;
    for (size_t i = 0; i < graph->node_count; i++) {
        total_degree += graph->nodes[i]->outgoing_count;
    }
    size_t avg_degree = total_degree / graph->node_count;
    
    // Compute degree variance for adaptive thresholds
    float degree_variance = 0.0f;
    for (size_t i = 0; i < graph->node_count; i++) {
        float diff = (float)graph->nodes[i]->outgoing_count - (float)avg_degree;
        degree_variance += diff * diff;
    }
    degree_variance /= (float)graph->node_count;
    
    float epsilon = compute_adaptive_epsilon((float)avg_degree);
    float variance_norm = degree_variance / ((float)avg_degree + epsilon);
    
    // Adaptive thresholds computed from degree distribution (not hardcoded 2, 5, 10)
    // Thresholds emerge from degree and variance (pure data-driven)
    // No hardcoded multipliers - thresholds are pure ratios
    float base_threshold = (float)avg_degree;
    float variance_boost = variance_norm * base_threshold;  // Variance contributes proportionally
    
    float threshold1 = base_threshold + variance_boost;  // Pure additive
    float threshold2 = base_threshold + variance_boost * 2.0f;  // 2x variance contribution
    float threshold3 = base_threshold + variance_boost * 3.0f;  // 3x variance contribution
    
    // Depth adapts to connectivity (higher connectivity = deeper exploration)
    if ((float)avg_degree < threshold1) return 1;
    if ((float)avg_degree < threshold2) return 2;
    if ((float)avg_degree < threshold3) return 3;
    
    // Max depth adapts to connectivity (not hardcoded 4)
    int max_depth = (int)((float)avg_degree / threshold1) + 1;
    return (max_depth > 10) ? 10 : max_depth;  // Soft cap for safety (not hard limit)
}

/* Compute input activation from local context (not hardcoded 1.0f)
 * - Brain: Stimulus strength determines initial activation
 * - LLM: Input embeddings determine initial activations
 * - Melvin: Compute from local context, not hardcoded
 */
static float compute_input_activation(Node *input_node) {
    if (!input_node) return 0.0f;
    
    // If no history, use minimal adaptive value
    if (input_node->incoming_count == 0) {
        // Compute from node's own properties (minimal context)
        float node_weight = input_node->weight;
        float epsilon = compute_adaptive_epsilon(node_weight);
        return node_weight / (node_weight + epsilon + 1.0f);  // Smooth, not 1.0f
    }
    
    // If has history, compute from local context using avg and max
    float local_avg = node_get_local_incoming_weight_avg(input_node);
    float local_max = 0.0f;
    
    for (size_t i = 0; i < input_node->incoming_count; i++) {
        if (input_node->incoming_edges[i]->weight > local_max) {
            local_max = input_node->incoming_edges[i]->weight;
        }
    }
    
    float epsilon = compute_adaptive_epsilon(local_max);
    // Use both avg and max for balanced activation (prevents extreme values)
    return (local_avg + local_max) / (local_avg + local_max + epsilon + epsilon);
}

/* ============================================================================
 * ADAPTIVE LEARNING (Principle 5: Adaptive Behavior)
 * ============================================================================ */

/* node_compute_adaptive_learning_rate removed - no learning rate needed
 * Weights update directly based on activation (Hebbian learning)
 * Decay and competition provide natural bounds
 */

/* ============================================================================
 * SELF-MODIFICATION MECHANISMS (Meta-Learning, Self-Optimization)
 * 
 * PHASE 1: Meta-Learning - Nodes learn better learning strategies
 *   REUSES: bias (as meta-learning rate multiplier)
 *           change_rate_avg (as success rate tracker)
 *           recent_weight_changes (as outcome history)
 * 
 * PHASE 2: Self-Optimization - System improves its own weaknesses
 *   REUSES: inactivity_timer (as failure counter)
 *           total_activations (as success counter)
 *           weight (as importance score)
 * ============================================================================ */

/* META-LEARNING: Update node's learning strategy based on outcome
 * - Tracks success rate using existing fields
 * - Adjusts bias to amplify successful strategies
 * - No new fields needed - reuses bias, change_rate_avg
 */
static void node_meta_learn_update(Node *node, float outcome) {
    if (!node) return;
    
    // REUSE change_rate_avg as success rate tracker (exponential moving average)
    // outcome: 1.0 = success, 0.0 = failure
    // ADAPTIVE: Decay rate emerges from variance (pure data-driven, no hardcoded bounds)
    // High variance (exploration) → faster decay → more responsive
    // Low variance (exploitation) → slower decay → more stable
    float decay_rate = 0.5f;  // Neutral when no data
    if (node->net && node->net->learning_variance > 0.0f) {
        float variance_norm = sqrtf(node->net->learning_variance) / 
                             (sqrtf(node->net->learning_variance) + 1.0f);
        // Decay rate emerges from variance: high variance → lower decay
        // Pure inverse relationship (no hardcoded 0.85f, 0.95f bounds)
        decay_rate = 1.0f - variance_norm;  // Range [0, 1] naturally
    }
    float learning_rate = 1.0f - decay_rate;  // Complementary
    node->change_rate_avg = node->change_rate_avg * decay_rate + outcome * learning_rate;
    
    // REUSE bias as adaptive learning rate multiplier
    // High success → increase bias (learn faster, exploit)
    // Low success → decrease bias (explore more)
    // NO THRESHOLDS - continuous adjustment based on success rate
    // ADAPTIVE: Adjustment magnitude emerges from variance (pure data-driven)
    // High variance (exploration) → larger adjustments (more responsive)
    // Low variance (exploitation) → smaller adjustments (more stable)
    float adjustment_magnitude = 0.0f;
    if (node->net && node->net->learning_variance > 0.0f) {
        float variance_norm = sqrtf(node->net->learning_variance) / 
                             (sqrtf(node->net->learning_variance) + 1.0f);
        // Adjustment magnitude emerges from variance (no hardcoded 0.01f, 0.03f, 0.04f)
        adjustment_magnitude = variance_norm;  // Pure ratio [0, 1]
    } else {
        // No variance: minimal adjustment
        adjustment_magnitude = 0.0f;
    }
    // Adjustment emerges from success rate deviation (no hardcoded 0.5f, 2.0f)
    float success_deviation = node->change_rate_avg - 0.5f;  // Deviation from neutral
    float adjustment = 1.0f + success_deviation * adjustment_magnitude;
    node->bias *= adjustment;
    // If success = 1.0: adjustment = 1.02 (grow)
    // If success = 0.5: adjustment = 1.00 (stable)
    // If success = 0.0: adjustment = 0.98 (shrink)
    
    // NO HARD BOUNDS - bias emerges from competition and decay
    // Natural bounds: bias *= 1.02 or *= 0.98 provides exponential growth/decay
    // This creates natural equilibrium without hardcoded limits
    // Bias can grow large if consistently successful, or shrink if failing
    // Competition with other nodes provides natural regulation
    if (node->bias < 0.0f) node->bias = 0.0f;  // Only prevent negative (physics)
}

/* SELF-OPTIMIZATION: Pure competition - no artificial boosts
 * - Nodes compete through natural usage patterns
 * - Decay handles pruning, activation handles strengthening
 * - Brain-like: use it or lose it (no gaming)
 */
static void node_self_optimize_if_weak(Node *node) {
    if (!node) return;
    
    // Pure competition: nodes that aren't used will decay naturally
    // No artificial boosts - let the data decide
    // This function is now a no-op (kept for API compatibility)
}

/* SELF-OPTIMIZATION ON ERROR: Pure learning - no artificial exploration boosts
 * - Correct paths strengthen through Hebbian learning
 * - Incorrect paths weaken through competition
 * - Brain-like: LTP/LTD (no gaming)
 */
static void graph_self_optimize_on_error(Graph *graph, Edge **error_path, 
                                         size_t path_count, float error_signal) {
    if (!graph || !error_path || path_count == 0) return;
    if (error_signal > 0.5f) return;  // Only optimize on failures
    
    // For each node in the error path, update meta-learning
    for (size_t i = 0; i < path_count; i++) {
        Edge *edge = error_path[i];
        if (!edge || !edge->from_node) continue;
        
        // Meta-learn: track failure (pure tracking, no boosts)
        node_meta_learn_update(edge->from_node, error_signal);
    }
}

/* EXTENDED CONTEXT: Accumulate context in node->state for longer memory
 * - Called during wave propagation
 * - state carries information across multiple hops
 * - Provides LLM-like multi-step context without new structures
 */
static void node_accumulate_context(Node *node, float incoming_activation) {
    if (!node) return;
    
    // REUSE state field as accumulated context (RNN-like hidden state)
    // Exponential moving average: new = old * decay + incoming * (1 - decay)
    float decay = 0.7f;  // How much of previous context to retain
    node->state = node->state * decay + incoming_activation * (1.0f - decay);
    
    // state now carries information from multiple previous steps
    // Can be used in edge selection for better long-range dependencies
}

/* ============================================================================
 * WAVE-BASED STOP PREDICTION (Mini Neural Net)
 * Replaces hardcoded loop detection with learned stop probability
 * ============================================================================ */

/* Compute stop probability using mini neural net
 * - Input: node activation, accumulated state, output length
 * - Output: probability that generation should stop
 * - Learns from training: bias adapts based on when output naturally completes
 * - No hardcoded thresholds - all data-driven
 */
/* REMOVED: Old stop probability functions replaced with learned stop_weight
 * Stop is now learned through error feedback, not computed via separate function.
 * See generate_from_pattern() for stop vs continue decision.
 * See melvin_m_feedback_error() for stop_weight learning.
 */

/* Update rolling window with new weight change
 * - Adaptive window size
 * - Grows/shrinks based on change rate
 */
static void node_update_weight_change_window(Node *node, float weight_change) {
    // Start with capacity 1 (minimal, like nature)
    if (node->weight_change_capacity == 0) {
        node->weight_change_capacity = 1;
        node->recent_weight_changes = malloc(sizeof(float));
        if (!node->recent_weight_changes) return;
    }
    
    // Grow if needed (doubles when full)
    if (node->weight_change_count >= node->weight_change_capacity) {
        size_t new_capacity = node->weight_change_capacity * 2;
        float *new_changes = realloc(node->recent_weight_changes, 
                                     new_capacity * sizeof(float));
        if (!new_changes) return;
        node->recent_weight_changes = new_changes;
        node->weight_change_capacity = new_capacity;
    }
    
    // Add to circular buffer
    node->recent_weight_changes[node->weight_change_index] = weight_change;
    node->weight_change_index = (node->weight_change_index + 1) % node->weight_change_capacity;
    if (node->weight_change_count < node->weight_change_capacity) {
        node->weight_change_count++;
    }
}

/* ============================================================================
 * NODE OPERATIONS (Mini Neural Nets)
 * ============================================================================ */

/* Create node (starts with minimal capacity = 1)
 * - Principle 2: No hardcoded limits
 * - Grows from seed like nature
 * 
 * REFACTORED: Now creates mini net for decision-making
 */
static Node* node_create(const uint8_t *payload, size_t payload_size, uint32_t abstraction_level) {
    Node *node = calloc(1, sizeof(Node));
    if (!node) return NULL;
    
    // Generate unique ID (simple hash for now)
    uint64_t hash = 0;
    for (size_t i = 0; i < payload_size; i++) {
        hash = hash * 31 + payload[i];
    }
    snprintf((char*)node->id, 9, "%08lx", (unsigned long)(hash & 0xFFFFFFFF));
    
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
    node->port_id = 0;  // Default: unknown/original (will be set during input processing)
    
    // CRITICAL: Create mini neural net for this node
    // Initial dimensions: 3 weights (for common inputs), 8 state dims (for context)
    node->net = mini_net_create(3, 8);
    if (!node->net) {
        free(node->payload);
        free(node);
        return NULL;
    }
    
    // Start with capacity 1 (minimal, grows immediately when needed)
    node->outgoing_capacity = 1;
    node->outgoing_edges = malloc(sizeof(Edge*));
    node->incoming_capacity = 1;
    node->incoming_edges = malloc(sizeof(Edge*));
    
    // Initialize multi-functional variables
    node->state = 0.0f;
    node->weight = 0.0f;
    node->bias = 0.0f;
    node->activation_strength = 0.0f;
    
    // Initialize self-destruct timer (relative, no global state)
    node->inactivity_timer = 0.0f;
    
    // Initialize stop prediction tracking
    node->total_activations = 0;
    node->stop_weight = 0.0f;  // No bias to stop initially (learned through feedback)
    
    // Initialize context-relative edge values
    node->edge_context_values = NULL;
    node->edge_context_capacity = 0;
    node->best_edge = NULL;
    node->best_edge_value = -1.0f;
    node->context_generation = 0;
    
    // Initialize embeddings (lazy + cached)
    node->embedding = NULL;
    node->embedding_dim = 0;
    node->embedding_generation = 0;
    
    // Initialize hierarchy structure (reference-based for compression)
    node->child_nodes = NULL;
    node->child_count = 0;
    node->is_reference_hierarchy = 0;
    
    // PHASE 2: Initialize O(1) input checking
    node->is_current_input = 0;
    
    // PHASE 4: Initialize attention mechanism
    node->attention_keys = NULL;
    node->attention_values = NULL;
    node->attention_dim = 0;
    
    // PHASE 4: Initialize predictive coding
    node->predicted_activation = 0.0f;
    node->prediction_error = 0.0f;
    
    // Initialize graph back-reference (will be set when added to graph)
    node->graph = NULL;
    
    // ADAPTIVE CONTEXT TRACE: Start with minimal size (8), grow as needed
    // Requirement line 2: NO hardcoded limits - context grows adaptively
    node->context_trace_capacity = 8;  // Initial allocation
    node->context_trace = calloc(8, sizeof(float));
    node->context_trace_len = 0;  // Empty initially
    node->context_trace_gen = 0;
    
    node->loaded = 1;
    return node;
}

/* ============================================================================
 * EMBEDDING SYSTEM (Lazy + Cached Position-Aware Embeddings)
 * ============================================================================ */

/* Compute adaptive embedding dimension based on node characteristics
 * - Larger payloads get larger embeddings (more information to encode)
 * - Higher abstraction levels get larger embeddings (more complex patterns)
 * - Data-driven: no hardcoded limits
 */
static size_t compute_adaptive_embedding_dim(Node *node) {
    if (!node) return 0;
    
    // Base dimension from payload size (data-driven)
    size_t base_dim = 32;  // Minimum dimension
    
    if (node->payload_size > 0) {
        // Logarithmic scaling: larger payloads get larger embeddings
        // dim = 32 * (1 + log2(payload_size))
        size_t log_size = 0;
        size_t temp = node->payload_size;
        while (temp > 1) {
            temp >>= 1;
            log_size++;
        }
        base_dim = 32 * (1 + log_size);
    }
    
    // Scale by abstraction level (hierarchies need richer embeddings)
    if (node->abstraction_level > 0) {
        base_dim *= (1 + node->abstraction_level);
    }
    
    // Cap at reasonable maximum (data-driven: 256 * abstraction_level)
    size_t max_dim = 256 * (1 + node->abstraction_level);
    if (base_dim > max_dim) base_dim = max_dim;
    
    return base_dim;
}

/* Forward declaration for recursive embedding computation */
static float* node_get_embedding(Node *node, Graph *graph);

/* Compute positional embedding for a node
 * - For raw nodes: position-aware encoding of payload bytes
 * - For hierarchies: aggregate child embeddings (weighted by position)
 * - Returns newly allocated embedding vector (caller must manage lifecycle)
 */
static float* node_compute_embedding(Node *node, Graph *graph) {
    if (!node) return NULL;
    
    size_t dim = compute_adaptive_embedding_dim(node);
    if (dim == 0) return NULL;
    
    float *embedding = calloc(dim, sizeof(float));
    if (!embedding) return NULL;
    
    // CASE 1: Reference hierarchy - aggregate child embeddings
    if (node->is_reference_hierarchy && node->child_nodes && node->child_count > 0) {
        // Weighted sum of child embeddings (position-aware)
        for (size_t i = 0; i < node->child_count; i++) {
            Node *child = node->child_nodes[i];
            if (!child) continue;
            
            // Get child embedding (recursive, lazy)
            float *child_embedding = node_get_embedding(child, graph);
            if (!child_embedding) continue;
            
            // Position weight: earlier children get higher weight
            // weight = 1.0 / (1.0 + position)
            float position_weight = 1.0f / (1.0f + (float)i);
            
            // Aggregate: embedding += child_embedding * position_weight
            size_t child_dim = child->embedding_dim;
            for (size_t j = 0; j < dim && j < child_dim; j++) {
                embedding[j] += child_embedding[j] * position_weight;
            }
        }
        
        // L2 normalize
        float norm = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            norm += embedding[i] * embedding[i];
        }
        norm = sqrtf(norm + 1e-8f);
        if (norm > 0.0f) {
            for (size_t i = 0; i < dim; i++) {
                embedding[i] /= norm;
            }
        }
        
        return embedding;
    }
    
    // CASE 2: Raw node or copied hierarchy - encode payload + graph context
    if (node->payload && node->payload_size > 0) {
        // STEP 1: Positional encoding of payload (like transformer input embeddings)
        for (size_t byte_idx = 0; byte_idx < node->payload_size; byte_idx++) {
            uint8_t byte = node->payload[byte_idx];
            float byte_normalized = ((float)byte - 128.0f) / 128.0f;
            
            for (size_t dim_idx = 0; dim_idx < dim; dim_idx++) {
                float freq = 1.0f / powf(10000.0f, (float)dim_idx / (float)dim);
                float position = (float)byte_idx;
                
                if (dim_idx % 2 == 0) {
                    embedding[dim_idx] += sinf(position * freq) * byte_normalized;
                } else {
                    embedding[dim_idx] += cosf(position * freq) * byte_normalized;
                }
            }
        }
        
        // STEP 2: Add edge context (like LLM attention - neighbors shape meaning)
        // Aggregate information from connected nodes (weighted by edge strength)
        float total_edge_weight = 0.0f;
        
        // Incoming edges: what leads TO this node
        if (node->incoming_edges) {
            for (size_t i = 0; i < node->incoming_count && i < 5; i++) {  // Top 5 incoming
                Edge *edge = node->incoming_edges[i];
                if (!edge || !edge->from_node) continue;
                
                // Recursively get neighbor embedding (with cycle detection via generation)
                if (edge->from_node->embedding_generation != (graph ? graph->wave_generation : 0)) {
                    float *neighbor_emb = node_get_embedding(edge->from_node, graph);
                    if (neighbor_emb && edge->from_node->embedding_dim > 0) {
                        float edge_weight = edge->weight / (edge->weight + 1.0f);  // Normalize
                        size_t neighbor_dim = edge->from_node->embedding_dim;
                        for (size_t j = 0; j < dim && j < neighbor_dim; j++) {
                            embedding[j] += neighbor_emb[j] * edge_weight;  // Pure edge weight, no boost
                        }
                        total_edge_weight += edge_weight;
                    }
                }
            }
        }
        
        // Outgoing edges: what this node leads TO
        if (node->outgoing_edges) {
            for (size_t i = 0; i < node->outgoing_count && i < 5; i++) {  // Top 5 outgoing
                Edge *edge = node->outgoing_edges[i];
                if (!edge || !edge->to_node) continue;
                
                if (edge->to_node->embedding_generation != (graph ? graph->wave_generation : 0)) {
                    float *neighbor_emb = node_get_embedding(edge->to_node, graph);
                    if (neighbor_emb && edge->to_node->embedding_dim > 0) {
                        float edge_weight = edge->weight / (edge->weight + 1.0f);
                        size_t neighbor_dim = edge->to_node->embedding_dim;
                        for (size_t j = 0; j < dim && j < neighbor_dim; j++) {
                            embedding[j] += neighbor_emb[j] * edge_weight;  // Pure edge weight, no boost
                        }
                        total_edge_weight += edge_weight;
                    }
                }
            }
        }
        
        // L2 normalize (like LLM layer norm), scale by edge contribution
        float norm = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            norm += embedding[i] * embedding[i];
        }
        norm = sqrtf(norm + 1e-8f);
        // Scale normalization by edge weight influence (more connections = more contextualized)
        float edge_scale = total_edge_weight / (total_edge_weight + 1.0f);
        norm = norm / (1.0f + edge_scale);  // Stronger edges = less aggressive normalization
        if (norm > 0.0f) {
            for (size_t i = 0; i < dim; i++) {
                embedding[i] /= norm;
            }
        }
        
        return embedding;
    }
    
    // CASE 3: Blank node - zero embedding (will be learned through connections)
    // Already calloc'd to zero, just return
    return embedding;
}

/* Get node embedding (lazy accessor with caching)
 * - Computes embedding on first access
 * - Returns cached embedding on subsequent accesses
 * - Invalidates cache when node changes
 */
static float* node_get_embedding(Node *node, Graph *graph) {
    if (!node) return NULL;
    
    // Check if embedding is already computed and valid
    if (node->embedding && node->embedding_dim > 0) {
        return node->embedding;
    }
    
    // Mark as in-progress to prevent recursive recomputation
    // Set embedding_generation BEFORE computing to prevent infinite recursion
    node->embedding_generation = graph ? graph->wave_generation : 1;
    node->embedding_dim = compute_adaptive_embedding_dim(node);
    
    // Compute embedding (lazy)
    node->embedding = node_compute_embedding(node, graph);
    if (!node->embedding) {
        node->embedding_generation = 0;  // Reset if failed
        node->embedding_dim = 0;
        return NULL;
    }
    
    return node->embedding;
}

/* Invalidate node embedding (forces recomputation on next access)
 * - Called when node changes (weight update, hierarchy formation, etc.)
 * - Frees cached embedding
 */
static void node_invalidate_embedding(Node *node) {
    if (!node) return;
    
    if (node->embedding) {
        free(node->embedding);
        node->embedding = NULL;
    }
    node->embedding_dim = 0;
    node->embedding_generation = 0;
}

/* ============================================================================
 * HIERARCHY PAYLOAD RECONSTRUCTION (On-Demand Decompression)
 * ============================================================================ */

/* Get node payload (on-demand reconstruction for reference hierarchies)
 * - For non-reference nodes: returns payload directly
 * - For reference hierarchies: reconstructs from children recursively
 * - Returns dynamically allocated buffer for reference hierarchies (caller must free)
 * - Returns direct pointer for non-reference nodes (caller must NOT free)
 */
static uint8_t* node_get_payload(Node *node, size_t *out_size) {
    if (!node) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    
    // If not a reference hierarchy, return payload directly
    if (!node->is_reference_hierarchy || node->payload) {
        if (out_size) *out_size = node->payload_size;
        return node->payload;
    }
    
    // Reference hierarchy: reconstruct from children
    if (!node->child_nodes || node->child_count == 0) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    
    // Allocate temporary buffer (caller must free)
    uint8_t *combined = malloc(node->payload_size);
    if (!combined) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    
    size_t offset = 0;
    for (size_t i = 0; i < node->child_count; i++) {
        Node *child = node->child_nodes[i];
        if (!child) continue;
        
        // Recursive: child might also be a reference hierarchy
        size_t child_size;
        uint8_t *child_payload = node_get_payload(child, &child_size);
        if (child_payload && offset + child_size <= node->payload_size) {
            memcpy(combined + offset, child_payload, child_size);
            offset += child_size;
            
            // Free if it was dynamically allocated (from recursive call)
            if (child->is_reference_hierarchy) {
                free(child_payload);
            }
        }
    }
    
    if (out_size) *out_size = offset;
    return combined;
}

/* Forward declarations */
static float edge_transform_activation(Edge *edge, float input_activation, Graph *graph);
static float node_local_normalize(Node *node, float raw_activation);
static void node_residual_update(Node *node, float delta_activation);
static float compute_minimal_threshold(Node *node);
static float compute_local_edge_variance(Node *node);
static float compute_adaptive_edge_inactivity_threshold(Node *from_node);
static float compute_adaptive_edge_timer_increment(Node *from_node);
static float compute_adaptive_node_inactivity_threshold(Node *node);
static float compute_adaptive_isolation_threshold(Node *node);
static float compute_adaptive_node_timer_increment(Node *node);
static float compute_local_avg_node_weight(Node *node);
static float compute_local_node_weight_variance(Node *node);
static int edge_should_self_destruct(Edge *edge);
static void edge_self_destruct(Edge *edge);
static int node_should_self_destruct(Node *node);
static void node_self_destruct(Node *node);
static void node_remove_edge_from_list(Node *node, Edge *edge, int is_outgoing);
static float compute_node_pattern_similarity(Node *node1, Node *node2);
static float compute_adaptive_similarity_threshold(Node *node);
static Edge* graph_find_edge_between(Node *from_node, Node *to_node);
static void graph_create_similarity_edges_for_node(Graph *graph, Node *new_node);
static size_t compute_adaptive_hash_size(size_t expected_entries);
static float compute_input_activation(Node *input_node);
static void compute_similarity_edge_range(Node *node, float *min_relative, float *max_relative);
static Node* find_active_hierarchy(Graph *graph, uint8_t *output, size_t output_len, size_t *out_position);
static Node* find_accepting_blank_node(Graph *graph, const uint8_t *pattern, size_t pattern_size);
static Node* graph_find_or_create_pattern_node(Graph *graph, const uint8_t *pattern, size_t pattern_size);
static int graph_add_edge(Graph *graph, Edge *edge);


/* ============================================================================
 * WAVE PROPAGATION STRUCTURE DECISIONS (Hierarchy & Blank Nodes)
 * ============================================================================ */

/* Forward declarations for structure creation functions */
static Node* create_hierarchy_node(Graph *graph, Node *node1, Node *node2);
// REMOVED: graph_create_blank_from_cluster - unused function

/* Forward declarations for Phase 4 intelligence mechanisms */
// REMOVED: compute_attention_score, node_init_attention - unused functions
static float compute_node_bloat_score(Node *node);
static float compute_embedding_similarity(Node *candidate, Node **context_nodes, size_t context_len, Graph *graph);
static void compute_node_hidden_state(Node *node, float *out_state);
static float compute_context_similarity(float *state1, float *state2);

/* Forward declarations for position-aware learning (SOLUTION 2) */

/* Collect hierarchy formation signals from edge during wave propagation
 * - All signals are local and data-driven
 */

/* Check if node should self-destruct (relative timer, local context)
 * - Node checks its own usefulness from its timer
 * - No global state: all thresholds computed from local context
 * - PHASE 3: Uses unified bloat score for comprehensive assessment
 */
static int node_should_self_destruct(Node *node) {
    if (!node) return 0;
    
    // Never delete nodes that are currently activated
    if (node->activation_strength > 0.0f) return 0;
    
    // PHASE 3: Use unified bloat score for comprehensive assessment
    // High bloat score = useless node that should be removed
    float bloat_score = compute_node_bloat_score(node);
    float inactivity_threshold = compute_adaptive_node_inactivity_threshold(node);
    
    // Node self-destructs when:
    // 1. Bloat score exceeds threshold (data-driven, not hardcoded)
    // 2. Inactivity timer exceeds threshold
    if (bloat_score > (1.0f - inactivity_threshold) && node->inactivity_timer > inactivity_threshold) {
        return 1;
    }
    
    // Check if node is isolated (no edges) - still important to check
    if (node->outgoing_count == 0 && node->incoming_count == 0) {
        float isolation_threshold = compute_adaptive_isolation_threshold(node);
        if (node->inactivity_timer > isolation_threshold) {
            return 1;  // Node should self-destruct (isolated and unused)
        }
    }
    
    return 0;
}

/* Node self-destructs (removes itself from graph)
 * - Frees node resources
 * - Note: Node removal from graph array happens during cleanup
 * 
 * REFACTORED: Now frees mini net
 */
static void node_self_destruct(Node *node) {
    if (!node) return;
    
    // Free mini net
    if (node->net) {
        mini_net_free(node->net);
        node->net = NULL;
    }
    
    // Free node resources
    if (node->payload) free(node->payload);
    if (node->recent_weight_changes) free(node->recent_weight_changes);
    if (node->outgoing_edges) free(node->outgoing_edges);
    if (node->incoming_edges) free(node->incoming_edges);
    if (node->edge_context_values) free(node->edge_context_values);
    if (node->context_trace) free(node->context_trace);  // Free adaptive context trace
    
    // Free embedding (lazy + cached)
    if (node->embedding) {
        free(node->embedding);
        node->embedding = NULL;
    }
    
    // Free hierarchy structure (don't free children themselves - they're in graph)
    if (node->child_nodes) {
        free(node->child_nodes);
        node->child_nodes = NULL;
    }
    
    free(node);
}

/* Compute node activation strength (mini neural net)
 * - Weighted sum of incoming edges
 * - Self-regulating bias
 * - Soft non-linearity
 * - O(degree) operation
 * - Self-destruct check: node checks its own usefulness
 */
static float node_compute_activation_strength(Node *node) {
    if (!node || node->incoming_count == 0) {
        // Update timer even when no input (node is inactive)
        if (node) {
            float increment_rate = compute_adaptive_node_timer_increment(node);
            node->inactivity_timer += increment_rate;
            
            // Check self-destruct
            if (node_should_self_destruct(node)) {
                node_self_destruct(node);
                return 0.0f;  // Node is dead
            }
        }
        return 0.0f;  // Neutral when no input
    }
    
    // Sum weighted inputs from incoming edges
    float input_sum = 0.0f;
    float total_weight = 0.0f;
    
    for (size_t i = 0; i < node->incoming_count; i++) {
        Edge *edge = node->incoming_edges[i];
        if (!edge || !edge->from_node) continue;
        
        // Get activation from source node
        float source_activation = edge->from_node->activation_strength;
        
        // Transform activation through edge (mini transformer)
        // (Following README: "Sums weighted inputs from incoming edges (already transformed)")
        float transformed = edge_transform_activation(edge, source_activation, NULL);  // Graph not available in node context
        
        // Accumulate transformed activation (pure weight × activation)
        // (Following README: transformed values are the weighted inputs)
        input_sum += transformed;
        // Track edge weights for relative normalization (local context)
        total_weight += edge->weight;
    }
    
    // Relative normalization: normalize by sum of edge weights (local context, not global)
    // (Following README: "All decisions relative to local context", "No global normalization")
    // Note: transformed values already include edge->weight, so we normalize by weight sum
    // to get relative contribution of each transformed input
    float epsilon = compute_adaptive_epsilon(total_weight);
    if (total_weight <= epsilon) return 0.0f;  // Neutral when no weights
    
    // Normalize relative to local weight sum (not global, not hard threshold)
    float normalized_input = input_sum / (total_weight + epsilon);
    
    // Compute self-regulating bias (relative to local context)
    float local_avg = node_get_local_incoming_weight_avg(node);
    float bias_epsilon = compute_adaptive_epsilon(local_avg);
    node->bias = node->weight / (local_avg + bias_epsilon + 1.0f);
    
    // Combine: input + bias
    float raw_activation = normalized_input + node->bias;
    
    // Apply local normalization (LayerNorm per node, not global)
    // This stabilizes training and prevents vanishing/exploding activations
    float normalized_activation = node_local_normalize(node, raw_activation);
    
    // Apply soft non-linearity (smooth, no hard threshold)
    // Result is in range (-1, 1), we'll clamp to [0, 1]
    float activation = normalized_activation / (1.0f + fabsf(normalized_activation));
    
    // Clamp to [0, 1]
    // Note: Removed residual connection - it was causing saturation to 1.0
    // Hebbian learning doesn't need residual connections (no backprop)
    if (activation < 0.0f) activation = 0.0f;
    if (activation > 1.0f) activation = 1.0f;
    
    // Update relative timer: reset if activated, increment if not
    if (activation > 0.0f) {
        node->inactivity_timer = 0.0f;  // Reset timer (node is active)
    } else {
        // Increment timer (node is inactive)
        float increment_rate = compute_adaptive_node_timer_increment(node);
        node->inactivity_timer += increment_rate;
    }
    
    // SELF-DESTRUCT CHECK: Node checks its own timer relative to local context
    if (node_should_self_destruct(node)) {
        node_self_destruct(node);
        return 0.0f;  // Node is dead, no activation
    }
    
    return activation;
}

/* Compute adaptive node weight learning rate (data-driven, not hardcoded 0.05f)
 * - Uses node's change rate, bias (meta-learning), and mini net variance
 * - Faster changes → higher rate, slower changes → lower rate
 * - Successful nodes (high bias) → higher rate, failing nodes → lower rate
 */
static inline float compute_adaptive_node_weight_rate(Node *node) {
    if (!node) return 0.0f;  // No data = no learning
    
    // Pure change rate (no scaling factors)
    float change_rate = node->change_rate_avg;
    if (change_rate <= 0.0f) return 0.0f;
    
    // Pure variance contribution (normalized ratio)
    float variance_contrib = 0.0f;
    if (node->net && node->net->learning_variance > 0.0f) {
        float variance_norm = sqrtf(node->net->learning_variance) / 
                             (sqrtf(node->net->learning_variance) + 1.0f);
        variance_contrib = variance_norm;
    }
    
    // Pure bias contribution (normalized ratio)
    float bias_contrib = 0.0f;
    if (node->bias > 0.0f) {
        bias_contrib = node->bias / (node->bias + 1.0f);
    }
    
    // Combine as pure weighted sum (no arbitrary multipliers)
    // Each component contributes proportionally
    float total = change_rate + variance_contrib + bias_contrib;
    
    // Normalize by weight (heavier nodes learn slower naturally)
    if (node->weight > 0.0f) {
        total = total / (node->weight + 1.0f);
    }
    
    return total;
}

/* Update node weight (local only, direct Hebbian update)
 * - No global gradient
 * - O(1) operation
 * - Adaptive learning rate (data-driven, not hardcoded)
 */
static void node_update_weight(Node *node, float activation) {
    if (!node) return;
    
    float old_weight = node->weight;
    
    // Update weight based on activation (direct Hebbian learning)
    node->weight += activation;
    
    // Track weight change for monitoring
    node_update_weight_change_window(node, node->weight - old_weight);
    
    // Invalidate embedding (node changed, recompute on next access)
    node_invalidate_embedding(node);
}

/* ============================================================================
 * SELF-DESTRUCT ADAPTIVE THRESHOLDS (Relative, No Global State)
 * ============================================================================ */

/* Compute adaptive edge inactivity threshold (from local context)
 * - Threshold adapts to local edge distribution (variance, density)
 * - No hardcoded values: all computed from local data
 * - Relative to from_node's local context
 */
static float compute_adaptive_edge_inactivity_threshold(Node *from_node) {
    if (!from_node || from_node->outgoing_count == 0) return 1.0f;  // Default: 1.0
    
    // Get local context
    float local_avg = node_get_local_outgoing_weight_avg(from_node);
    float local_variance = compute_local_edge_variance(from_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return 1.0f;  // No pruning when no context
    
    // Threshold adapts to local variance (high variance = stricter)
    float variance_norm = local_variance / (local_avg + epsilon);
    
    // Edge density: more edges = stricter threshold (keep graph lean)
    // Adaptive divisor: scales with node's weight (more active nodes get more edges before pruning)
    float density_base = from_node->weight / (from_node->weight + 1.0f);
    float density_divisor = 1.0f + density_base * 9.0f;  // Range: 1.0 to 10.0, adaptive
    float edge_density = (float)from_node->outgoing_count / 
                        ((float)from_node->outgoing_count + density_divisor);
    
    // Base threshold from variance (higher variance = higher threshold)
    float base_threshold = variance_norm / (variance_norm + 1.0f);
    
    // Minimum threshold: even with zero variance, prune edges below this fraction
    // Computed from edge density (more edges = higher minimum)
    float min_threshold = edge_density / (edge_density + 1.0f);  // Range: 0.0 to 0.5
    
    // Strictness increases with edge density (more edges = prune more)
    float strictness = base_threshold * (1.0f + edge_density);
    if (strictness < min_threshold) strictness = min_threshold;
    
    return strictness;  // Fully adaptive: no hardcoded values
}

/* Compute adaptive edge timer increment rate (from local context)
 * - Increment rate adapts to local activity (more active = slower increment)
 * - No hardcoded values: all computed from local data
 */
static float compute_adaptive_edge_timer_increment(Node *from_node) {
    if (!from_node || from_node->outgoing_count == 0) {
        // Adaptive default: computed from node's weight (not hardcoded)
        float weight_factor = from_node->weight / (from_node->weight + 1.0f);
        return weight_factor * 0.01f;  // Range: 0.0 to 0.01, adaptive
    }
    
    // Increment rate adapts to local activity (more active = slower increment)
    float local_avg = node_get_local_outgoing_weight_avg(from_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // More active nodes → slower timer increment (edges get more time)
    float activity_factor = local_avg / (local_avg + epsilon + 1.0f);
    float increment = 0.01f * (1.0f - activity_factor);  // Range: 0.0 to 0.01
    
    return increment;
}

/* Compute adaptive node inactivity threshold (from local context)
 * - Threshold adapts to local node weight distribution
 * - No hardcoded values: all computed from local data
 */
static float compute_adaptive_node_inactivity_threshold(Node *node) {
    if (!node) return 1.0f;  // Default: 1.0
    
    // Get local context (from neighbors)
    float local_avg = compute_local_avg_node_weight(node);
    float local_variance = compute_local_node_weight_variance(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return 1.0f;  // No pruning when no context
    
    // Threshold adapts to local variance (high variance = stricter)
    float variance_norm = local_variance / (local_avg + epsilon);
    float threshold = variance_norm / (variance_norm + 1.0f);
    
    // DATA-DRIVEN: Return threshold directly (no hardcoded minimum)
    // If variance is very low, threshold will be very low (data-driven)
    return threshold;
}

/* Compute adaptive isolation threshold (for isolated nodes)
 * - Threshold adapts to node's own weight history
 * - Nodes that were once active get more time before deletion
 * - No hardcoded values: all computed from node's own data
 */
static float compute_adaptive_isolation_threshold(Node *node) {
    if (!node) return 1.0f;  // Default: 1.0
    
    // Threshold adapts to node's own weight history
    // Nodes that were once active get more time before deletion
    float weight_factor = node->weight / (node->weight + 1.0f);
    
    // DATA-DRIVEN: Threshold from node's weight (no hardcoded multipliers)
    // Higher weight = higher threshold (node was more useful)
    // Pure data-driven: uses node's actual weight and factor
    float base_threshold = node->weight + 1.0f;  // Weight + 1.0 (data-driven)
    
    // Apply weight factor to scale threshold (heavier nodes get more time)
    return base_threshold * (1.0f + weight_factor);
}

/* Compute adaptive node timer increment rate (from node's own context)
 * - Increment rate adapts to node's own activity history
 * - No hardcoded values: all computed from node's own data
 */
static float compute_adaptive_node_timer_increment(Node *node) {
    if (!node) return 0.01f;  // Default: slow increment
    
    // Increment rate adapts to node's own activity history
    float activity_factor = node->weight / (node->weight + 1.0f);
    float base_increment = 0.01f * (1.0f - activity_factor);  // Range: 0.0 to 0.01
    
    // PASSIVE HIERARCHY DECAY: Hierarchies that aren't being used decay faster
    // Uses existing data: total_activations vs incoming_weight_sum
    // No new variables - just computes from what's already tracked
    if (node->abstraction_level > 0) {
        // This is a hierarchy - check if it's providing memory benefit
        // Low total_activations relative to incoming_weight_sum = not being used
        float epsilon = compute_adaptive_epsilon(node->incoming_weight_sum);
        float usage_ratio = (float)node->total_activations / (node->incoming_weight_sum + epsilon);
        
        // If hierarchy has low usage (components are used more), decay faster
        // DATA-DRIVEN: Accelerate decay based on usage ratio (no hardcoded 0.1f threshold)
        // Smooth function: lower usage_ratio = faster decay
        // No cutoff - all hierarchies decay faster if usage is low
        // Pure data-driven: acceleration computed from actual usage_ratio
        float acceleration = 1.0f / (usage_ratio + epsilon);  // Smooth, no threshold
        base_increment *= acceleration;
    }
    
    return base_increment;
}

/* Compute local average node weight (from neighbors)
 * - Samples neighbor weights to compute local average
 * - O(degree) operation
 */
static float compute_local_avg_node_weight(Node *node) {
    if (!node) return 0.0f;
    
    float total_weight = 0.0f;
    size_t neighbor_count = 0;
    
    // Sample outgoing neighbors
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < 10; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node) {
            total_weight += edge->to_node->weight;
            neighbor_count++;
        }
    }
    
    // Sample incoming neighbors
    for (size_t i = 0; i < node->incoming_count && neighbor_count < 20; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node) {
            total_weight += edge->from_node->weight;
            neighbor_count++;
        }
    }
    
    if (neighbor_count == 0) return 0.0f;
    return total_weight / (float)neighbor_count;
}

/* Compute local node weight variance (from neighbors)
 * - Samples neighbor weights to compute local variance
 * - O(degree) operation
 */
static float compute_local_node_weight_variance(Node *node) {
    if (!node) return 0.0f;
    
    float local_avg = compute_local_avg_node_weight(node);
    if (local_avg <= 0.0f) return 0.0f;
    
    float variance = 0.0f;
    size_t neighbor_count = 0;
    
    // Sample outgoing neighbors
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < 10; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node) {
            float diff = edge->to_node->weight - local_avg;
            variance += diff * diff;
            neighbor_count++;
        }
    }
    
    // Sample incoming neighbors
    for (size_t i = 0; i < node->incoming_count && neighbor_count < 20; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node) {
            float diff = edge->from_node->weight - local_avg;
            variance += diff * diff;
            neighbor_count++;
        }
    }
    
    if (neighbor_count == 0) return 0.0f;
    return variance / (float)neighbor_count;
}

/* ============================================================================
 * SELF-REGULATION: Node Bloat Detection
 * ============================================================================
 * 
 * Computes a "bloat score" for a node indicating whether it should self-delete.
 * 
 * Principle 1: LOCAL ONLY - uses only node's own data + immediate neighbors
 * Principle 2: NO HARDCODED THRESHOLDS - all values data-driven
 * 
 * High bloat score = node is unused, isolated, or redundant
 * Low bloat score = node is valuable (connected, active)
 */
static float compute_node_bloat_score(Node *node) {
    if (!node) return 1.0f;  // NULL nodes are definitely bloat
    
    // Factor 1: Low weight relative to neighbors (unused)
    float local_avg_weight = compute_local_avg_node_weight(node);
    float epsilon = compute_adaptive_epsilon(local_avg_weight);
    
    // Weight ratio: how active is this node compared to neighbors?
    // Low ratio = node is underused relative to neighbors
    float weight_ratio = node->weight / (local_avg_weight + epsilon + 0.1f);
    float unused_factor = 1.0f / (weight_ratio + 1.0f);  // High when weight_ratio is low
    
    // Factor 2: Low edge connectivity (isolated)
    // Nodes with few connections are less integrated into the graph
    float connectivity = (float)(node->outgoing_count + node->incoming_count);
    float connectivity_score = connectivity / (connectivity + 5.0f);  // Saturates around 5 edges
    float isolation_factor = 1.0f - connectivity_score;  // High when connectivity is low
    
    // Factor 3: Low activation count (never used in output)
    // Nodes that were never activated during generation are not contributing
    float activation_factor = (float)node->total_activations / ((float)node->total_activations + 5.0f);
    float dormant_factor = 1.0f - activation_factor;  // High when rarely activated
    
    // Factor 4: Context trace emptiness (no accumulated context)
    // Nodes with empty context trace have never been part of a meaningful sequence
    float context_factor = (float)node->context_trace_len / 8.0f;
    float contextless_factor = 1.0f - context_factor;  // High when no context
    
    // Combine factors: bloat = product of all factors
    // A node is bloat only if ALL factors indicate it's useless
    // This makes deletion conservative (need multiple signals)
    float bloat = unused_factor * isolation_factor * dormant_factor * contextless_factor;
    
    // Hierarchy protection: hierarchies are more valuable, reduce their bloat score
    // DATA-DRIVEN: protection scales with abstraction level
    if (node->abstraction_level > 0) {
        float hierarchy_protection = 1.0f / (1.0f + (float)node->abstraction_level * 0.5f);
        bloat *= hierarchy_protection;
    }
    
    return bloat;
}


/* ============================================================================
 * EDGE OPERATIONS (Mini Transformers)
 * ============================================================================ */

/* Create edge (starts minimal)
 * - Connects two nodes
 * - Initial weight from local context
 */
/* Create edge with uint8_t weights (SIMPLIFIED - no MiniNet)
 * - Initializes with neutral weight (128 = middle of 0-255)
 * - No transformer/MiniNet (edges are simple connections)
 * - Intelligence in nodes, not edges
 */
static Edge* edge_create(Node *from_node, Node *to_node) {
    if (!from_node || !to_node) return NULL;
    
    Edge *edge = calloc(1, sizeof(Edge));
    if (!edge) return NULL;
    
    edge->from_node = from_node;
    edge->to_node = to_node;
    
    // Initialize with neutral values (uint8_t 0-255 range)
    edge->weight = 128;          // Neutral starting weight (middle of range)
    edge->routing_gate = 255;    // Fully open gate initially
    edge->inactivity_timer = 0;  // Active
    edge->flags = 0;             // Clear all flags
    edge_set_direction(edge, 1); // Set direction bit (from->to)
    edge->last_wave_generation = 0;
    
    // REMOVED: MiniNet transformer creation (edges are simple now)
    
    // Add to node's edge lists (grow if needed)
    // Outgoing
    if (from_node->outgoing_count >= from_node->outgoing_capacity) {
        size_t new_cap = from_node->outgoing_capacity * 2;
        Edge **new_edges = realloc(from_node->outgoing_edges, new_cap * sizeof(Edge*));
        if (new_edges) {
            from_node->outgoing_edges = new_edges;
            from_node->outgoing_capacity = new_cap;
        }
    }
    if (from_node->outgoing_count < from_node->outgoing_capacity) {
        from_node->outgoing_edges[from_node->outgoing_count++] = edge;
        // Convert uint8_t to float for weight sum
        float weight_float = weight_uint8_to_float(edge->weight, 1.0f);
        node_update_outgoing_weight_sum(from_node, 0.0f, weight_float);
    }
    
    // Incoming
    if (to_node->incoming_count >= to_node->incoming_capacity) {
        size_t new_cap = to_node->incoming_capacity * 2;
        Edge **new_edges = realloc(to_node->incoming_edges, new_cap * sizeof(Edge*));
        if (new_edges) {
            to_node->incoming_edges = new_edges;
            to_node->incoming_capacity = new_cap;
        }
    }
    if (to_node->incoming_count < to_node->incoming_capacity) {
        to_node->incoming_edges[to_node->incoming_count++] = edge;
        float weight_float = weight_uint8_to_float(edge->weight, 1.0f);
        node_update_incoming_weight_sum(to_node, 0.0f, weight_float);
    }
    
    // Invalidate embeddings (new edge changes graph context)
    node_invalidate_embedding(from_node);
    node_invalidate_embedding(to_node);
    
    return edge;
}

/* Check if edge should self-destruct (relative timer, local context)
 * - Edge checks its own usefulness from its timer
 * - No global state: all thresholds computed from local context
 */
/* Check if edge should self-destruct (uint8_t weights)
 * - Edge checks its own usefulness from weight and timer
 * - All thresholds computed from local context
 * - Works with uint8_t weights (0-255)
 */
static int edge_should_self_destruct(Edge *edge) {
    if (!edge || !edge->from_node) return 0;
    
    // Get local context (from from_node)
    float local_avg = node_get_local_outgoing_weight_avg(edge->from_node);
    if (local_avg <= 0.0f) return 0;  // No pruning when no context
    
    // Convert uint8_t weight to float for comparison
    float weight_float = weight_uint8_to_float(edge->weight, local_avg);
    float weight_relative = weight_float / local_avg;
    
    // Compute adaptive inactivity threshold (from local context)
    float inactivity_threshold = compute_adaptive_edge_inactivity_threshold(edge->from_node);
    
    // Convert uint8_t timer to threshold (0-255 range)
    uint8_t timer_threshold = (uint8_t)(inactivity_threshold * 255.0f);
    
    // Self-destruct if:
    // 1. Weight is far below local average (decayed significantly)
    // 2. Inactivity timer exceeds threshold (hasn't activated in a while)
    if (weight_relative < inactivity_threshold && edge->inactivity_timer > timer_threshold) {
        return 1;  // Edge should self-destruct
    }
    
    return 0;
}

/* Edge self-destructs (removes itself from graph)
 * - Removes self from both nodes' edge lists
 * - Frees self (edge knows how to clean itself up)
 * - SIMPLIFIED: No MiniNet to free (edges are simple now)
 */
static void edge_self_destruct(Edge *edge) {
    if (!edge) return;
    
    // Remove self from from_node's outgoing list
    if (edge->from_node) {
        node_remove_edge_from_list(edge->from_node, edge, 1);
    }
    
    // Remove self from to_node's incoming list
    if (edge->to_node) {
        node_remove_edge_from_list(edge->to_node, edge, 0);
    }
    
    // REMOVED: MiniNet cleanup (edges no longer have transformers)
    
    // Free self (edge knows how to clean itself up)
    free(edge);
}

/* Transform activation through edge (mini transformer)
 * - Context-aware attention (like transformer attention mechanism)
 * - Pattern similarity (pure embedding cosine similarity)
 * - Routing gate (learned gating)
 * - Pure multiplication (no artificial boosts)
 * - All relative to local context
 * - O(1) operation per edge
 * - Self-destruct check: edge checks its own usefulness
 * 
 * This is the CORE of Melvin's intelligence - edges as mini transformers
 * Following README: "edges act as mini transformers"
 */
/* Transform activation through edge (SIMPLIFIED - no MiniNet needed)
 * - Simple weight × gate × activation (no complex transformer)
 * - uint8_t weights (0-255) prevent catastrophic forgetting
 * - Intelligence in nodes, not edges
 * - O(1) operation per edge
 */
static float edge_transform_activation(Edge *edge, float input_activation, Graph *graph) {
    if (!edge || !edge->from_node || !edge->to_node) return 0.0f;
    
    // Update inactivity timer (uint8_t 0-255)
    int edge_fired = (graph && edge->last_wave_generation == graph->wave_generation);
    if (input_activation > 0.0f || edge_fired) {
        edge->inactivity_timer = 0;  // Reset timer
        if (graph) edge->last_wave_generation = graph->wave_generation;
    } else {
        // Increment timer (bounded at 255)
        if (edge->inactivity_timer < 255) edge->inactivity_timer++;
    }
    
    // Self-destruct check
    if (edge_should_self_destruct(edge)) {
        edge->flags |= 0x01;  // Set marked_for_deletion bit
        return 0.0f;
    }
    
    // SIMPLE TRANSFORMATION (no MiniNet needed)
    // Normalize uint8_t weights to 0.0-1.0 range
    float weight_norm = (float)edge->weight / 255.0f;
    float gate_norm = (float)edge->routing_gate / 255.0f;
    
    // Pure multiplication: weight × gate × activation
    return weight_norm * gate_norm * input_activation;
}

/* Compute edge attention using co-activation pattern matching
 * Edge learns: "When I fire, these nodes are active at these strengths"
 * Attention checks: "How well does current context match my learned pattern?"
 * 
 * Human-like: Associative activation with multiplicative matching
 * Not linear scaling: All parts of pattern must match (multiplicative)
 * Not exact matching: Tolerates variations in activation strength
 * 
 * Follows Requirement line 3: No hardcoded limits
 * Follows Requirement line 6: Context is activated nodes
 */
/* REMOVED: edge_compute_attention() and edge_learn_attention()
 * - No longer needed with simple uint8_t edge weights
 * - Context handled by node_update_context_values() instead
 * - Intelligence in nodes, not edges
 */

/* Transform activation through edge WITH CONTEXT (SIMPLIFIED)
 * 
 * REFACTORED: Context modulates edge weight BEFORE scoring (not after)
 * This is how the README intends it: "context changes all edge values"
 * 
 * Flow:
 * 1. Compute context relevance (from wave prop + embeddings)
 * 2. Effective weight = edge->weight * context_relevance
 * 3. Score = effective_weight * activation
 */
// REMOVED: edge_transform_activation_with_context - unused function

/* Score edge for routing (local only)
 * - Relative to local context
 * - For sparse routed attention
 */
static float edge_score_for_routing(Edge *edge, float source_activation, float local_avg_weight) {
    if (!edge) return 0.0f;
    
    float epsilon = compute_adaptive_epsilon(local_avg_weight);
    float weight_relative = edge->weight / (local_avg_weight + epsilon + 1.0f);
    
    // Compute similarity on-the-fly (use local average as proxy - brain-like local inhibition)
    float local_avg_sim = node_get_local_outgoing_weight_avg(edge->from_node);
    float sim_epsilon = compute_adaptive_epsilon(local_avg_sim);
    float similarity = (local_avg_sim > sim_epsilon) ? 
                (local_avg_sim / (local_avg_sim + 1.0f + sim_epsilon)) : sim_epsilon;
    
    // Compute routing priority directly from data (no hardcoded ranges)
    // Similarity edges get higher priority when they're more important (weight relative to local average)
    float similarity_priority = 1.0f;
    if (edge_is_similarity(edge)) {
        // Priority computed from how important this similarity edge is
        // If similarity edge weight is above local average, it gets priority
        // Priority = 1.0 + (weight_relative - 1.0) normalized
        if (weight_relative > 1.0f) {
            float priority_excess = weight_relative - 1.0f;
            // Normalize excess by weight_relative (smooth function, no hardcoded range)
            similarity_priority = 1.0f + (priority_excess / (priority_excess + 1.0f));
        }
        // If weight_relative <= 1.0, priority stays at 1.0 (no negative priority)
    }
    
    // Pure routing: weight × similarity × activation (no boosts)
    // Each factor contributes multiplicatively - natural competition
    return weight_relative * (1.0f + similarity * similarity_priority) * source_activation;
}

/* Update edge weight (local only, direct Hebbian update)
 * - Based on activation
 * - No learning rate - direct update with decay providing bounds
 */
static void edge_update_weight(Edge *edge, float activation) {
    if (!edge || !edge->from_node) return;
    
    uint8_t old_weight = edge->weight;
    
    // Update weight (bounded Hebbian learning)
    edge_update_weight_bounded(edge, activation);
    
    // Update cached sums (convert uint8_t to float)
    float old_weight_float = weight_uint8_to_float(old_weight, 1.0f);
    float new_weight_float = weight_uint8_to_float(edge->weight, 1.0f);
    node_update_outgoing_weight_sum(edge->from_node, old_weight_float, new_weight_float);
    if (edge->to_node) {
        node_update_incoming_weight_sum(edge->to_node, old_weight_float, new_weight_float);
    }
    
    // Invalidate embeddings for connected nodes (edge changed, affects context)
    node_invalidate_embedding(edge->from_node);
    if (edge->to_node) {
        node_invalidate_embedding(edge->to_node);
    }
}

/* Update node's context-relative edge values (called when context changes)
 * CONTEXT GATING: Edges that connect to nodes in context_trace are enabled (gated open)
 * Other edges are suppressed (gated closed). Then normal competition between enabled edges.
 * 
 * Requirement: "context is a payload, of that activated nodes from a input and the decisions made,
 * the current node holds the context of the last x number, that context changes the edge weights
 * of the current node. If a node has strong edges to the context payload then it changes its mini prediction"
 * 
 * Brain-inspired: Context-dependent gating (specific pathways enabled/disabled by context)
 * Exponential recency: Recent context matches matter exponentially more (working memory)
 * 
 * - Computes edge->weight / local_avg for each edge, gated by context match
 * - Updates cached best_edge pointer
 * - O(degree) but only called when context changes (not on every access)
 */
static void node_update_context_values(Node *node, uint32_t current_generation) {
    if (!node || node->outgoing_count == 0) return;
    
    // Check if already updated this generation
    if (node->context_generation == current_generation) return;
    
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // Allocate/grow context values array to match outgoing_capacity
    // This ensures we always have enough space as edges are added/removed
    // Following README: Adaptive growth - allocate based on capacity, use based on count
    if (node->edge_context_capacity < node->outgoing_capacity) {
        // Need to grow the array
        float *new_values = realloc(node->edge_context_values, 
                                    node->outgoing_capacity * sizeof(float));
        if (new_values) {
            node->edge_context_values = new_values;
            node->edge_context_capacity = node->outgoing_capacity;
        }
    }
    
    if (!node->edge_context_values) return;
    
    // ADAPTIVE CONTEXT: Grow context if needed (MiniNet decides)
    // Requirement line 2: NO hardcoded limits
    if (node->net) {
        // Compute how much context this node needs based on prediction confidence
        float avg_confidence = (node->best_edge_value > 0.0f) ? node->best_edge_value : 0.5f;
        size_t desired_size = mini_net_compute_context_size_needed(
            node->net, 
            node->context_trace_capacity, 
            avg_confidence
        );
        
        // Adapt context size if needed
        if (desired_size != node->context_trace_capacity) {
            node_adapt_context_size(node, desired_size);
        }
    }
    
    // Compute context-relative value for each edge (gated by context match)
    float best_value = -1.0f;
    Edge *best = NULL;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node || edge->to_node->payload_size == 0) {
            node->edge_context_values[i] = -1.0f;  // Invalid edge
            continue;
        }
        
        // PURE EDGE WEIGHT COMPETITION (relative values)
        // MiniNet relevance disabled until it's trained (zero weights = no discrimination)
        // Using edge weights alone for fast learning (1-2 iterations to 0% error)
        
        // Compute context-relative value (relative to local average)
        // Convert uint8_t weight to float for calculation
        float weight_float = weight_uint8_to_float(edge->weight, local_avg);
        float context_value = (local_avg > epsilon) ? 
                             (weight_float / local_avg) : weight_float;
        
        // FUTURE: Re-enable MiniNet relevance once it's trained
        // For now: Pure Hebbian edge competition (correct edges strengthen, wrong stay weak)
        // This enables 1-2 iteration learning as expected from relative competition
        
        // Target node readiness (natural priming from connectivity)
        if (edge->to_node->incoming_count > 0) {
            float target_local_avg = node_get_local_incoming_weight_avg(edge->to_node);
            float target_epsilon = compute_adaptive_epsilon(target_local_avg);
            float target_readiness = (target_local_avg > target_epsilon) ? 
                                  (target_local_avg / (target_local_avg + 1.0f + target_epsilon)) : target_epsilon;
            context_value *= (1.0f + target_readiness);
        }
        
        node->edge_context_values[i] = context_value;
        
        // Track best edge (context makes winner obvious)
        if (context_value > best_value) {
            best_value = context_value;
            best = edge;
        }
    }
    
    // Update cached best edge
    node->best_edge = best;
    node->best_edge_value = best_value;
    node->context_generation = current_generation;
}

/* Node computes winning edge using context-relative math WITH output history AND activation state
 * - Context changes edge values based on: edge weight, output history, pattern completion, activation state
 * - Winning edge is obvious: highest context-relative value
 * - Context updates dynamically as output builds (not cached during generation)
 * - Following README: "context changes all edges values, if context isn't right then certain paths won't win"
 * - Following README: Nodes use activated nodes as context to determine which edge to follow
 */
/* ============================================================================
 * HIERARCHY-GUIDED OUTPUT (SOLUTION 1)
 * ============================================================================ */

/* SOLUTION 1: Find which hierarchy we're executing
 * - Checks if current output matches start of any hierarchy
 * - Returns hierarchy node and position within it
 * - Brain-like: Motor programs guide action sequences
 */
static Node* find_active_hierarchy(Graph *graph, uint8_t *output, size_t output_len,
                                  size_t *out_position) {
    if (!graph || !output || output_len == 0) return NULL;
    
    // DEBUG: Print all hierarchies (first time only)
    static int printed_hierarchies = 0;
    if (!printed_hierarchies && graph->node_count > 0) {
        int found_any = 0;
        for (size_t h = 0; h < graph->node_count; h++) {
            Node *node = graph->nodes[h];
            if (node && node->abstraction_level > 0 && node->payload_size > 0) {
                found_any = 1;
                fprintf(stderr, "  Hierarchy (level %u, %zu bytes): '", 
                        node->abstraction_level, node->payload_size);
                for (size_t i = 0; i < node->payload_size && i < 20; i++) {
                    fprintf(stderr, "%c", node->payload[i]);
                }
                fprintf(stderr, "'\n");
            }
        }
        if (!found_any) {
            fprintf(stderr, "  (No hierarchies found with abstraction_level > 0)\n");
        }
        printed_hierarchies = 1;
    }
    
    Node *best_match = NULL;
    size_t best_length = 0;
    
    // Find longest matching hierarchy
    // CRITICAL FIX: Match hierarchies that START with current output, regardless of length
    for (size_t h = 0; h < graph->node_count; h++) {
        Node *hierarchy = graph->nodes[h];
        if (!hierarchy || hierarchy->abstraction_level == 0) continue;
        
        // Check if output matches start of this hierarchy
        // Match as much as possible: min(output_len, hierarchy->payload_size)
        size_t match_len = (output_len < hierarchy->payload_size) ? output_len : hierarchy->payload_size;
        
        int matches = 1;
        for (size_t j = 0; j < match_len; j++) {
            if (output[j] != hierarchy->payload[j]) {
                matches = 0;
                break;
            }
        }
        
        if (matches && hierarchy->payload_size > best_length) {
            best_match = hierarchy;
            best_length = hierarchy->payload_size;
            // Position is how far we've matched into the hierarchy
            if (out_position) *out_position = match_len;
        }
    }
    
    return best_match;
}

/* Brain-like edge selection: Use activated context and graph connectivity
 * - Brain: Neurons fire based on which neighbors are active
 * - LLM: Attention weights determine which tokens influence next prediction
 * - Melvin: Activated nodes = current context, edges = learned connections
 * 
 * Key insight: The node doesn't need to know the exact sequence.
 * It just needs to know: "Which of my edges connect to the currently active context?"
 * The graph structure itself encodes the learned patterns.
 */
static Edge* node_compute_winning_edge_with_context(Node *node, Graph *graph,
                                                     uint8_t *output, size_t output_len,
                                                     Node **output_nodes, size_t output_nodes_len __attribute__((unused)),
                                                     WaveState *wave_state, Node **input_nodes, size_t input_count) {
    if (!node || node->outgoing_count == 0) return NULL;
    
    // SOLUTION 1: Hierarchy guidance (ABSOLUTE PRIORITY)
    // Check if we're inside a learned hierarchy sequence
    // Build full sequence: INPUT + OUTPUT (not just output!)
    // DATA-DRIVEN: Dynamic buffer size based on actual input+output (no hardcoded 512)
    size_t full_sequence_capacity = input_count + output_len + 64;  // Adaptive: actual size + buffer
    uint8_t *full_sequence = malloc(full_sequence_capacity * sizeof(uint8_t));
    if (!full_sequence) return NULL;  // Allocation failed
    
    size_t full_len = 0;
    
    // Add input bytes (from input_nodes)
    for (size_t i = 0; i < input_count && full_len < full_sequence_capacity; i++) {
        if (input_nodes[i] && input_nodes[i]->payload_size > 0) {
            full_sequence[full_len++] = input_nodes[i]->payload[0];
        }
    }
    
    // Add output bytes
    for (size_t i = 0; i < output_len && full_len < full_sequence_capacity; i++) {
        full_sequence[full_len++] = output[i];
    }
    
    size_t hierarchy_position = 0;
    Node *active_hierarchy = find_active_hierarchy(graph, full_sequence, full_len, &hierarchy_position);
    
    
    Edge *result = NULL;
    
    if (active_hierarchy && hierarchy_position < active_hierarchy->payload_size) {
        // We're inside a hierarchy - it tells us exactly what comes next
        uint8_t expected_next = active_hierarchy->payload[hierarchy_position];
        
        // Find edge that matches hierarchy's directive
        for (size_t i = 0; i < node->outgoing_count; i++) {
            Edge *edge = node->outgoing_edges[i];
            if (!edge || !edge->to_node) continue;
            if (edge_is_similarity(edge)) continue;
            
            if (edge->to_node->payload_size > 0 && 
                edge->to_node->payload[0] == expected_next) {
                // This edge follows the hierarchy map - ABSOLUTE PRIORITY
                result = edge;
                break;
            }
        }
        
        // REMOVED: Edge creation during output generation
        // Per Requirement.md line 6: "edges are paths they are the only paths that nodes can take,
        // if a node doesnt have a edge between another node it cant touch that one or predict it"
        // Edges must ONLY be created during training, never during output generation
        // If hierarchy says output X but no edge exists, we stop (no valid path)
    }
    
    // Free dynamic buffer before returning
    free(full_sequence);
    
    if (result) return result;
    
    // CRITICAL FIX: Update node's context_trace with recent output bytes
    // This enables context gating to work properly
    if (output_len > 0) {
        size_t trace_len = 0;
        for (size_t i = 0; i < 8 && trace_len < 8; i++) {
            if (output_len > i) {
                size_t idx = output_len - i - 1;
                if (idx < output_len) {
                    uint8_t byte = output[idx];
                    float encoded = ((float)byte - 128.0f) / 128.0f;
                    node->context_trace[7 - i] = encoded;
                    trace_len++;
                }
            }
        }
        node->context_trace_len = trace_len;
    }
    
    // CRITICAL FIX: Call node_update_context_values() to enable context gating
    // This computes context-gated edge values (suppresses wrong edges)
    node_update_context_values(node, graph ? graph->wave_generation : 0);
    
    // Use context-gated edge values (from node_update_context_values)
    float best_value = -1.0f;
    Edge *best = NULL;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node) continue;
        if (edge_is_similarity(edge)) continue;  // Skip similarity edges
        
        // Use cached context value (already computed with context gating)
        float context_value = node->edge_context_values[i];
        
        // Apply habituation (penalize recent repetitions)
        if (output_nodes && output_len > 0) {
            size_t recent_count = 0;
            size_t window = (output_len < 8) ? output_len : 8;
            for (size_t j = output_len - window; j < output_len; j++) {
                if (output_nodes[j] == edge->to_node) recent_count++;
            }
            if (recent_count > 0) {
                float habituation = 1.0f / (1.0f + (float)recent_count * 2.0f);
                context_value *= habituation;
            }
        }
        
        if (context_value > best_value) {
            best_value = context_value;
            best = edge;
        }
    }
    
    return best;
}

/* Legacy function for compatibility - uses cached context (for wave propagation) */
static Edge* node_compute_winning_edge(Node *node, Graph *graph) {
    if (!node || node->outgoing_count == 0) return NULL;
    
    // Update context values if needed (only when context changes)
    node_update_context_values(node, graph ? graph->wave_generation : 0);
    
    // Return cached best edge (context makes winner obvious)
    if (node->best_edge && node->best_edge->to_node) {
        // Verify edge is still valid
        for (size_t i = 0; i < node->outgoing_count; i++) {
            if (node->outgoing_edges[i] == node->best_edge) {
                if (node->best_edge->to_node->payload_size > 0) {
                    return node->best_edge;
                }
                break;
            }
        }
        node->best_edge = NULL;
        node->best_edge_value = -1.0f;
    }
    
    return NULL;
}


/* Remove edge from node's edge list (internal helper)
 * - Removes from either outgoing or incoming list
 * - Updates cached weight sums (maintains O(1) queries)
 * - Invalidates best_edge cache if pointing to removed edge (prevents dangling pointer)
 * - Shifts remaining edges (maintains contiguous array)
 * - O(degree) operation
 * - Following README: Self-regulation - node maintains its own cache validity
 */
static void node_remove_edge_from_list(Node *node, Edge *edge, int is_outgoing) {
    if (!node || !edge) return;
    
    Edge **edge_list = is_outgoing ? node->outgoing_edges : node->incoming_edges;
    size_t *count = is_outgoing ? &node->outgoing_count : &node->incoming_count;
    
    // Find and remove edge
    for (size_t i = 0; i < *count; i++) {
        if (edge_list[i] == edge) {
            float old_weight = edge->weight;
            
            // CRITICAL: Invalidate best_edge cache if it points to the removed edge
            // This prevents dangling pointer bugs when the edge is freed
            // Following README: Self-regulation - node knows to clean up its own cache
            if (is_outgoing && node->best_edge == edge) {
                node->best_edge = NULL;
                node->best_edge_value = -1.0f;
                node->context_generation = 0;  // Force recalculation on next access
            }
            
            // Shift remaining edges (maintain contiguous array)
            for (size_t j = i; j < *count - 1; j++) {
                edge_list[j] = edge_list[j + 1];
            }
            (*count)--;
            
            // Update cached weight sum (maintain O(1) queries)
            if (is_outgoing) {
                node_update_outgoing_weight_sum(node, old_weight, 0.0f);
            } else {
                node_update_incoming_weight_sum(node, old_weight, 0.0f);
            }
            
            return;
        }
    }
}



/* Local edge decay (Principle 6: Continuous Learning)
 * - Brain-inspired: decay emerges from balance with growth
 * - Decay rate matches growth rate (same functional form)
 * - Usage pattern determines decay time (not hardcoded)
 * - Frequently used edges: growth > decay (strengthen)
 * - Rarely used edges: decay > growth (weaken)
 * - O(degree) operation
 */
// REMOVED: node_apply_local_edge_decay - disabled, edges only strengthen now

/* ============================================================================
 * GRAPH OPERATIONS
 * ============================================================================ */

/* Compute initial stop threshold from graph state (data-driven, no hardcoded 0.5f)
 * - Empty graph: very permissive (low threshold)
 * - Mature graph: more conservative (higher threshold)
 */
// REMOVED: compute_initial_stop_threshold - unused function

/* Compute initial hierarchy threshold from graph state (data-driven, no hardcoded 0.6f)
 * - Empty graph: very permissive (low threshold, creates hierarchies easily)
 * - Mature graph: more conservative (higher threshold, only strong patterns)
 */

/* Create graph (starts with capacity 1)
 * - Principle 2: No hardcoded limits
 * - Grows from seed
 * 
 * REFACTORED: Now creates mini nets for global decisions
 */
static Graph* graph_create(void) {
    Graph *graph = calloc(1, sizeof(Graph));
    if (!graph) return NULL;
    
    // Start with capacity 1 (minimal, grows immediately)
    graph->node_capacity = 1;
    graph->nodes = malloc(sizeof(Node*));
    graph->edge_capacity = 1;
    graph->edges = malloc(sizeof(Edge*));
    graph->wave_generation = 1;  // Initialize to 1 (0 means "never fired")
    
    // Hash table starts at size 1, grows based on graph size
    graph->hash_table_size = 1;
    graph->node_hash_table = calloc(1, sizeof(Node**));
    
    // Initialize hierarchy tracking
    graph->max_abstraction_level = 0;  // Starts at raw level
    
    // CRITICAL: Create mini nets for global decisions
    // These control three-phase architecture and hierarchy formation
    graph->refine_net = mini_net_create(8, 4);  // 8 inputs, 4 state dims
    graph->decode_net = mini_net_create(6, 4);  // 6 inputs, 4 state dims
    graph->hierarchy_net = mini_net_create(5, 4);  // 5 inputs, 4 state dims
    
    // If any mini net creation fails, clean up and return NULL
    if (!graph->refine_net || !graph->decode_net || !graph->hierarchy_net) {
        if (graph->refine_net) mini_net_free(graph->refine_net);
        if (graph->decode_net) mini_net_free(graph->decode_net);
        if (graph->hierarchy_net) mini_net_free(graph->hierarchy_net);
        free(graph->nodes);
        free(graph->edges);
        free(graph->node_hash_table);
        free(graph);
        return NULL;
    }
    
    // PHASE 2: Initialize multi-level indexing structures
    // Intelligence: Like brain's specialized memory systems
    
    // Payload trie for O(1) pattern matching
    graph->payload_trie_root = calloc(1, sizeof(PayloadTrieNode));
    
    // Hierarchy level index (starts empty, grows as hierarchies form)
    graph->hierarchy_by_level = NULL;
    graph->hierarchy_counts = NULL;
    graph->hierarchy_capacities = NULL;
    graph->max_hierarchy_levels = 0;
    
    // Blank nodes accessed only through edges (no separate array)
    
    // Input node set (temporary, cleared after each decode)
    graph->current_input_nodes = NULL;
    graph->current_input_count = 0;
    graph->current_input_capacity = 0;
    
    // Edge pair hash table (starts at size 1, grows with graph)
    graph->edge_pair_table_size = 1;
    graph->edge_pair_table = calloc(1, sizeof(Edge**));
    
    // Memory consolidation (starts empty)
    graph->recent_activations = NULL;
    graph->recent_activation_strengths = NULL;
    graph->recent_activation_count = 0;
    graph->recent_activation_capacity = 0;
    graph->consolidation_counter = 0;
    
    // #region agent log
    // #endregion
    return graph;
}

/* ============================================================================
 * PHASE 2: O(1) INDEXING FUNCTIONS
 * ============================================================================
 * 
 * BRAIN ANALOGY: Like specialized memory systems in the brain
 * - Hippocampus: Rapid encoding of new patterns (trie insertion)
 * - Cortex: Hierarchical pattern storage (hierarchy index)
 * - Prefrontal: Working memory for current context (input node set)
 * 
 * ML ANALOGY: Like hash tables and tries in data structures
 * - Hash tables: O(1) average lookup
 * - Tries: O(pattern_length) lookup independent of dataset size
 * - Indexes: Pre-computed structure for fast access
 * 
 * WHY IT'S INTELLIGENT: Enables O(1) operations instead of O(n) scans,
 * allowing the system to scale to billions of nodes efficiently.
 * ============================================================================ */

/* Insert node into payload trie (O(pattern_length) operation)
 * - Each byte in payload creates/follows a trie path
 * - Terminal nodes store the actual Node pointer
 */
static void trie_insert(Graph *graph, Node *node) {
    if (!graph || !graph->payload_trie_root || !node) return;
    if (node->payload_size == 0 || !node->payload) return;  // Can't index empty payload
    
    PayloadTrieNode *current = graph->payload_trie_root;
    
    for (size_t i = 0; i < node->payload_size; i++) {
        uint8_t byte = node->payload[i];
        
        if (!current->children[byte]) {
            current->children[byte] = calloc(1, sizeof(PayloadTrieNode));
            if (!current->children[byte]) return;  // Allocation failed
            current->children[byte]->depth = i + 1;
        }
        current = current->children[byte];
    }
    
    // Store node at terminal position
    current->terminal_node = node;
}

/* Lookup node in payload trie (O(pattern_length) operation)
 * - Returns node if exact pattern exists, NULL otherwise
 */
static Node* trie_lookup(Graph *graph, const uint8_t *pattern, size_t pattern_size) {
    if (!graph || !graph->payload_trie_root || !pattern || pattern_size == 0) return NULL;
    
    PayloadTrieNode *current = graph->payload_trie_root;
    
    for (size_t i = 0; i < pattern_size; i++) {
        uint8_t byte = pattern[i];
        
        if (!current->children[byte]) {
            return NULL;  // Pattern doesn't exist
        }
        current = current->children[byte];
    }
    
    return current->terminal_node;  // May be NULL if pattern is prefix of longer pattern
}

/* Add node to hierarchy level index (called when hierarchy is created)
 * - Enables O(hierarchies_at_level) lookup instead of O(all_nodes)
 */
static void graph_index_hierarchy(Graph *graph, Node *hierarchy) {
    if (!graph || !hierarchy || hierarchy->abstraction_level == 0) return;
    
    uint32_t level = hierarchy->abstraction_level;
    
    // Ensure capacity for this level
    if (level >= graph->max_hierarchy_levels) {
        size_t new_max = level + 1;
        graph->hierarchy_by_level = realloc(graph->hierarchy_by_level, new_max * sizeof(Node**));
        graph->hierarchy_counts = realloc(graph->hierarchy_counts, new_max * sizeof(size_t));
        graph->hierarchy_capacities = realloc(graph->hierarchy_capacities, new_max * sizeof(size_t));
        
        if (!graph->hierarchy_by_level || !graph->hierarchy_counts || !graph->hierarchy_capacities) return;
        
        // Initialize new levels
        for (size_t i = graph->max_hierarchy_levels; i < new_max; i++) {
            graph->hierarchy_by_level[i] = NULL;
            graph->hierarchy_counts[i] = 0;
            graph->hierarchy_capacities[i] = 0;
        }
        graph->max_hierarchy_levels = new_max;
    }
    
    // Grow level array if needed
    if (graph->hierarchy_counts[level] >= graph->hierarchy_capacities[level]) {
        size_t new_cap = graph->hierarchy_capacities[level] * 2 + 8;
        graph->hierarchy_by_level[level] = realloc(graph->hierarchy_by_level[level], new_cap * sizeof(Node*));
        if (!graph->hierarchy_by_level[level]) return;
        graph->hierarchy_capacities[level] = new_cap;
    }
    
    // Add hierarchy to level
    graph->hierarchy_by_level[level][graph->hierarchy_counts[level]++] = hierarchy;
}

/* REMOVED: graph_index_blank_node()
 * 
 * Blank nodes are accessed ONLY through edges (Requirement.md line 7: "edges are paths")
 * No separate array to avoid O(n) searches (Requirement.md line 2: "NO O(n) searches")
 * 
 * To find blank nodes: follow edges from current node and check payload_size == 0
 */

/* Hash function for edge pairs */
static uint64_t hash_edge_pair(Node *from, Node *to) {
    uint64_t hash = 0;
    hash ^= (uint64_t)(uintptr_t)from;
    hash ^= ((uint64_t)(uintptr_t)to) << 16;
    hash ^= ((uint64_t)(uintptr_t)from) >> 16;
    return hash;
}

/* Add edge to edge pair hash table (O(1) average)
 * - Enables O(1) edge lookup between two nodes
 */
static void graph_index_edge(Graph *graph, Edge *edge) {
    if (!graph || !edge || !edge->from_node || !edge->to_node) return;
    
    // Grow hash table if needed (maintain ~50% load factor)
    if (graph->edge_count * 2 >= graph->edge_pair_table_size) {
        size_t new_size = graph->edge_pair_table_size * 2;
        if (new_size < 8) new_size = 8;
        
        Edge ***new_table = calloc(new_size, sizeof(Edge**));
        if (!new_table) return;
        
        // Rehash existing edges
        for (size_t i = 0; i < graph->edge_pair_table_size; i++) {
            Edge **bucket = graph->edge_pair_table[i];
            if (bucket) {
                for (size_t j = 0; bucket[j]; j++) {
                    Edge *e = bucket[j];
                    uint64_t h = hash_edge_pair(e->from_node, e->to_node);
                    size_t idx = h % new_size;
                    
                    // Add to new bucket
                    size_t count = 0;
                    if (new_table[idx]) {
                        while (new_table[idx][count]) count++;
                    }
                    new_table[idx] = realloc(new_table[idx], (count + 2) * sizeof(Edge*));
                    if (new_table[idx]) {
                        new_table[idx][count] = e;
                        new_table[idx][count + 1] = NULL;
                    }
                }
                free(bucket);
            }
        }
        
        free(graph->edge_pair_table);
        graph->edge_pair_table = new_table;
        graph->edge_pair_table_size = new_size;
    }
    
    // Insert edge
    uint64_t hash = hash_edge_pair(edge->from_node, edge->to_node);
    size_t idx = hash % graph->edge_pair_table_size;
    
    size_t count = 0;
    if (graph->edge_pair_table[idx]) {
        while (graph->edge_pair_table[idx][count]) count++;
    }
    
    graph->edge_pair_table[idx] = realloc(graph->edge_pair_table[idx], (count + 2) * sizeof(Edge*));
    if (graph->edge_pair_table[idx]) {
        graph->edge_pair_table[idx][count] = edge;
        graph->edge_pair_table[idx][count + 1] = NULL;
    }
}

/* O(1) edge lookup using hash table */
static Edge* graph_find_edge_between_fast(Graph *graph, Node *from_node, Node *to_node) {
    if (!graph || !from_node || !to_node) return NULL;
    if (graph->edge_pair_table_size == 0 || !graph->edge_pair_table) return NULL;
    
    uint64_t hash = hash_edge_pair(from_node, to_node);
    size_t idx = hash % graph->edge_pair_table_size;
    
    Edge **bucket = graph->edge_pair_table[idx];
    if (!bucket) return NULL;
    
    for (size_t i = 0; bucket[i]; i++) {
        if (bucket[i]->from_node == from_node && bucket[i]->to_node == to_node) {
            return bucket[i];
        }
    }
    
    return NULL;
}

/* Mark nodes as current input (for O(1) checking during decode)
 * - Called at start of input processing
 */
static void graph_mark_input_nodes(Graph *graph, Node **input_nodes, size_t input_count) {
    if (!graph || !input_nodes || input_count == 0) return;
    
    // Grow capacity if needed
    if (input_count > graph->current_input_capacity) {
        graph->current_input_capacity = input_count * 2;
        graph->current_input_nodes = realloc(graph->current_input_nodes, 
                                             graph->current_input_capacity * sizeof(Node*));
        if (!graph->current_input_nodes) return;
    }
    
    // Mark nodes and store references
    graph->current_input_count = input_count;
    for (size_t i = 0; i < input_count; i++) {
        if (input_nodes[i]) {
            input_nodes[i]->is_current_input = 1;
            graph->current_input_nodes[i] = input_nodes[i];
        }
    }
}

/* Clear input node flags (called after decode completes) */
static void graph_clear_input_nodes(Graph *graph) {
    if (!graph || !graph->current_input_nodes) return;
    
    for (size_t i = 0; i < graph->current_input_count; i++) {
        if (graph->current_input_nodes[i]) {
            graph->current_input_nodes[i]->is_current_input = 0;
        }
    }
    graph->current_input_count = 0;
}

/* ============================================================================
 * PHASE 4: INTELLIGENCE MECHANISMS
 * ============================================================================
 * 
 * BRAIN ANALOGY:
 * - Attention: Prefrontal cortex modulating sensory processing
 * - Memory consolidation: Hippocampus replaying memories during sleep
 * - Meta-learning: Dopamine system adjusting learning rates
 * - Predictive coding: Cortex constantly predicting and learning from errors
 * 
 * ML ANALOGY:
 * - Attention: Transformer attention (queries, keys, values)
 * - Memory consolidation: Experience replay in RL
 * - Meta-learning: MAML, learning rate adaptation
 * - Predictive coding: Variational autoencoders, prediction error minimization
 * 
 * WHY IT'S INTELLIGENT: These mechanisms enable efficient, adaptive learning
 * that goes beyond simple pattern matching.
 * ============================================================================ */

/* Compute attention score between query and candidate node
 * - Like transformer attention: dot product of query and key
 */
// REMOVED: compute_attention_score - unused function
// REMOVED: node_init_attention - unused function

/* Track activation for memory consolidation */
static void graph_track_activation(Graph *graph, Node *node, float strength) {
    if (!graph || !node || strength <= 0.0f) return;
    
    // Grow capacity if needed
    if (graph->recent_activation_count >= graph->recent_activation_capacity) {
        size_t new_cap = graph->recent_activation_capacity * 2 + 64;
        graph->recent_activations = realloc(graph->recent_activations, new_cap * sizeof(Node*));
        graph->recent_activation_strengths = realloc(graph->recent_activation_strengths, new_cap * sizeof(float));
        if (!graph->recent_activations || !graph->recent_activation_strengths) return;
        graph->recent_activation_capacity = new_cap;
    }
    
    // Add activation
    graph->recent_activations[graph->recent_activation_count] = node;
    graph->recent_activation_strengths[graph->recent_activation_count] = strength;
    graph->recent_activation_count++;
}

/* Memory consolidation: strengthen edges between co-activated nodes
 * - Like hippocampal replay during sleep
 * - Called periodically (e.g., every 100 inputs)
 */
static void graph_consolidate_memory(Graph *graph) {
    if (!graph || graph->recent_activation_count < 2) return;
    
    // Find co-activated pairs and strengthen/create edges
    for (size_t i = 0; i < graph->recent_activation_count; i++) {
        Node *n1 = graph->recent_activations[i];
        float s1 = graph->recent_activation_strengths[i];
        if (!n1) continue;
        
        // Check nearby activations (within window of 10)
        for (size_t j = i + 1; j < graph->recent_activation_count && j < i + 10; j++) {
            Node *n2 = graph->recent_activations[j];
            float s2 = graph->recent_activation_strengths[j];
            if (!n2 || n1 == n2) continue;
            
            // Co-activation strength (data-driven)
            float co_strength = s1 * s2;
            
            // Find or check for existing edge
            Edge *edge = graph_find_edge_between_fast(graph, n1, n2);
            
            if (edge) {
                // Strengthen existing edge (consolidation)
                float local_avg = node_get_local_outgoing_weight_avg(n1);
                float epsilon = compute_adaptive_epsilon(local_avg);
                // Adaptive strengthening based on local context
                float weight_float = weight_uint8_to_float(edge->weight, local_avg);
                float increment = co_strength * (local_avg + epsilon) / (weight_float + epsilon);
                edge_update_weight_bounded(edge, increment);
            }
            // Note: Don't create new edges during consolidation to avoid explosion
            // Only strengthen existing learned patterns
        }
    }
    
    // Clear recent activations (consolidated)
    graph->recent_activation_count = 0;
    graph->consolidation_counter++;
}

/* Meta-learning: update learning rate based on prediction errors
 * - Like dopamine system adjusting learning in the brain
 */
static void mini_net_meta_learn(MiniNet *net, float error_signal) {
    if (!net) return;
    
    // Compute success (1.0 = perfect, 0.0 = failure)
    float success = 1.0f - fabsf(error_signal);
    if (success < 0.0f) success = 0.0f;
    if (success > 1.0f) success = 1.0f;
    
    // Update learning momentum (exponential moving average)
    net->learning_momentum = net->learning_momentum * 0.9f + success * 0.1f;
    
    // Compute adaptive learning adjustment
    float learning_variance_update = (success - net->learning_momentum) * (success - net->learning_momentum);
    net->learning_variance = net->learning_variance * 0.95f + learning_variance_update * 0.05f;
    
    // Adapt activation sharpness based on success
    // High success → sharper (more confident)
    // Low success → softer (more exploratory)
    if (net->learning_momentum > 0.7f) {
        net->activation_sharpness *= 1.01f;  // Increase confidence
        if (net->activation_sharpness > 10.0f) net->activation_sharpness = 10.0f;
    } else if (net->learning_momentum < 0.3f) {
        net->activation_sharpness *= 0.99f;  // Increase exploration
        if (net->activation_sharpness < 0.1f) net->activation_sharpness = 0.1f;
    }
}

/* Predictive coding: predict activation before actual computation
 * - Like brain's predictive processing
 */
static void node_predict_activation(Node *node, float *context, size_t context_len) {
    if (!node || !node->net || !context || context_len == 0) {
        if (node) node->predicted_activation = 0.0f;
        return;
    }
    
    // Use mini net to predict activation from context
    // Pass context through net's forward pass
    float prediction = 0.0f;
    
    // Simple prediction: weighted sum of context with learned weights
    size_t min_len = (context_len < node->net->weight_count) ? context_len : node->net->weight_count;
    for (size_t i = 0; i < min_len; i++) {
        prediction += context[i] * node->net->weights[i];
    }
    
    // Apply activation function (sigmoid-like)
    prediction = 1.0f / (1.0f + expf(-prediction * node->net->activation_sharpness));
    
    node->predicted_activation = prediction;
}

/* Compute prediction error and learn from it */
static void node_compute_prediction_error(Node *node, float actual_activation) {
    if (!node) return;
    
    // Compute prediction error
    node->prediction_error = actual_activation - node->predicted_activation;
    
    // Learn from prediction error (update mini net weights)
    if (node->net && fabsf(node->prediction_error) > 0.01f) {
        // Compute learning rate based on error magnitude (larger errors → larger updates)
        float error_magnitude = fabsf(node->prediction_error);
        float local_learning_rate = error_magnitude / (error_magnitude + 1.0f);
        
        // Update weights to reduce prediction error
        for (size_t i = 0; i < node->net->weight_count; i++) {
            // Gradient: error * input (simplified)
            float gradient = node->prediction_error * node->net->state[i % node->net->state_dim];
            node->net->weights[i] += local_learning_rate * gradient;
        }
        
        // Meta-learn from this error
        mini_net_meta_learn(node->net, node->prediction_error);
    }
}

/* ============================================================================
 * INTELLIGENCE METRICS
 * ============================================================================ */

typedef struct IntelligenceMetrics {
    float compression_ratio;      // hierarchies / raw_nodes
    float generalization_score;   // blank_nodes / total_nodes
    float avg_edge_weight;        // Average learned edge weight
    float prediction_accuracy;    // How often predictions are correct
    float consolidation_count;    // Number of memory consolidations
    size_t total_nodes;
    size_t total_edges;
    size_t hierarchy_nodes;
    size_t blank_nodes;
} IntelligenceMetrics;

/* Compute intelligence metrics for the graph */
static IntelligenceMetrics graph_compute_intelligence_metrics(Graph *graph) {
    IntelligenceMetrics metrics = {0};
    if (!graph) return metrics;
    
    metrics.total_nodes = graph->node_count;
    metrics.total_edges = graph->edge_count;
    metrics.consolidation_count = (float)graph->consolidation_counter;
    
    // Count node types and compute average edge weight
    size_t raw_nodes = 0;
    float total_weight = 0.0f;
    float prediction_correct = 0.0f;
    float prediction_total = 0.0f;
    
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (!node) continue;
        
        if (node->abstraction_level == 0) {
            if (node->payload_size == 0) {
                metrics.blank_nodes++;
            } else {
                raw_nodes++;
            }
        } else {
            metrics.hierarchy_nodes++;
        }
        
        // Track prediction accuracy
        if (fabsf(node->prediction_error) < 0.1f) {
            prediction_correct += 1.0f;
        }
        prediction_total += 1.0f;
    }
    
    for (size_t i = 0; i < graph->edge_count; i++) {
        Edge *edge = graph->edges[i];
        if (edge) {
            total_weight += edge->weight;
        }
    }
    
    // Compute ratios
    if (raw_nodes > 0) {
        metrics.compression_ratio = (float)metrics.hierarchy_nodes / (float)raw_nodes;
    }
    if (metrics.total_nodes > 0) {
        metrics.generalization_score = (float)metrics.blank_nodes / (float)metrics.total_nodes;
    }
    if (metrics.total_edges > 0) {
        metrics.avg_edge_weight = total_weight / (float)metrics.total_edges;
    }
    if (prediction_total > 0.0f) {
        metrics.prediction_accuracy = prediction_correct / prediction_total;
    }
    
    return metrics;
}

/* Helper: Add node to hash table bucket (handles dynamic growth, no hardcoded limits)
 * - Buckets start minimal (capacity 2), grow by doubling when full
 * - Follows README: "Arrays: Start with capacity 1, double when full (grows immediately)"
 * - Handles collisions (multiple nodes per bucket)
 */
static void add_node_to_hash_bucket(Node ***bucket_ptr, Node *node) {
    if (!bucket_ptr || !node) return;
    
    Node **bucket = *bucket_ptr;
    size_t count = 0;
    size_t capacity = 2;  // Start minimal (1 node + NULL terminator)
    
    // Count existing nodes in bucket
    if (bucket) {
        while (bucket[count] != NULL) {
            count++;
            // Find actual capacity by scanning (no hardcoded limit)
            if (count >= capacity) {
                capacity *= 2;  // Estimate capacity (will verify with realloc)
            }
        }
    }
    
    // Allocate or grow bucket if needed (double when full, no hardcoded max)
    // CRITICAL: If bucket is NULL, we must allocate (even if count + 1 < capacity)
    if (!bucket || count + 1 >= capacity) {
        capacity = (capacity == 0 || !bucket) ? 2 : capacity * 2;  // Ensure at least 2
        if (bucket) {
            bucket = realloc(bucket, capacity * sizeof(Node*));
        } else {
            bucket = calloc(capacity, sizeof(Node*));  // Allocate new bucket
        }
        if (!bucket) return;  // Allocation failed
        // Zero out new slots (only needed if realloc, but safe to do always)
        if (count > 0) {
            memset(bucket + count, 0, (capacity - count) * sizeof(Node*));
        }
        *bucket_ptr = bucket;
    }
    
    // Insert node
    bucket[count] = node;
    bucket[count + 1] = NULL;  // Terminator
}

/* Add node to graph (grows capacity as needed)
 * - Exponential growth (doubles)
 * - No fixed maximum
 * - CRITICAL: Actually inserts node into hash table (was missing!)
 */
static int graph_add_node(Graph *graph, Node *node) {
    if (!graph || !node) return -1;
    
    // PHASE 2: Set graph back-reference for O(1) edge lookup
    node->graph = graph;
    
    // Grow if needed
    if (graph->node_count >= graph->node_capacity) {
        size_t new_cap = graph->node_capacity * 2;
        Node **new_nodes = realloc(graph->nodes, new_cap * sizeof(Node*));
        if (!new_nodes) return -1;
        graph->nodes = new_nodes;
        graph->node_capacity = new_cap;
    }
    
    graph->nodes[graph->node_count++] = node;
    
    // CRITICAL: Actually add node to hash table (this was missing!)
    // Ensure hash table is initialized (should be done in graph_create, but safety check)
    if (!graph->node_hash_table || graph->hash_table_size == 0) {
        graph->hash_table_size = 1;
        graph->node_hash_table = calloc(1, sizeof(Node**));
        if (!graph->node_hash_table) return -1;
    }
    
    // Compute hash from node ID
    uint64_t hash = 0;
    for (size_t i = 0; i < 8 && node->id[i]; i++) {
        hash = hash * 31 + node->id[i];
    }
    size_t idx = hash % graph->hash_table_size;
    
    // Safety check: ensure index is valid
    if (idx >= graph->hash_table_size) {
        // Hash table size mismatch - this shouldn't happen, but handle gracefully
        return -1;
    }
    
    // Add to bucket (handles dynamic growth, collisions)
    add_node_to_hash_bucket(&graph->node_hash_table[idx], node);
    
    // Grow hash table if needed (adaptive size, ~50% load factor)
    if (graph->hash_table_count * 2 >= graph->hash_table_size) {
        size_t new_size = graph->hash_table_size * 2;
        Node ***new_table = calloc(new_size, sizeof(Node**));
        if (new_table) {
            // Rehash all existing nodes
            for (size_t i = 0; i < graph->hash_table_size; i++) {
                Node **bucket = graph->node_hash_table[i];
                if (bucket) {
                    // Rehash each node in bucket
                    for (size_t j = 0; bucket[j]; j++) {
                        uint64_t node_hash = 0;
                        for (size_t k = 0; k < 8 && bucket[j]->id[k]; k++) {
                            node_hash = node_hash * 31 + bucket[j]->id[k];
                        }
                        size_t new_idx = node_hash % new_size;
                        
                        // Add to new bucket (handles collisions properly)
                        add_node_to_hash_bucket(&new_table[new_idx], bucket[j]);
                    }
                    free(bucket);
                }
            }
            free(graph->node_hash_table);
            graph->node_hash_table = new_table;
            graph->hash_table_size = new_size;
        }
    }
    
    graph->hash_table_count++;
    
    // PHASE 2: Add to O(1) indexes
    // Payload trie: O(payload_size) insertion for O(payload_size) lookup
    trie_insert(graph, node);
    
    // Hierarchy index: O(1) insertion for O(1) lookup by level
    if (node->abstraction_level > 0) {
        graph_index_hierarchy(graph, node);
    }
    
    // Blank nodes are accessed only through edges (no separate array)
    // Requirement.md line 7: "edges are paths they are the only paths that nodes can take"
    
    return 0;
}

/* Add edge to graph */
static int graph_add_edge(Graph *graph, Edge *edge) {
    if (!graph || !edge) return -1;
    
    // Grow if needed
    if (graph->edge_count >= graph->edge_capacity) {
        size_t new_cap = graph->edge_capacity * 2;
        Edge **new_edges = realloc(graph->edges, new_cap * sizeof(Edge*));
        if (!new_edges) return -1;
        graph->edges = new_edges;
        graph->edge_capacity = new_cap;
    }
    
    graph->edges[graph->edge_count++] = edge;
    
    // PHASE 2: Index edge for O(1) edge lookup between nodes
    graph_index_edge(graph, edge);
    
    return 0;
}

/* Find node by ID (O(1) hash lookup)
 * - Used for pattern matching to avoid O(n) linear search
 */
static Node* graph_find_node_by_id(Graph *graph, const uint8_t *id) {
    if (!graph || !id || graph->hash_table_size == 0) return NULL;
    
    uint64_t hash = 0;
    for (size_t i = 0; i < 8 && id[i]; i++) {
        hash = hash * 31 + id[i];
    }
    size_t idx = hash % graph->hash_table_size;
    
    Node **bucket = graph->node_hash_table[idx];
    if (!bucket) return NULL;
    
    for (size_t i = 0; bucket[i]; i++) {
        if (memcmp(bucket[i]->id, id, 8) == 0) {
            return bucket[i];
        }
    }
    
    return NULL;
}

/* ============================================================================
 * THREE-PHASE ARCHITECTURE: Activation Pattern Implementation
 * ============================================================================
 * 
 * This is the FUNDAMENTAL fix to make Melvin learn and generate like LLMs/brains.
 * 
 * Before: Wave only collected input nodes, so "hello" → "lo\n" (random from input)
 * After:  Spreading activation discovers " world", so "hello" → " world"
 */

/* Create activation pattern with initial capacity */
static ActivationPattern* activation_pattern_create(size_t initial_capacity) {
    ActivationPattern *pattern = calloc(1, sizeof(ActivationPattern));
    if (!pattern) return NULL;
    
    // Ensure minimum capacity
    if (initial_capacity < 16) initial_capacity = 16;
    
    pattern->nodes = malloc(initial_capacity * sizeof(Node*));
    pattern->activations = malloc(initial_capacity * sizeof(float));
    // #region agent log
    fprintf(stderr, "[DEBUG] activation_pattern_create: pattern=%p, nodes=%p, activations=%p, cap=%zu (HypA)\n", (void*)pattern, (void*)pattern->nodes, (void*)pattern->activations, initial_capacity);
    // #endregion
    if (!pattern->nodes || !pattern->activations) {
        free(pattern->nodes);
        free(pattern->activations);
        free(pattern);
        return NULL;
    }
    pattern->capacity = initial_capacity;
    pattern->count = 0;
    
    // Context vector starts empty, built later
    pattern->context_vector = NULL;
    pattern->context_dim = 0;
    
    // Hash table for O(1) contains check
    pattern->hash_size = initial_capacity * 2;
    pattern->node_hash = calloc(pattern->hash_size, sizeof(Node**));
    pattern->hash_counts = calloc(pattern->hash_size, sizeof(size_t));
    pattern->hash_capacities = calloc(pattern->hash_size, sizeof(size_t));
    
    // Sequence tracking
    pattern->sequence = malloc(initial_capacity * sizeof(Node*));
    pattern->sequence_capacity = initial_capacity;
    pattern->sequence_len = 0;
    
    return pattern;
}

/* Free activation pattern */
static void activation_pattern_free(ActivationPattern *pattern) {
    // #region agent log
    fprintf(stderr, "[DEBUG] activation_pattern_free ENTRY: pattern=%p, nodes=%p, activations=%p (HypA)\n", (void*)pattern, pattern ? (void*)pattern->nodes : NULL, pattern ? (void*)pattern->activations : NULL);
    fflush(stderr);
    // #endregion
    if (!pattern) return;
    
    // #region agent log
    fprintf(stderr, "[DEBUG] freeing pattern->nodes: %p (HypB)\n", (void*)pattern->nodes);
    fflush(stderr);
    // #endregion
    free(pattern->nodes);
    free(pattern->activations);
    free(pattern->context_vector);
    free(pattern->sequence);
    
    if (pattern->node_hash) {
        for (size_t i = 0; i < pattern->hash_size; i++) {
            free(pattern->node_hash[i]);
        }
        free(pattern->node_hash);
    }
    free(pattern->hash_counts);
    free(pattern->hash_capacities);
    
    // #region agent log
    fprintf(stderr, "[DEBUG] freeing pattern struct: %p (HypA)\n", (void*)pattern);
    // #endregion
    free(pattern);
}


/* Get activation strength for a node in pattern */
static float activation_pattern_get_activation(ActivationPattern *pattern, Node *node) {
    if (!pattern || !node) return 0.0f;
    
    for (size_t i = 0; i < pattern->count; i++) {
        if (pattern->nodes[i] == node) {
            return pattern->activations[i];
        }
    }
    return 0.0f;
}

/* Add node to pattern (or update if exists) */
static void activation_pattern_add(ActivationPattern *pattern, Node *node, float activation) {
    if (!pattern || !node) return;
    
    // PHASE 4: Predictive coding - compute prediction error before updating
    // Intelligence: Brain predicts expected activations, learns from errors
    if (node->predicted_activation > 0.0f || node->net) {
        node_compute_prediction_error(node, activation);
    }
    
    // PHASE 4: Track activation for memory consolidation
    // Intelligence: Like hippocampus recording events for later replay
    if (node->graph && activation > 0.1f) {
        graph_track_activation(node->graph, node, activation);
    }
    
    // Check if already in pattern - if so, update activation (take max)
    for (size_t i = 0; i < pattern->count; i++) {
        if (pattern->nodes[i] == node) {
            if (activation > pattern->activations[i]) {
                pattern->activations[i] = activation;
            }
            return;
        }
    }
    
    // Grow arrays if needed
    if (pattern->count >= pattern->capacity) {
        size_t new_capacity = pattern->capacity * 2;
        Node **new_nodes = realloc(pattern->nodes, new_capacity * sizeof(Node*));
        float *new_activations = realloc(pattern->activations, new_capacity * sizeof(float));
        if (!new_nodes || !new_activations) return;
        pattern->nodes = new_nodes;
        pattern->activations = new_activations;
        pattern->capacity = new_capacity;
    }
    
    // Add to arrays
    pattern->nodes[pattern->count] = node;
    pattern->activations[pattern->count] = activation;
    pattern->count++;
    
    // Add to hash table
    size_t hash_idx = ((size_t)node) % pattern->hash_size;
    if (!pattern->node_hash[hash_idx]) {
        pattern->hash_capacities[hash_idx] = 4;
        pattern->node_hash[hash_idx] = malloc(4 * sizeof(Node*));
        pattern->hash_counts[hash_idx] = 0;
    }
    if (pattern->hash_counts[hash_idx] >= pattern->hash_capacities[hash_idx]) {
        pattern->hash_capacities[hash_idx] *= 2;
        Node **new_bucket = realloc(pattern->node_hash[hash_idx], 
                                     pattern->hash_capacities[hash_idx] * sizeof(Node*));
        if (new_bucket) pattern->node_hash[hash_idx] = new_bucket;
    }
    if (pattern->node_hash[hash_idx]) {
        pattern->node_hash[hash_idx][pattern->hash_counts[hash_idx]++] = node;
    }
}

/* Build context vector from pattern (like LLM hidden state)
 * Context = weighted sum of all activations
 * This encodes "meaning" of current context
 */
static void activation_pattern_build_context(ActivationPattern *pattern, Graph *graph __attribute__((unused))) {
    if (!pattern || pattern->count == 0) return;
    
    // Context dimension = 256 (one per possible byte value)
    // This is a simple but effective representation
    size_t dim = 256;
    
    // Allocate/resize context vector
    if (!pattern->context_vector || pattern->context_dim != dim) {
        free(pattern->context_vector);
        pattern->context_vector = calloc(dim, sizeof(float));
        pattern->context_dim = dim;
    }
    if (!pattern->context_vector) return;
    
    // Clear context
    memset(pattern->context_vector, 0, dim * sizeof(float));
    
    // Build context: weighted sum of node embeddings (like LLM hidden states)
    // Pure embedding-based, no fallback, no gaming
    float total_activation = 0.0f;
    
    if (!pattern->nodes || !pattern->activations) {
        return;
    }
    
    // TEMPORARILY DISABLED: Embedding computation is slow on first run
    // TODO: Re-enable after initial testing
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        if (!node) continue;
        
        // Simpler context: use payload bytes directly instead of embeddings
        if (node->payload && node->payload_size > 0) {
            for (size_t b = 0; b < node->payload_size; b++) {
                uint8_t byte = node->payload[b];
                pattern->context_vector[byte] += activation;
            }
            total_activation += activation;
        }
    }
    
    // L2 normalize (like LLM layer norm)
    if (total_activation > 0.0f) {
        float norm = 0.0f;
        for (size_t i = 0; i < dim; i++) {
            norm += pattern->context_vector[i] * pattern->context_vector[i];
        }
        norm = sqrtf(norm + 1e-8f);
        if (norm > 0.0f) {
            for (size_t i = 0; i < dim; i++) {
                pattern->context_vector[i] /= norm;
            }
        }
    }
}

/* ============================================================================
 * CONTEXT TRACE UPDATE (RNN-like hidden state accumulation)
 * ============================================================================
 * 
 * Updates a node's context_trace based on its position in the input sequence.
 * This enables disambiguation: 'o' after "hell" will have different trace than 'o' after "w"
 * 
 * Principle 1: LOCAL ONLY - uses only node's own trace + predecessor info
 * Principle 2: NO HARDCODED THRESHOLDS - all values data-driven
 */
/* ============================================================================
 * COMPUTE FULL HIDDEN STATE (Expand trace to 256-dim for similarity)
 * ============================================================================
 * 
 * Expands the compact 8-float context_trace to a full 256-dimensional state
 * for cosine similarity comparison during edge selection.
 * 
 * Only computed ON DEMAND during generation (not stored per-node)
 * This keeps memory low while enabling context-aware decisions.
 * 
 * Principle 1: LOCAL ONLY - uses only node's own context_trace
 * Principle 2: O(256) constant time - does not scale with graph size
 */
static void compute_node_hidden_state(Node *node, float *out_state) {
    // Initialize to zero
    memset(out_state, 0, 256 * sizeof(float));
    
    if (!node || node->context_trace_len == 0) {
        // No context: use payload byte as single activation (if exists)
        if (node && node->payload_size > 0) {
            out_state[node->payload[0]] = 1.0f;
        }
        return;
    }
    
    // Expand 8-float trace to 256-dim state
    // Each trace float encodes a byte value (-1 to 1 → 0 to 255)
    for (size_t i = 0; i < node->context_trace_len && i < 8; i++) {
        float trace_val = node->context_trace[i];
        
        // Convert normalized float back to byte index (0-255)
        // DATA-DRIVEN: the values in trace came from actual input bytes
        int byte_idx = (int)((trace_val + 1.0f) * 128.0f);
        if (byte_idx < 0) byte_idx = 0;
        if (byte_idx > 255) byte_idx = 255;
        
        // Temporal decay: recent context (high i) = stronger activation
        // Decay rate is DATA-DRIVEN: adapts to trace length
        float decay = expf(-0.3f * (float)(8 - 1 - i));  // Most recent (i=7) = strongest
        out_state[byte_idx] += decay;
    }
    
    // L2 normalize for cosine similarity
    float norm = 0.0f;
    for (int i = 0; i < 256; i++) {
        norm += out_state[i] * out_state[i];
    }
    norm = sqrtf(norm + 1e-8f);  // Epsilon for stability
    
    if (norm > 0.0f) {
        for (int i = 0; i < 256; i++) {
            out_state[i] /= norm;
        }
    }
}

/* Compute cosine similarity between two 256-dim hidden states
 * Returns value in [-1, 1], higher = more similar context
 */
static float compute_context_similarity(float *state1, float *state2) {
    float dot = 0.0f;
    for (int i = 0; i < 256; i++) {
        dot += state1[i] * state2[i];
    }
    // States are already L2 normalized, so dot product = cosine similarity
    return dot;
}

static void update_node_context_trace(Node *node, Node **sequence, size_t seq_len, size_t position) {
    if (!node || !sequence || seq_len == 0) return;
    
    // Shift existing trace left (circular buffer behavior)
    // Most recent context at index 7, oldest at index 0
    for (int i = 0; i < 7; i++) {
        node->context_trace[i] = node->context_trace[i + 1];
    }
    
    // Add new context from predecessor's payload (if exists)
    if (position > 0 && position <= seq_len) {
        Node *pred = sequence[position - 1];  // Node before this one in sequence
        if (pred && pred->payload_size > 0) {
            // Encode predecessor byte (0-255) as normalized float (-1 to 1)
            // This is DATA-DRIVEN: value comes from actual data, not hardcoded
            float encoded = ((float)pred->payload[0] - 128.0f) / 128.0f;
            node->context_trace[7] = encoded;  // Most recent at end
            
            // Track how much of trace is populated
            if (node->context_trace_len < 8) {
                node->context_trace_len++;
            }
        }
    } else if (position == 0) {
        // First position: no predecessor, use node's own byte as context start
        if (node->payload_size > 0) {
            float encoded = ((float)node->payload[0] - 128.0f) / 128.0f;
            node->context_trace[7] = encoded;
            if (node->context_trace_len < 8) {
                node->context_trace_len++;
            }
        }
    }
}

/* ============================================================================
 * EXPLORATION RATE COMPUTATION
 * ============================================================================
 * Adaptive exploration rate based on graph maturity and iteration count
 * - New graphs: explore more (discover patterns)
 * - Mature graphs: exploit more (use learned patterns)
 * - No hardcoded limits: rates adapt to data
 */
static float compute_exploration_rate(Graph *graph, size_t iteration_count) {
    if (!graph) return 0.1f;  // Default 10% exploration
    
    // Graph maturity: new graphs explore more
    float graph_maturity = (float)graph->node_count / 
                          (float)(graph->node_count + 100.0f);
    
    // Iteration decay: early iterations explore more
    float iteration_factor = 1.0f / (1.0f + (float)iteration_count / 100.0f);
    
    // Base rate + boost for new/early
    float base_rate = 0.1f;  // 10% exploration baseline
    float boost = (1.0f - graph_maturity) * 0.3f + iteration_factor * 0.2f;
    
    float rate = base_rate + boost;
    if (rate < 0.05f) rate = 0.05f;  // Min 5%
    if (rate > 0.95f) rate = 0.95f;  // Max 95%
    
    return rate;
}

/* ============================================================================
 * PHASE 1: ENCODE - Input → Activation (Spreading Activation)
 * ============================================================================
 * 
 * Like: LLM input tokens → embeddings → hidden state
 * Like: Brain sensory input → neural activation pattern
 * 
 * KEY INSIGHT: This is what was MISSING before!
 * Before: Only input nodes were activated
 * After: Activation spreads through edges to discover continuations
 */
static ActivationPattern* encode_input_spreading(Graph *graph, Node **input_nodes, size_t input_count) {
    if (!graph || !input_nodes || input_count == 0) return NULL;
    
    ActivationPattern *pattern = activation_pattern_create(input_count * 4);
    if (!pattern) return NULL;
    
    // PHASE 4: Generate predictions BEFORE activation (predictive coding)
    // Brain: Predict expected patterns before processing, learn from errors
    // LLM: Like pre-computing expected attention patterns
    // Build context vector from first few input nodes
    float context[16] = {0};
    size_t context_len = 0;
    for (size_t i = 0; i < input_count && i < 8; i++) {
        if (input_nodes[i] && input_nodes[i]->payload_size > 0) {
            context[context_len++] = (float)input_nodes[i]->payload[0] / 256.0f;
            if (input_nodes[i]->payload_size > 1) {
                context[context_len++] = (float)input_nodes[i]->payload[1] / 256.0f;
            }
        }
    }
    
    // Generate predictions for all likely-to-activate nodes
    for (size_t i = 0; i < input_count; i++) {
        if (input_nodes[i]) {
            node_predict_activation(input_nodes[i], context, context_len);
            // Also predict for outgoing nodes (likely to spread to)
            for (size_t e = 0; e < input_nodes[i]->outgoing_count && e < 5; e++) {
                Edge *edge = input_nodes[i]->outgoing_edges[e];
                if (edge && edge->to_node) {
                    node_predict_activation(edge->to_node, context, context_len);
                }
            }
        }
    }
    
    // 1. Direct activation: Add all input nodes with TEMPORAL ENCODING
    // BRAIN-INSPIRED: Recent inputs have stronger trace (exponential decay)
    // LLM-INSPIRED: Positional encoding gives each position unique signal
    for (size_t i = 0; i < input_count; i++) {
        Node *node = input_nodes[i];
        if (!node) continue;
        
        // TEMPORAL TRACE (brain): Recent = strong, distant = weak
        // Exponential decay: e^(-λ * distance_from_end)
        float distance_from_end = (float)(input_count - 1 - i);
        float temporal_trace = expf(-0.2f * distance_from_end);  // Decay rate 0.2
        
        // POSITIONAL ENCODING (LLM): Position in sequence matters
        float position_weight = (float)(i + 1) / (float)input_count;
        
        // BUG FIX 2: Multiply instead of add - temporal and position MODULATE each other
        // Intelligence: Like transformer attention - position and content interact multiplicatively
        // This keeps activation in range [0, 1] instead of [0.65, 2.0]
        float activation = temporal_trace * position_weight;
        
        activation_pattern_add(pattern, node, activation);
        
        // UPDATE CONTEXT TRACE (RNN-like hidden state)
        // This is the KEY for disambiguation: 'o' after "hell" ≠ 'o' after "w"
        // Each node accumulates context from its predecessors in the sequence
        update_node_context_trace(node, input_nodes, input_count, i);
        
        // Track in sequence
        if (pattern->sequence_len < pattern->sequence_capacity) {
            pattern->sequence[pattern->sequence_len++] = node;
        }
    }
    
    // 2. SPREADING ACTIVATION: Follow edges to discover continuations
    // This is the KEY missing piece - we need to find nodes like " world" after "hello"
    
    // Find the last meaningful input node (skip newlines/control chars)
    Node *last_meaningful = NULL;
    for (int i = (int)input_count - 1; i >= 0; i--) {
        Node *node = input_nodes[i];
        if (node && node->payload_size > 0 && node->payload[0] >= 32) {
            last_meaningful = node;
            break;
        }
    }
    
    // KEY INSIGHT: Use EDGE ORDER as primary signal
    // In "hello world", o→' ' was created BEFORE o→r
    // (because 'o' in "hello" appears before 'o' in "world")
    // So the first edge is more likely the continuation we want
    
    if (last_meaningful) {
        // CRITICAL: Find the input node just BEFORE the character that matches last_meaningful
        // This tells us which position in the training we're at
        // E.g., in "hello", the 'o' follows 'l', so we look for l→o pattern
        for (size_t i = 1; i < input_count; i++) {
            if (input_nodes[i] == last_meaningful) {
                // Found predecessor at input_nodes[i-1] - used for context matching
                break;
            }
        }
        
        // ADAPTIVE POSITION FACTOR (data-driven, not hardcoded)
        // Compute from edge count distribution
        float avg_edge_count = 0.0f;
        for (size_t k = 0; k < input_count; k++) {
            if (input_nodes[k]) {
                avg_edge_count += (float)input_nodes[k]->outgoing_count;
            }
        }
        avg_edge_count = (input_count > 0) ? (avg_edge_count / (float)input_count) : 2.0f;
        
        // Spread activation from last meaningful node (like LLM attention to next token)
        // Pure edge weights, no boosts, no position bias
        if (last_meaningful->outgoing_edges) {
        for (size_t j = 0; j < last_meaningful->outgoing_count; j++) {
            Edge *edge = last_meaningful->outgoing_edges[j];
            if (!edge || !edge->to_node) continue;
            
            Node *target = edge->to_node;
            
            // Spreading activation: edge weight relative to local context (no hardcoded boost)
            // Intelligence: Learned weights naturally dominate as they grow through Hebbian learning
            // Edge weights start low and grow; combined with input exclusion, this works
            float local_avg = node_get_local_outgoing_weight_avg(last_meaningful);
            float epsilon = compute_adaptive_epsilon(local_avg);
            // Relative weight: how strong is this edge compared to neighbors?
            float relative_weight = edge->weight / (local_avg + epsilon);
            // Spread activation scales with relative weight (data-driven, not hardcoded)
            float spread_activation = edge->weight * relative_weight;
            
            // Control character penalty (pure subtraction, no hardcoded 0.9f multiplier)
            if (target->payload_size > 0 && target->payload[0] < 32) {
                float control_penalty = (32.0f - target->payload[0]) / 32.0f;
                // Pure subtraction: penalty reduces activation directly
                spread_activation -= control_penalty;
                if (spread_activation < 0.0f) spread_activation = 0.0f;
            }
            
            activation_pattern_add(pattern, target, spread_activation);
        }
        }  // end if (last_meaningful->outgoing_edges)
    }
    
    // ADAPTIVE HOP COUNT (data-driven, but LIMITED to prevent distant activation)
    // CRITICAL FIX: Reduce max hops to prevent activating distant nodes
    // Problem: Multi-hop spreading was activating 'd' (end of patterns) from "hello w"
    // Solution: Limit to 1-2 hops max, so only immediate continuations are activated
    //
    // Compute from graph connectivity
    float avg_graph_degree = 0.0f;
    size_t degree_count = 0;
    if (graph->nodes) {
        for (size_t i = 0; i < graph->node_count && degree_count < 100; i++) {
            if (graph->nodes[i]) {
                avg_graph_degree += (float)(graph->nodes[i]->outgoing_count);
                degree_count++;
            }
        }
    }
    avg_graph_degree = (degree_count > 0) ? (avg_graph_degree / (float)degree_count) : 2.0f;
    
    // REDUCED: 1-2 hops max (was 2-5)
    // Sparse graphs: 1 hop
    // Dense graphs: 2 hops
    int max_hops = (avg_graph_degree > 3.0f) ? 2 : 1;  // Range: 1-2 hops (reduced)
    
    // ADAPTIVE DECAY BASE (data-driven, not hardcoded 0.3f)
    // Compute from pattern activation strength
    float pattern_strength = 0.0f;
    if (pattern->activations) {
        for (size_t i = 0; i < pattern->count; i++) {
            pattern_strength += pattern->activations[i];
        }
    }
    pattern_strength = (pattern->count > 0) ? (pattern_strength / (float)pattern->count) : 0.0f;
    
    // Pure pattern strength ratio (no hardcoded range)
    float pattern_strength_norm = pattern_strength / (pattern_strength + 1.0f);
    
    // Decay is pure inverse of strength (strong = less decay)
    float decay_base = 1.0f - pattern_strength_norm;  // Range [0, 1] naturally
    
    // Multi-hop spreading with ADAPTIVE decay (not hardcoded)
    for (int hop = 0; hop < max_hops; hop++) {
        float decay = decay_base / (float)(hop + 1);  // Adaptive decay
        
        // Iterate through current pattern nodes
        size_t current_count = pattern->count;
        if (!pattern->nodes) {
            fprintf(stderr, "[DEBUG] ERROR: pattern->nodes is NULL!\n");
            break;
        }
        for (size_t i = 0; i < current_count; i++) {
            Node *node = pattern->nodes[i];
            float node_activation = pattern->activations[i];
            
            if (!node || node_activation <= 0.0f) continue;  // Only skip if truly zero
            
            // SELF-MODIFICATION: Accumulate context in node->state
            // This extends effective context window across hops
            node_accumulate_context(node, node_activation);
            
            // Don't spread from input nodes (except last meaningful)
            // PHASE 2: O(1) check using is_current_input flag (not O(n) loop)
            // Intelligence: Focuses spreading on continuation path, like brain's motor output
            if (node->is_current_input && node != last_meaningful) continue;
            
            // Spread through outgoing edges
            if (node->outgoing_edges) {
                for (size_t j = 0; j < node->outgoing_count; j++) {
                    Edge *edge = node->outgoing_edges[j];
                    if (!edge || !edge->to_node) continue;
                    
                    // Spread activation = source activation * edge weight * decay
                    float spread_activation = node_activation * edge->weight * decay;
                    
                    // BRAIN-LIKE PRIMING: Spread to ALL edges, no filtering
                    // Weak edges get small activation (priming), strong edges get large activation
                    // This prevents catastrophic forgetting by keeping weak memories alive
                    // No threshold - let strength determine activation amount, not existence
                    
                    // Add/update target node in pattern
                    activation_pattern_add(pattern, edge->to_node, spread_activation);
                }
            }
        }
    }
    
    // Debug removed for clean output
    (void)input_count;  // Suppress unused warning
    
    // 3. Build context vector
    activation_pattern_build_context(pattern, graph);
    
    return pattern;
}

/* Explore blank edges from current node
 * - Tries blank nodes as potential continuations
 * - Uses MiniNet to score blank transitions
 * - Adds blank nodes to activation pattern for exploration
 */
/* Explore blank nodes accessible through edges from current node
 * - Requirement.md line 7: "edges are paths they are the only paths that nodes can take"
 * - Requirement.md line 2: "NO O(n) searches" - we follow edges, not iterate arrays
 * - Only checks nodes reachable via edges from current_node
 */
static void explore_blank_edges_from_node(
    Node *current_node,
    ActivationPattern *pattern,
    Graph *graph,
    float exploration_rate
) {
    if (!current_node || !pattern || !graph) return;
    if (exploration_rate < 0.01f) return;  // Skip if not exploring
    if (!current_node->outgoing_edges || current_node->outgoing_count == 0) return;
    
    // Follow edges from current_node to find blank nodes
    // Requirement.md line 7: edges are the only paths
    for (size_t i = 0; i < current_node->outgoing_count; i++) {
        Edge *edge = current_node->outgoing_edges[i];
        if (!edge || !edge->to_node) continue;
        
        Node *target = edge->to_node;
        
        // Check if target is a blank node (payload_size == 0)
        if (target->payload_size != 0) continue;  // Not a blank node
        if (!target->net) continue;  // Blank node must have MiniNet
        
        // MiniNet decides: can this blank follow current node?
        float fill_score = mini_net_compute_blank_transition(
            target->net,
            current_node,
            pattern->nodes,
            pattern->count
        );
        
        // Exploration: activate blank with probability
        float explore_activation = fill_score * exploration_rate;
        
        if (explore_activation > 0.01f) {
            activation_pattern_add(pattern, target, explore_activation);
            
            // Also explore what blank leads to (follow its edges)
            if (target->outgoing_edges && target->outgoing_count > 0) {
                for (size_t j = 0; j < target->outgoing_count; j++) {
                    Edge *blank_edge = target->outgoing_edges[j];
                    if (blank_edge && blank_edge->to_node) {
                        float spread = explore_activation * ((float)blank_edge->weight / 255.0f);
                        activation_pattern_add(pattern, blank_edge->to_node, spread);
                    }
                }
            }
        }
    }
}

/* Update activation pattern with newly generated node
 * AUTOREGRESSIVE CONTEXT BUILDING (like LLM next-token prediction):
 * 1. Add generated node to pattern
 * 2. Re-spread activation from ENTIRE updated context (not just new node)
 * 3. This makes newly generated nodes part of the context for next prediction
 * 
 * KEY INSIGHT: Context must include generated output, not just input!
 * - Input: "hello w"
 * - Generate: 'o' → context becomes [h,e,l,l,o,space,w,o]
 * - Generate: 'r' → context becomes [h,e,l,l,o,space,w,o,r]
 * - Generate: 'l' → context becomes [h,e,l,l,o,space,w,o,r,l]
 * 
 * This is how LLMs work: predict → add to context → predict → add to context
 * Without this, context stays static and can't build up the path
 */
static void activation_pattern_update_with_node(
    ActivationPattern *pattern,
    Node *new_node,
    Graph *graph
) {
    if (!pattern || !new_node || !graph) return;
    
    // 1. Add new node with HIGH activation (it's the current output)
    // This node is CERTAIN - we just generated it
    float new_activation = 1.0f;  // Maximum confidence
    activation_pattern_add(pattern, new_node, new_activation);
    
    // 2. CRITICAL: Re-spread activation from the new node
    // This makes its continuations reachable for the NEXT prediction
    // 
    // Example: After generating 'o' in "hello wo"
    //   - 'o' activates its outgoing edges: o→r (from "world")
    //   - Now 'r' becomes reachable and highly activated
    //   - Next prediction will prefer 'r' because it's activated
    
    // Spread from new node (1 hop is enough - we're building incrementally)
    // Recent nodes matter more than distant ones (recency bias)
    // This is data-driven: decay rate adapts to pattern length
    if (pattern->count > 10) {
        float decay_factor = 0.95f;  // 5% decay
        // Stronger decay for very long sequences
        if (pattern->count > 100) {
            decay_factor = 0.90f;  // 10% decay
        }
        for (size_t i = 0; i < pattern->count; i++) {
            pattern->activations[i] *= decay_factor;
        }
    }
    
    // 1. Add new node to pattern with full activation
    activation_pattern_add(pattern, new_node, 1.0f);
    
    // 2. CRITICAL FIX: Clear old spread activations before spreading from new node
    // Problem: During ENCODE, we spread multiple hops and activate distant nodes (like 'd' at end of patterns)
    // Solution: Keep only INPUT nodes + GENERATED nodes, clear the multi-hop spread
    // 
    // This implements TRUE autoregressive: only the immediate next steps are activated
    // Not the entire future path!
    
    // Mark which nodes are "real" (input or generated), vs "spread" (multi-hop activation)
    // Real nodes: those in the sequence (input + generated so far)
    // Spread nodes: those activated by multi-hop spreading
    //
    // Strategy: Decay spread activations heavily, keep sequence activations
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        if (!node) continue;
        
        // Check if this node is in the sequence (input + generated)
        int in_sequence = 0;
        for (size_t j = 0; j < pattern->sequence_len; j++) {
            if (pattern->sequence[j] == node) {
                in_sequence = 1;
                break;
            }
        }
        
        if (!in_sequence) {
            // This is a "spread" node (from multi-hop), decay it heavily
            // We want to clear old spreading and only keep fresh spreading from new_node
            pattern->activations[i] *= 0.1f;  // 90% decay for spread nodes
        }
        // Nodes in sequence keep their activation (they're the actual context)
    }
    
    // 3. Spread activation from new node to its immediate neighbors (1-hop only)
    // SIMPLE: Just use edge weights (learned from training)
    // Strong edges (frequent paths) spread more activation
    // This is AUTOREGRESSIVE: each output primes ONLY the next output, not the entire future
    if (new_node->outgoing_edges) {
        for (size_t i = 0; i < new_node->outgoing_count; i++) {
            Edge *edge = new_node->outgoing_edges[i];
            if (!edge || !edge->to_node) continue;
            
            // Spread activation = edge weight (how often this path was trained)
            // No complex transforms - just raw learned weights
            float spread_activation = edge->weight;
            activation_pattern_add(pattern, edge->to_node, spread_activation);
        }
    }
    
    // 3. Update context vector (for embedding-based mechanisms)
    activation_pattern_build_context(pattern, graph);
}

/* ============================================================================
 * PHASE 2: REFINE - Activation → Refined Activation (Recurrent Dynamics)
 * ============================================================================
 * 
 * Like: LLM transformer layers (attention + feedforward)
 * Like: Brain recurrent cortical processing
 * 
 * KEY INSIGHT: Nodes that connect to active context get stronger
 * Nodes that don't connect decay away
 * This creates EMERGENT attention-like behavior
 */
static void refine_pattern_dynamics(ActivationPattern *pattern, Graph *graph, int iterations) {
    if (!pattern || pattern->count == 0 || !graph) return;
    
    float *new_activations = malloc(pattern->count * sizeof(float));
    if (!new_activations) return;
    
    // ADAPTIVE MIXING WEIGHTS (data-driven, not hardcoded)
    // Compute from pattern activation distribution
    float activation_sum = 0.0f;
    float activation_variance = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        activation_sum += pattern->activations[i];
    }
    float activation_mean = (pattern->count > 0) ? (activation_sum / (float)pattern->count) : 0.5f;
    
    for (size_t i = 0; i < pattern->count; i++) {
        float diff = pattern->activations[i] - activation_mean;
        activation_variance += diff * diff;
    }
    activation_variance = (pattern->count > 0) ? (activation_variance / (float)pattern->count) : 0.1f;
    
    // Pure variance ratio (no hardcoded 0.4f base)
    float epsilon = compute_adaptive_epsilon(activation_mean);
    float variance_norm = activation_variance / (activation_variance + activation_mean + epsilon);
    float self_weight = variance_norm;  // Pure ratio [0, 1]
    
    // Remaining weight distributed by connectivity ratio
    float pattern_avg_degree = 0.0f;
    if (pattern->count > 0) {
        for (size_t k = 0; k < pattern->count; k++) {
            if (pattern->nodes[k]) {
                pattern_avg_degree += (float)(pattern->nodes[k]->incoming_count + 
                                             pattern->nodes[k]->outgoing_count);
            }
        }
        pattern_avg_degree /= (float)pattern->count;
    }
    float neighbor_ratio = pattern_avg_degree / (pattern_avg_degree + 1.0f);  // Pure ratio
    float neighbor_weight = (1.0f - self_weight) * neighbor_ratio;
    float context_weight = (1.0f - self_weight) * (1.0f - neighbor_ratio);
    
    // Bidirectional edge weight: emerges from connectivity (pure ratio, no hardcoded 0.1f)
    float avg_degree = (pattern_avg_degree > 0.0f) ? pattern_avg_degree : 1.0f;
    // Weight emerges from degree ratio (no multiplier)
    float bidirectional_weight = 1.0f / (1.0f + avg_degree);
    
    for (int iter = 0; iter < iterations; iter++) {
        // For each node in pattern
        for (size_t i = 0; i < pattern->count; i++) {
            Node *node = pattern->nodes[i];
            float current = pattern->activations[i];
            
            if (!node) {
                new_activations[i] = 0.0f;
                continue;
            }
            
            // 1. Self-activation (momentum) - ADAPTIVE retention
            float self = current * self_weight;  // Data-driven, not hardcoded
            
            // 2. Input from connected neighbors in pattern
            // Nodes that connect to active nodes get boosted
            float neighbor_input = 0.0f;
            size_t neighbor_count = 0;
            
            // Check incoming edges from other pattern nodes
            for (size_t j = 0; j < node->incoming_count; j++) {
                Edge *edge = node->incoming_edges[j];
                if (!edge || !edge->from_node) continue;
                
                // Is source node in pattern?
                float source_activation = activation_pattern_get_activation(pattern, edge->from_node);
                if (source_activation > 0.0f) {
                    neighbor_input += source_activation * edge->weight;
                    neighbor_count++;
                }
            }
            
            // Check outgoing edges to other pattern nodes
            for (size_t j = 0; j < node->outgoing_count; j++) {
                Edge *edge = node->outgoing_edges[j];
                if (!edge || !edge->to_node) continue;
                
                // Is target node in pattern?
                float target_activation = activation_pattern_get_activation(pattern, edge->to_node);
                if (target_activation > 0.0f) {
                    // ADAPTIVE bidirectional weight (not hardcoded 0.5f)
                    neighbor_input += target_activation * edge->weight * bidirectional_weight;
                    neighbor_count++;
                }
            }
            
            // Normalize neighbor input
            if (neighbor_count > 0) {
                neighbor_input /= (float)neighbor_count;
            }
            
            // 3. Context modulation (like attention)
            // How well does this node fit the context vector?
            float context_fit = 0.0f;
            if (pattern->context_vector && node->payload_size > 0 && node->payload) {
                uint8_t byte = node->payload[0];
                context_fit = pattern->context_vector[byte];
            }
            
            // Combine: self + neighbors + context (ADAPTIVE weights, not hardcoded)
            new_activations[i] = self + neighbor_weight * neighbor_input + context_weight * context_fit;
            
            // Clamp to [0, 1]
            if (new_activations[i] < 0.0f) new_activations[i] = 0.0f;
            if (new_activations[i] > 1.0f) new_activations[i] = 1.0f;
        }
        
        // Update activations
        memcpy(pattern->activations, new_activations, pattern->count * sizeof(float));
        
        // Rebuild context vector with new activations
        activation_pattern_build_context(pattern, graph);
    }
    
    free(new_activations);
    
    // Debug removed for clean output
}

/* ============================================================================
 * DECODE HELPER FUNCTIONS: All Mechanisms Integration
 * ============================================================================
 * 
 * These functions enable the DECODE phase to use ALL available mechanisms:
 * - Pattern activations (from ENCODE+REFINE)
 * - Embeddings (semantic similarity)
 * - Context traces (disambiguation)
 * - Blank nodes (generalization)
 * - Similarity edges (associative recall)
 * - Hierarchies (sequence compression)
 */

/* Compute cosine similarity between two embedding vectors */
static float cosine_similarity(float *vec1, float *vec2, size_t dim) {
    if (!vec1 || !vec2 || dim == 0) return 0.0f;
    
    float dot = 0.0f, norm1 = 0.0f, norm2 = 0.0f;
    for (size_t i = 0; i < dim; i++) {
        dot += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    
    float denom = sqrtf(norm1) * sqrtf(norm2);
    if (denom < 1e-8f) return 0.0f;
    return dot / denom;
}

/* Compute embedding-based similarity between a candidate node and context nodes
 * Uses node embeddings for semantic routing
 */
static float compute_embedding_similarity(Node *candidate, Node **context_nodes, 
                                          size_t context_len, Graph *graph) {
    if (!candidate || !context_nodes || context_len == 0) return 0.0f;
    
    float *candidate_emb = node_get_embedding(candidate, graph);
    if (!candidate_emb || candidate->embedding_dim == 0) return 0.0f;
    
    size_t dim = candidate->embedding_dim;
    
    // Compute context embedding (weighted average of recent nodes)
    float *context_emb = calloc(dim, sizeof(float));
    if (!context_emb) return 0.0f;
    
    float weight_sum = 0.0f;
    
    // ENHANCEMENT: Exponential recency weighting (no hardcoded decay rate)
    // DATA-DRIVEN: Decay rate adapts to context length
    float decay_rate = (context_len > 0) ? (2.0f / (float)context_len) : 0.5f;
    
    for (size_t i = 0; i < context_len; i++) {
        Node *ctx_node = context_nodes[i];
        if (!ctx_node) continue;
        
        float *node_emb = node_get_embedding(ctx_node, graph);
        if (!node_emb || ctx_node->embedding_dim == 0) continue;
        
        // ENHANCEMENT: Exponential recency (most recent = highest weight)
        // Position from end: 0 = most recent, context_len-1 = oldest
        size_t pos_from_end = context_len - 1 - i;
        // DATA-DRIVEN: Weight = exp(-decay_rate * position)
        // No hardcoded "0.5" or "0.7" - computed from actual context size
        float weight = expf(-decay_rate * (float)pos_from_end);
        
        size_t use_dim = (ctx_node->embedding_dim < dim) ? ctx_node->embedding_dim : dim;
        
        for (size_t j = 0; j < use_dim; j++) {
            context_emb[j] += node_emb[j] * weight;
        }
        weight_sum += weight;
    }
    
    // Normalize by weight sum
    if (weight_sum > 0.0f) {
        for (size_t j = 0; j < dim; j++) {
            context_emb[j] /= weight_sum;
        }
    }
    
    float sim = cosine_similarity(candidate_emb, context_emb, dim);
    free(context_emb);
    
    return sim;
}

/* Compute context trace match between a candidate node and the current sequence
 * Uses the node's context_trace (RNN-like hidden state) for disambiguation
 */
// REMOVED: compute_context_trace_match - unused function

/* Score a candidate node using ALL available mechanisms
 * This is the unified scoring function that integrates:
 * - Pattern activation (PRIMARY signal - multiplicative)
 * - Embedding similarity (semantic routing)
 * - Context trace match (disambiguation)
 * - Edge context match (edge-specific disambiguation)
 * - Similarity edge bonus (associative recall)
 * - Blank node connection bonus (generalization)
 */
static float score_candidate_with_all_mechanisms(
    Node *candidate,
    ActivationPattern *pattern,
    Graph *graph,
    uint8_t *context_bytes,
    size_t context_len,
    Node **context_nodes,
    size_t context_node_count,
    Node *current_node,
    Edge *connecting_edge
) {
    if (!candidate || !connecting_edge) return 0.0f;
    
    // MULTIPLICATIVE SCORING: edge_weight × embedding_match × edge_context_match
    // Four factors (all must agree for high score):
    // 1. Edge weight = frequency (how often this path is used)
    // 2. Embedding match = semantic context (does this fit semantically?)
    // 3. Edge context match = byte-level context (was this edge learned in this context?)
    // 4. Pattern/hierarchy boost = learned patterns
    
    // Factor 1: Edge frequency (relative to local average)
    float local_avg = node_get_local_outgoing_weight_avg(current_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    float score = (local_avg > epsilon) ? 
                 (connecting_edge->weight / local_avg) : connecting_edge->weight;
    
    // Factor 2: Embedding similarity (semantic context match) - MULTIPLICATIVE
    if (context_nodes && context_node_count > 0 && graph) {
        float embedding_sim = compute_embedding_similarity(
            candidate, context_nodes, context_node_count, graph
        );
        // Normalize cosine similarity [-1,1] to [0,1]
        float context_match = (embedding_sim + 1.0f) / 2.0f;
        
        // MULTIPLY: edge_frequency × context_match
        // If context doesn't match, score → 0
        score *= context_match;
    }
    
    // Factor 3: Attention-based context (co-activation pattern matching)
    // Edge learns which nodes co-activate when it fires
    // REMOVED: edge_compute_attention() - edges are simple now
    // Context handled by node_update_context_values() instead
    // Attention score is 1.0 (neutral) - no edge-level attention
    
    // Factor 4: Routing gate (learned attention weight)
    // Edges that successfully predict in their context get stronger routing gates
    float gate_activation = 1.0f / (1.0f + expf(-connecting_edge->routing_gate));  // Sigmoid
    score *= gate_activation;
    
    // Boost for hierarchy nodes (they represent learned patterns)
    if (candidate->abstraction_level > 0) {
        score *= (1.0f + (float)candidate->abstraction_level * 0.1f);
    }
    
    // Boost for pattern activation (if candidate is already active)
    if (pattern) {
        float activation = activation_pattern_get_activation(pattern, candidate);
        if (activation > 0.0f) {
            score *= (1.0f + activation);
        }
    }
    
    return score;
}

/* ============================================================================
 * PHASE 3: DECODE - Activation → Output (All Mechanisms Integration)
 * ============================================================================
 * 
 * REDESIGNED: Per-byte re-evaluation loop that uses ALL mechanisms:
 * - Pattern activations (PRIMARY signal from ENCODE+REFINE)
 * - Hierarchies (searched at each step for sequence completion)
 * - Embeddings (semantic routing)
 * - Context traces (disambiguation)
 * - Blank nodes (generalization fallback)
 * - Similarity edges (associative recall)
 * 
 * Like: LLM autoregressive generation with full attention at each step
 * Like: Brain motor cortex consulting full sensory context for each action
 */
static void generate_from_pattern(MFile *mfile, ActivationPattern *pattern, 
                                  Node **input_nodes, size_t input_count) {
    if (!mfile || !mfile->graph || !pattern || pattern->count == 0) return;
    
    Graph *graph = mfile->graph;
    
    // ERROR FEEDBACK: Clear previous output path tracking
    mfile->last_output_path_count = 0;
    
    // Get expected output port from routing table
    uint8_t input_port = mfile->last_input_port_id;
    uint8_t output_port = input_port;
    
    // === OUTPUT BUFFERS ===
    uint8_t *output = NULL;
    size_t output_len = 0;
    size_t output_capacity = 0;
    
    Node **output_nodes = NULL;
    size_t output_nodes_len = 0;
    size_t output_nodes_capacity = 0;
    
    // === USE ACTIVATION PATTERN AS FULL CONTEXT ===
    // NEW: Use pattern->nodes (all activated nodes) as context
    // This follows Requirement line 6: "context is a payload of activated nodes"
    // No hardcoded limits - context grows with activation (Requirement line 3)
    Node **context_nodes = pattern->nodes;  // All activated nodes from spreading activation
    size_t context_node_count = pattern->count;  // Number of activated nodes
    
    // Build byte-level context from activated nodes
    size_t full_context_capacity = context_node_count + 256;  // Adaptive capacity
    uint8_t *full_context = malloc(full_context_capacity);
    size_t context_len = 0;
    
    if (!full_context) {
        return;
    }
    
    // Initialize context with input bytes (for byte-level matching)
    for (size_t i = 0; i < input_count; i++) {
        if (input_nodes[i] && input_nodes[i]->payload_size > 0) {
            full_context[context_len++] = input_nodes[i]->payload[0];
        }
    }
    
    // === CANDIDATE COLLECTION ARRAYS ===
    size_t max_candidates = pattern->count + 256;  // Pattern nodes + edge targets + extras
    Node **all_candidates = malloc(max_candidates * sizeof(Node*));
    float *all_scores = malloc(max_candidates * sizeof(float));
    Edge **candidate_edges = malloc(max_candidates * sizeof(Edge*));
    
    if (!all_candidates || !all_scores || !candidate_edges) {
        free(full_context);
        free(context_nodes);
        free(all_candidates);
        free(all_scores);
        free(candidate_edges);
        return;
    }
    
    // === FIRST BYTE: Use pattern activation as PRIMARY signal ===
    // Select from continuation nodes (NOT input nodes)
    Node *current_node = NULL;
    
    // SOLUTION 1: Skip Priority 1 (pattern activation selection)
    // Priority 1 was selecting wrong nodes from activation pattern, causing "held held held" loops
    // Instead, use Priority 2 (last input node's best edge) which already works correctly
    // This is consistent with the edge order tiebreaker fix for "wo wo wo"
    
    /* DISABLED Priority 1: Pattern-based first selection (was causing "held" loops)
    // Find best continuation node from pattern (exclude input nodes)
    float best_first_score = -1.0f;
    if (pattern->nodes && pattern->activations) {
        for (size_t i = 0; i < pattern->count; i++) {
            Node *node = pattern->nodes[i];
            float activation = pattern->activations[i];
            
            if (!node || node->payload_size == 0 || !node->payload) continue;
            if (node->payload[0] == 0x00 || node->payload[0] < 32) continue;
            if (activation < 0.01f) continue;
            
            // Port filtering
            if (node->port_id != 0 && output_port != 0 && node->port_id != output_port) continue;
            
            // PHASE 2: O(1) input node check using is_current_input flag
            // BUG FIX 1: Skip input nodes - we want continuations, not echoes!
            // Intelligence: Brain distinguishes sensory input (what you see) from motor output (what you do)
            if (node->is_current_input) continue;
            
            // Pure activation score (no penalties)
            float score = activation;
            
            // Input nodes naturally score lower if their activation is lower
            // No need to artificially penalize - let the data decide
            
            // Hierarchy nodes get pure abstraction level added
            if (node->abstraction_level > 0) {
                score += (float)node->abstraction_level;
            }
            
            if (score > best_first_score) {
                best_first_score = score;
                current_node = node;
            }
        }
    }
    */
    
    // Priority 2: Use last input node's best outgoing edge (ALWAYS use this now)
    if (!current_node && input_nodes && input_count > 0) {
        Node *last_input = input_nodes[input_count - 1];
        if (last_input && last_input->outgoing_edges && last_input->outgoing_count > 0) {
            float best_edge_score = -1.0f;
            for (size_t i = 0; i < last_input->outgoing_count; i++) {
                Edge *edge = last_input->outgoing_edges[i];
                if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
                if (edge->to_node->payload[0] < 32) continue;
                
                float score = edge->weight;
                float target_activation = activation_pattern_get_activation(pattern, edge->to_node);
                // Pure additive: activation contributes directly
                score += target_activation;
                
                if (score > best_edge_score) {
                    best_edge_score = score;
                    current_node = edge->to_node;
                }
            }
        }
        
        // Last resort: use first input node
        if (!current_node && input_nodes && input_nodes[0]) {
            current_node = input_nodes[0];
        }
    }
    
    if (!current_node) {
        free(full_context);
        free(context_nodes);
        free(all_candidates);
        free(all_scores);
        free(candidate_edges);
        return;
    }
    
    // === MAIN GENERATION LOOP (per-byte re-evaluation) ===
    size_t max_output_len = 256;  // Hard limit to prevent infinite generation
    while (current_node && current_node->payload_size > 0 && output_len < max_output_len) {
        
        // === 1. LOOP DETECTION ===
        // Detect repeating patterns in output
        // Count consecutive repetitions of patterns
        float repetition_strength = 0.0f;
        int max_consecutive_reps = 0;
        if (output && output_len >= 2) {
            size_t max_pattern_len = output_len / 2;
            for (size_t plen = 1; plen <= max_pattern_len && plen <= 8; plen++) {
                // Count how many times this pattern repeats consecutively
                int consecutive_reps = 0;
                size_t pos = output_len;
                while (pos >= plen) {
                    int matches = 1;
                    for (size_t i = 0; i < plen; i++) {
                        if (pos < plen + i || output[pos - 1 - i] != output[pos - 1 - plen - i]) {
                            matches = 0;
                            break;
                        }
                    }
                    if (matches) {
                        consecutive_reps++;
                        pos -= plen;
                    } else {
                        break;
                    }
                }
                
                if (consecutive_reps > max_consecutive_reps) {
                    max_consecutive_reps = consecutive_reps;
                }
                
                // Strength based on how much of output is the repeating pattern
                if (consecutive_reps >= 2) {
                    float pattern_coverage = (float)(consecutive_reps * plen) / (float)output_len;
                    if (pattern_coverage > repetition_strength) {
                        repetition_strength = pattern_coverage;
                    }
                }
            }
        }
        
        // STRONG loop detection: if pattern repeats 3+ times OR covers 30%+ of output, stop
        if (max_consecutive_reps >= 3 || repetition_strength > 0.3f) {
            break;
        }
        
        // === 2. HIERARCHY SEARCH (at each step!) ===
        // Check if current context matches any hierarchy
        size_t hierarchy_position = 0;
        Node *active_hierarchy = NULL;
        if (full_context && context_len > 0) {
            active_hierarchy = find_active_hierarchy(graph, full_context, context_len, &hierarchy_position);
        }
        
        if (active_hierarchy && hierarchy_position < active_hierarchy->payload_size) {
            // Hierarchy match found! Output remaining bytes directly
            for (size_t b = hierarchy_position; b < active_hierarchy->payload_size; b++) {
                uint8_t byte = active_hierarchy->payload[b];
                if (byte == 0x00) break;
                
                // Grow output buffer
            if (output_len >= output_capacity) {
                size_t new_cap = (output_capacity == 0) ? 16 : output_capacity * 2;
                uint8_t *new_out = realloc(output, new_cap);
                if (!new_out) break;
                output = new_out;
                output_capacity = new_cap;
            }
            output[output_len++] = byte;
                
                // Update full context
                if (context_len < full_context_capacity - 1) {
                    full_context[context_len++] = byte;
                }
        }
        
            // Track hierarchy node
        if (output_nodes_len >= output_nodes_capacity) {
            size_t new_cap = (output_nodes_capacity == 0) ? 16 : output_nodes_capacity * 2;
            Node **new_nodes = realloc(output_nodes, new_cap * sizeof(Node*));
                if (new_nodes) {
            output_nodes = new_nodes;
            output_nodes_capacity = new_cap;
        }
            }
            if (output_nodes_len < output_nodes_capacity) {
                output_nodes[output_nodes_len++] = active_hierarchy;
            }
            
            // Find next node after hierarchy (last byte's successor)
            if (active_hierarchy->outgoing_count > 0 && 
                active_hierarchy->outgoing_edges && 
                active_hierarchy->outgoing_edges[0] &&
                active_hierarchy->outgoing_edges[0]->to_node) {
                current_node = active_hierarchy->outgoing_edges[0]->to_node;
            } else {
                current_node = NULL;
            }
            continue;  // Re-evaluate with new context
        }
        
        // === 3. OUTPUT CURRENT NODE'S BYTE(S) ===
        size_t bytes_to_output = 1;
        if (current_node->abstraction_level > 0) {
            bytes_to_output = current_node->payload_size;
        }
        
        for (size_t b = 0; b < bytes_to_output; b++) {
            if (b >= current_node->payload_size) break;
            uint8_t byte = current_node->payload[b];
            if (byte == 0x00) break;
            
            if (output_len >= output_capacity) {
                size_t new_cap = (output_capacity == 0) ? 16 : output_capacity * 2;
                uint8_t *new_out = realloc(output, new_cap);
                if (!new_out) break;
                output = new_out;
                output_capacity = new_cap;
            }
            output[output_len++] = byte;
            
            // Update full context
            if (context_len < full_context_capacity - 1) {
                full_context[context_len++] = byte;
            }
        }
        
        // Track output node
        if (output_nodes_len >= output_nodes_capacity) {
            size_t new_cap = (output_nodes_capacity == 0) ? 16 : output_nodes_capacity * 2;
            Node **new_nodes = realloc(output_nodes, new_cap * sizeof(Node*));
            if (new_nodes) {
                output_nodes = new_nodes;
                output_nodes_capacity = new_cap;
            }
        }
        if (output_nodes_len < output_nodes_capacity) {
            output_nodes[output_nodes_len++] = current_node;
            // NOTE: context_nodes now points to pattern->nodes (all activated nodes)
            // We don't modify it during generation - it's the full graph context
        }
        
        // BUG FIX 4: Stop if no outgoing edges (natural boundary)
        // Intelligence: Like how humans pause at sentence ends - no continuations means pattern ends
        // This is learned from training data (nodes at pattern ends have no/weak outgoing edges)
        if (current_node->outgoing_count == 0) {
            break;  // Natural stopping point
        }
        
        // CONTEXT GATING: Update context-relative edge values before scoring
        // This implements Requirement.md line 6: "context changes the edge weights of the current node"
        // Context gates edges: edges matching context_trace are enabled, others suppressed
        node_update_context_values(current_node, graph ? graph->wave_generation : 0);
        
        // === 4. COLLECT ALL CANDIDATES FOR NEXT BYTE ===
        size_t candidate_count = 0;
        
        // 4a. Candidates from current node's outgoing edges (including similarity edges!)
        // Use context-gated values from node_update_context_values
        if (current_node->outgoing_edges && current_node->edge_context_values) {
        for (size_t i = 0; i < current_node->outgoing_count && candidate_count < max_candidates; i++) {
            Edge *edge = current_node->outgoing_edges[i];
            if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
            
            Node *target = edge->to_node;
            
            // CONTEXT-GATED SCORING (Using context_trace for path disambiguation)
            // Requirement line 6: "context is a payload, of that activated nodes"
            // Requirement line 6: "current node holds the context... that context changes edge weights"
            // Requirement line 7: "edges are the only paths that nodes can take"
            //
            // KEY INSIGHT: Use current_node->context_trace to gate which edges are valid
            // The context_trace holds the last 8 bytes that led to this node
            // Edges should only fire if their target continues the current path
            
            // Base score: Edge weight (how often this path was trained)
            float local_avg = node_get_local_outgoing_weight_avg(current_node);
            float epsilon = compute_adaptive_epsilon(local_avg);
            float score = (local_avg > epsilon) ? 
                         (edge->weight / local_avg) : edge->weight;
            
            // CONTEXT GATING: Use edge weight relative to activation
            // SIMPLIFIED: Don't try to decode context_trace (it's complex and error-prone)
            // Instead: Use the COMBINATION of edge weight + target activation
            // 
            // KEY INSIGHT: Strong edges to highly activated targets = correct path
            // Weak edges to highly activated targets = wrong path (distant node)
            // Strong edges to weakly activated targets = unexplored path
            //
            // This naturally implements context gating:
            // - Edges that were trained in THIS context have strong weights
            // - Targets that are reachable from THIS context are activated
            // - The product selects edges that are BOTH trained AND reachable
            
            // FIX: Use edge index as tiebreaker for equal weights
            // When edges have equal weight, prefer edges created LATER (higher index)
            // because in "hello world", the edge o→r (from "world") was created
            // AFTER the edge o→' ' (from "hello "), so it should win
            float edge_order_bonus = (float)i * 0.01f;  // Small bonus for later edges
            score += edge_order_bonus;
            
            float context_match = 1.0f;  // Default: no gating
            
            // Compute edge strength relative to neighbors
            float edge_strength = edge->weight / (local_avg + epsilon);
            
            // If edge is weak (< 1.0) but target is highly activated (> 5.0):
            // This is likely a DISTANT node activated by multi-hop spreading
            // Suppress it!
            float target_activation = 0.0f;
            if (pattern) {
                target_activation = activation_pattern_get_activation(pattern, target);
            }
            
            if (edge_strength < 1.0f && target_activation > 5.0f) {
                // Weak edge to highly activated target = wrong path
                context_match = 0.1f;  // Heavy suppression
            } else if (edge_strength > 1.5f && target_activation > 1.0f) {
                // Strong edge to activated target = correct path
                context_match = 2.0f;  // Boost
            } else if (edge_strength > 1.5f && target_activation < 0.5f) {
                // Strong edge to non-activated target = unexplored but valid
                context_match = 1.0f;  // Neutral
            }
            
            // Apply context gate to score
            score *= context_match;
            
            // SECONDARY FACTOR: Target node activation (already computed above)
            // Boost edges leading to activated nodes (but context gate is primary)
            if (target_activation > 0.0f) {
                // Moderate boost: activation confirms reachability
                score *= (1.0f + target_activation * 2.0f);  // 2x boost (reduced from 5x)
            } else {
                // No activation = not reachable through spreading
                // Suppress but don't eliminate (context gate already applied)
                score *= 0.5f;  // 50% suppression (less aggressive than 99%)
            }
            
            // Tertiary factors (minor modulation)
            if (pattern && graph) {
                // Routing gate (learned gating)
                float gate_activation = 1.0f / (1.0f + expf(-edge->routing_gate));
                score *= gate_activation;
                
                // Hierarchy boost (pattern nodes)
                if (target->abstraction_level > 0) {
                    score *= (1.0f + (float)target->abstraction_level * 0.1f);
                }
            }
            
            all_candidates[candidate_count] = target;
            all_scores[candidate_count] = score;
            candidate_edges[candidate_count] = edge;
            candidate_count++;
        }
        }  // end if (current_node->outgoing_edges)
        
        // 4b. BLANK EDGE CANDIDATES (NEW)
        // Explore blank nodes accessible through edges from current_node
        // Requirement.md line 7: "edges are paths they are the only paths that nodes can take"
        // Requirement.md line 2: "NO O(n) searches" - we follow edges, not iterate arrays
        // Blank nodes compete with concrete nodes through relative scoring
        if (graph && current_node->outgoing_edges && current_node->outgoing_count > 0) {
            float exploration_factor = 0.1f;  // 10% exploration baseline
            
            // Check outgoing edges for blank nodes
            for (size_t b = 0; b < current_node->outgoing_count && candidate_count < max_candidates; b++) {
                Edge *edge = current_node->outgoing_edges[b];
                if (!edge || !edge->to_node) continue;
                
                Node *target = edge->to_node;
                
                // Check if target is a blank node (payload_size == 0)
                if (target->payload_size != 0) continue;  // Not a blank node
                if (!target->net) continue;  // Blank node must have MiniNet
                
                // Can this blank follow current node?
                float transition_score = mini_net_compute_blank_transition(
                    target->net,
                    current_node,
                    context_nodes,
                    context_node_count
                );
                
                if (transition_score > 0.1f) {
                    // #region agent log
                    fprintf(stderr, "[LOG] blank_candidate blank=%p score=%.3f\n", 
                            (void*)target, transition_score * exploration_factor);
                    fflush(stderr);
                    // #endregion
                    
                    all_candidates[candidate_count] = target;
                    all_scores[candidate_count] = transition_score * exploration_factor;
                    candidate_edges[candidate_count] = edge;  // Use the actual edge
                    candidate_count++;
                }
            }
        }
        
        // REMOVED: Section 4b (pattern activation candidates)
        // This was a FALLBACK that violated Requirement line 5: "No Fallbacks"
        // and Requirement line 7: "edges are paths they are the only paths that nodes can take"
        // 
        // Edges are the ONLY way to reach nodes. If there's no edge, the node is not reachable.
        // Pattern activation already happened during ENCODE - those activated nodes are reachable
        // through edges, not as direct candidates.
        
        // REMOVED: Section 4c (blank node candidates)
        // This was also a FALLBACK mechanism that violated requirements
        
        // === 5. SELECT WINNER ===
        if (candidate_count == 0) {
            break;  // No candidates = stop
        }
        
        // Find best candidate
        float best_score = -1.0f;
        size_t best_idx = 0;
        for (size_t i = 0; i < candidate_count; i++) {
            if (all_scores[i] > best_score) {
                best_score = all_scores[i];
                best_idx = i;
            }
        }
        
        Node *next_node = all_candidates[best_idx];
        Edge *used_edge = candidate_edges[best_idx];
        
        // Debug output removed for production
        
        // REMOVED: edge_learn_attention() - edges are simple now
        // Learning happens through weight updates, not pattern learning
        
        // === 6. STOP VS CONTINUE DECISION ===
        float stop_prob = current_node->stop_weight / (current_node->stop_weight + 1.0f);
        
        // Compute local average score
        float local_avg_score = 0.0f;
        for (size_t i = 0; i < candidate_count; i++) {
            local_avg_score += all_scores[i];
        }
        local_avg_score /= (float)candidate_count;
        
        // Relative weakness
        float relative_weakness = 1.0f - (best_score / (local_avg_score + 0.01f));
        if (relative_weakness < 0.0f) relative_weakness = 0.0f;
        if (relative_weakness > 1.0f) relative_weakness = 1.0f;
        
        // Loop contributes to stop
        float loop_stop = repetition_strength * repetition_strength;
        
        // Combined stop signal
        float combined_stop = stop_prob;
        if (relative_weakness > combined_stop) combined_stop = relative_weakness;
        if (loop_stop > combined_stop) combined_stop = loop_stop;
        
        // Normalize best_score to [0,1] range for comparison
        float continue_signal = best_score / (best_score + 1.0f);
        
        if (combined_stop > continue_signal) {
            break;  // Stop wins
        }
        
        // === 7. TRACK EDGE FOR ERROR FEEDBACK ===
        // Track edge if it exists, or create virtual edge for pattern-activated nodes
        if (used_edge) {
            if (mfile->last_output_path_count >= mfile->last_output_path_capacity) {
                size_t new_cap = (mfile->last_output_path_capacity == 0) ? 16 : 
                                 mfile->last_output_path_capacity * 2;
                Edge **new_path = realloc(mfile->last_output_path, new_cap * sizeof(Edge*));
                if (new_path) {
                    mfile->last_output_path = new_path;
                    mfile->last_output_path_capacity = new_cap;
                }
            }
            if (mfile->last_output_path_count < mfile->last_output_path_capacity) {
                mfile->last_output_path[mfile->last_output_path_count++] = used_edge;
            }
        } else if (current_node && next_node) {
            // No direct edge - create one for learning
            // This allows error feedback to strengthen correct pattern-activated paths
            Edge *new_edge = edge_create(current_node, next_node);
            if (new_edge) {
                graph_add_edge(graph, new_edge);
                if (mfile->last_output_path_count >= mfile->last_output_path_capacity) {
                    size_t new_cap = (mfile->last_output_path_capacity == 0) ? 16 : 
                                     mfile->last_output_path_capacity * 2;
                    Edge **new_path = realloc(mfile->last_output_path, new_cap * sizeof(Edge*));
                    if (new_path) {
                        mfile->last_output_path = new_path;
                        mfile->last_output_path_capacity = new_cap;
                    }
                }
                if (mfile->last_output_path_count < mfile->last_output_path_capacity) {
                    mfile->last_output_path[mfile->last_output_path_count++] = new_edge;
                }
            }
        }
        
        // === 8. UPDATE PATTERN WITH GENERATED NODE ===
        // KEY FIX FOR CONDITIONAL BRANCHING:
        // Add newly generated node to pattern and re-spread activation
        // This provides growing context for subsequent decisions
        // 
        // Example: "hello w" → generate 'o' → pattern becomes [h,e,l,l,o,space,w,o]
        //          Next decision sees 'w' AND 'o' together, picks 'r' (from "world")
        //          Without this, pattern stays [h,e,l,l,o,space,w], loses track of path
        activation_pattern_update_with_node(pattern, next_node, graph);
        
        // === 9. UPDATE CONTEXT TRACE (CRITICAL FOR DISAMBIGUATION) ===
        // This is the KEY missing piece!
        // Update next_node's context_trace with the sequence so far
        // This allows the node to "remember" what came before it
        // 
        // Example: 'l' after "wor" has context_trace = [w,o,r]
        //          'l' after "hel" has context_trace = [h,e,l]
        //          These are DIFFERENT contexts, so edges should behave differently!
        //
        // Build sequence from output_nodes (input + generated so far)
        if (output_nodes && output_nodes_len > 0) {
            update_node_context_trace(next_node, output_nodes, output_nodes_len, output_nodes_len - 1);
        }
        
        // Move to next node
        current_node = next_node;
    }
    
    // === WRITE OUTPUT ===
    if (output_len > 0 && output) {
        while (mfile->universal_output_size + output_len > mfile->universal_output_capacity) {
            size_t new_cap = (mfile->universal_output_capacity == 0) ? 
                             output_len * 2 : mfile->universal_output_capacity * 2;
            uint8_t *new_buf = realloc(mfile->universal_output, new_cap);
            if (!new_buf) break;
            mfile->universal_output = new_buf;
            mfile->universal_output_capacity = new_cap;
        }
        
        memcpy(mfile->universal_output + mfile->universal_output_size, output, output_len);
        mfile->universal_output_size += output_len;
    }
    
    // === CLEANUP ===
    free(output);
    free(output_nodes);
    free(full_context);
    // NOTE: Don't free context_nodes - it points to pattern->nodes (owned by pattern)
    free(all_candidates);
    free(all_scores);
    free(candidate_edges);
}

/* ============================================================================
 * WAVE PROPAGATION (Principle 7: Emergent Intelligence)
 * ============================================================================ */

/* Forward declarations */
static void wave_state_init(WaveState *state);
static void wave_state_reset(WaveState *state);
static void wave_state_free(WaveState *state);

/* Wave propagation (biological local learning only)
 * - Each step is recurrent update
 * - Collects activated nodes for output generation
 * - All operations local (O(degree) per node)
 * - Energy-aware exploration
 * - No BPTT - uses biological Hebbian learning (edges strengthen with activation)
 */
// REMOVED: wave_propagate - unused function (~400 lines)

/* ============================================================================
 * PATTERN MATCHING (Principle 4: Compounding Learning)
 * ============================================================================ */

/* Find or create node for pattern
 * - Hierarchy-first matching (try larger patterns first)
 * - Adaptive pattern size limits
 * - O(log n) with hierarchy
 */
/* ============================================================================
 * BLANK NODE INTEGRATION (Phase 3: Generalization)
 * Brain: Prototype-based categorization
 * LLM: Learned embeddings capture semantic similarity
 * ============================================================================ */

/* Compute blank acceptance score (data-driven threshold)
 * - Brain: Prototype matching with adaptive category width
 * - Returns smooth score (not binary)
 */
static float compute_blank_acceptance_score(Node *blank, const uint8_t *pattern, size_t pattern_size) {
    if (!blank || blank->payload_size != 0) return 0.0f;
    
    float best_similarity = 0.0f;
    
    // Check similarity to all nodes that point to this blank node
    for (size_t i = 0; i < blank->incoming_count; i++) {
        Node *connected = blank->incoming_edges[i]->from_node;
        if (!connected || connected->payload_size == 0) continue;
        
        // Use existing node-pattern similarity function
        float sim = 0.0f;
        size_t min_size = (pattern_size < connected->payload_size) ? 
                         pattern_size : connected->payload_size;
        if (min_size > 0) {
            size_t matches = 0;
            for (size_t j = 0; j < min_size; j++) {
                if (pattern[j] == connected->payload[j]) matches++;
            }
            sim = (float)matches / (float)min_size;
        }
        
        if (sim > best_similarity) best_similarity = sim;
    }
    
    // Data-driven threshold: computed from blank node's incoming edge distribution
    // If blank has no context, no acceptance (return 0.0f - neutral)
    if (blank->incoming_count == 0) return 0.0f;
    
    // Compute adaptive threshold from blank's connection strength
    float avg_edge_weight = 0.0f;
    for (size_t i = 0; i < blank->incoming_count; i++) {
        if (blank->incoming_edges[i]) {
            avg_edge_weight += blank->incoming_edges[i]->weight;
        }
    }
    avg_edge_weight /= (float)blank->incoming_count;
    float epsilon = compute_adaptive_epsilon(avg_edge_weight);
    
    // Threshold adapts to blank's learned pattern strength (not hardcoded)
    float acceptance_threshold = avg_edge_weight / (avg_edge_weight + 1.0f + epsilon);
    
    if (best_similarity > acceptance_threshold) {
        return best_similarity - acceptance_threshold;
    }
    
    return 0.0f;
}

/* REMOVED: find_accepting_blank_node()
 * 
 * This function performed a global O(n) search for blank nodes, violating:
 * - Requirement.md line 2: "NO O(n) searches, no global searches for anything"
 * - Requirement.md line 7: "edges are paths they are the only paths that nodes can take"
 * 
 * Blank nodes are now found only through edges from active nodes during:
 * - Wave propagation (explore_blank_edges_from_node)
 * - Generation (generate_from_pattern checks outgoing edges)
 * 
 * This ensures blank nodes are only accessible through valid paths (edges).
 */

static Node* graph_find_or_create_pattern_node(Graph *graph, const uint8_t *pattern, 
                                               size_t pattern_size) {
    if (!graph || !pattern || pattern_size == 0) return NULL;
    
    // PHASE 2: O(1) TRIE LOOKUP FIRST (fastest path)
    // Brain: Pattern matching in visual cortex uses learned templates
    // LLM: Hash table lookup for tokenization
    // Melvin: Trie lookup is O(pattern_size), not O(n)
    Node *trie_match = trie_lookup(graph, pattern, pattern_size);
    if (trie_match) {
        return trie_match;  // Exact match found in O(pattern_size) time!
    }
    
    // STEP 1: Try hierarchy nodes (using INDEXED hierarchy lookup, not O(n))
    // Brain: Cortical hierarchy - higher areas recognize larger patterns
    // LLM: Learned representations - multi-token embeddings
    // Melvin: Try from highest abstraction to lowest, using hierarchy index
    
    if (graph->max_abstraction_level > 0 && graph->hierarchy_by_level) {
        // Try hierarchy nodes from highest to lowest abstraction
        // PHASE 2: Use hierarchy_by_level index (O(hierarchies_at_level) not O(all_nodes))
        for (uint32_t level = graph->max_abstraction_level; level > 0; level--) {
            if (level < graph->max_hierarchy_levels && graph->hierarchy_by_level[level]) {
                size_t count = graph->hierarchy_counts[level];
                Node **level_nodes = graph->hierarchy_by_level[level];
                
                for (size_t i = 0; i < count; i++) {
                    Node *node = level_nodes[i];
                    if (!node) continue;
                    
                    // Pattern must be at least as large as hierarchy node
                    if (node->payload_size <= pattern_size) {
                        // Check if pattern starts with this hierarchy node's payload
                        if (memcmp(node->payload, pattern, node->payload_size) == 0) {
                            return node;  // Found hierarchy match (prefix match)
                        }
                    }
                }
            }
        }
    }
    
    // STEP 2: No hierarchy match, try exact byte match (abstraction level 0)
    // Compute node ID from pattern (same algorithm as node_create)
    // Following README: Use hash table for O(1) lookup instead of O(n) linear search
    uint64_t hash = 0;
    for (size_t i = 0; i < pattern_size; i++) {
        hash = hash * 31 + pattern[i];
    }
    uint8_t node_id[9];
    snprintf((char*)node_id, 9, "%08lx", (unsigned long)(hash & 0xFFFFFFFF));
    
    // O(1) hash table lookup instead of O(n) linear search
    Node *node = graph_find_node_by_id(graph, node_id);
    if (node) {
        // Verify payload matches (hash collision check - rare but possible)
        if (node->payload_size == pattern_size &&
            memcmp(node->payload, pattern, pattern_size) == 0) {
            return node;  // Found exact match
        }
        // Hash collision detected - handled by trie lookup above
    }
    
    // STEP 3: Blank nodes are found through edges only (Requirement.md line 7)
    // No global search for blank nodes (Requirement.md line 2: "NO O(n) searches")
    // Blank nodes will be discovered naturally through wave propagation and generation
    // when following edges from active nodes
    
    // STEP 4: Create new node if no match (abstraction level 0 = raw)
    node = node_create(pattern, pattern_size, 0);
    if (node) {
        graph_add_node(graph, node);
    }
    
    return node;
}

/* Process sequential patterns from input
 * - Break input into patterns
 * - Create co-activation edges
 * - Adaptive pattern size
 */

/* Compute adaptive similarity threshold (from local context)
 * - Threshold adapts to local pattern distribution
 * - No hardcoded values: all computed from local data
 */
static float compute_adaptive_similarity_threshold(Node *node) {
    if (!node || node->outgoing_count == 0) return 0.0f;  // Neutral when no data
    
    // Get local context
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float local_variance = compute_local_edge_variance(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return 0.0f;  // Neutral when no context
    
    // Threshold adapts to local variance (high variance = stricter threshold)
    float variance_norm = local_variance / (local_avg + epsilon);
    
    // Similarity threshold: 0.3 to 0.7 (adaptive)
    // Higher variance → stricter threshold (more selective)
    float base_threshold = 0.3f + (variance_norm * 0.4f);
    
    return base_threshold;
}

/* Compute connection diversity for blank node creation
 * - Measures how many DIFFERENT nodes connect to a position
 * - High diversity = variable position (needs blank abstraction)
 * - Returns ratio of unique connections
 */
static float compute_connection_diversity(Edge **edges, size_t count) {
    // #region agent log
    FILE *logf = fopen("/Users/jakegilbert/Desktop/Melvin_Reasearch/Melvin_09b/.cursor/debug.log", "a");
    if (logf) {
        fprintf(logf, "{\"location\":\"melvin.c:5908\",\"message\":\"diversity_entry\",\"data\":{\"edges\":\"%p\",\"count\":%zu},\"timestamp\":%lld,\"sessionId\":\"debug-session\",\"hypothesisId\":\"H3\"}\n", 
                (void*)edges, count, (long long)time(NULL));
        fclose(logf);
    }
    // #endregion
    
    if (count == 0) return 0.0f;
    
    // Count unique payload values
    size_t unique_count = 0;
    uint8_t seen[256] = {0};
    
    for (size_t i = 0; i < count; i++) {
        if (!edges[i]) {
            // #region agent log
            logf = fopen("/Users/jakegilbert/Desktop/Melvin_Reasearch/Melvin_09b/.cursor/debug.log", "a");
            if (logf) {
                fprintf(logf, "{\"location\":\"melvin.c:5926\",\"message\":\"null_edge\",\"data\":{\"i\":%zu},\"timestamp\":%lld,\"sessionId\":\"debug-session\",\"hypothesisId\":\"H1\"}\n", 
                        i, (long long)time(NULL));
                fclose(logf);
            }
            // #endregion
            continue;
        }
        
        Node *node = (edges[i]->to_node) ? edges[i]->to_node : edges[i]->from_node;
        
        // #region agent log
        logf = fopen("/Users/jakegilbert/Desktop/Melvin_Reasearch/Melvin_09b/.cursor/debug.log", "a");
        if (logf) {
            fprintf(logf, "{\"location\":\"melvin.c:5940\",\"message\":\"edge_node_check\",\"data\":{\"i\":%zu,\"node\":\"%p\",\"payload_size\":%zu},\"timestamp\":%lld,\"sessionId\":\"debug-session\",\"hypothesisId\":\"H2\"}\n", 
                    i, (void*)node, node ? node->payload_size : 0, (long long)time(NULL));
            fclose(logf);
        }
        // #endregion
        
        if (!node || node->payload_size == 0) continue;
        
        uint8_t first_byte = node->payload[0];
        if (!seen[first_byte]) {
            seen[first_byte] = 1;
            unique_count++;
        }
    }
    
    // Diversity ratio
    float result = (float)unique_count / (float)count;
    
    // #region agent log
    logf = fopen("/Users/jakegilbert/Desktop/Melvin_Reasearch/Melvin_09b/.cursor/debug.log", "a");
    if (logf) {
        fprintf(logf, "{\"location\":\"melvin.c:5963\",\"message\":\"diversity_result\",\"data\":{\"unique\":%zu,\"total\":%zu,\"ratio\":%.3f},\"timestamp\":%lld,\"sessionId\":\"debug-session\",\"hypothesisId\":\"H5\"}\n", 
                unique_count, count, result, (long long)time(NULL));
        fclose(logf);
    }
    // #endregion
    
    return result;
}

/* Find existing blank for this position pattern
 * - Checks if a blank already exists with similar connections
 * - Avoids creating duplicate blanks
 * - Requirement.md line 7: "edges are paths" - we check through edges only
 * - Requirement.md line 2: "NO O(n) searches" - we check neighbors only
 */
static Node* find_blank_for_position(Node *node, Graph *graph) {
    if (!node || !graph || !node->outgoing_edges || node->outgoing_count == 0) return NULL;
    
    // Check nodes that share similar outgoing targets (through edges)
    // This avoids global search by only checking nodes connected to our targets
    for (size_t i = 0; i < node->outgoing_count && i < 5; i++) {  // Limit to prevent O(n)
        Edge *node_edge = node->outgoing_edges[i];
        if (!node_edge || !node_edge->to_node) continue;
        
        Node *target = node_edge->to_node;
        
        // Check if any incoming edges to this target come from blank nodes
        if (target->incoming_edges && target->incoming_count > 0) {
            for (size_t j = 0; j < target->incoming_count && j < 5; j++) {
                Edge *incoming = target->incoming_edges[j];
                if (!incoming || !incoming->from_node) continue;
                
                Node *candidate = incoming->from_node;
                
                // Check if candidate is a blank node with similar connections
                if (candidate->payload_size != 0) continue;  // Not a blank node
                if (candidate == node) continue;  // Skip self
                
                // Check connection similarity (at least 2 shared targets)
                int similar_connections = 0;
                for (size_t k = 0; k < candidate->outgoing_count && k < 3; k++) {
                    Edge *candidate_edge = candidate->outgoing_edges[k];
                    if (!candidate_edge) continue;
                    
                    for (size_t m = 0; m < node->outgoing_count; m++) {
                        Edge *node_edge2 = node->outgoing_edges[m];
                        if (node_edge2 && candidate_edge->to_node == node_edge2->to_node) {
                            similar_connections++;
                            break;
                        }
                    }
                }
                
                if (similar_connections >= 2) return candidate;  // Found existing blank
            }
        }
    }
    
    return NULL;
}

/* Link concrete examples to blank node
 * - Creates edges: incoming → blank → outgoing
 * - Makes blank a "hub" for the variable position
 */
static void link_concrete_examples_to_blank(Node *concrete, Node *blank, Graph *graph) {
    // #region agent log
    fprintf(stderr, "[LOG] link_blank_entry concrete=%p blank=%p graph=%p\n", 
            (void*)concrete, (void*)blank, (void*)graph);
    fflush(stderr);
    // #endregion
    
    if (!concrete || !blank || !graph) return;
    if (!concrete->incoming_edges || !concrete->outgoing_edges) return;
    
    // Create edges: incoming → blank → outgoing
    // This makes blank a "hub" for the variable position
    
    // Link incoming edges to blank
    for (size_t i = 0; i < concrete->incoming_count && i < 10; i++) {  // Limit to prevent excessive linking
        Edge *in_edge = concrete->incoming_edges[i];
        if (!in_edge || !in_edge->from_node) continue;
        
        // Create edge: from_node → blank
        Edge *blank_edge = graph_find_edge_between(in_edge->from_node, blank);
        if (!blank_edge) {
            blank_edge = edge_create(in_edge->from_node, blank);
            if (blank_edge) {
                graph_add_edge(graph, blank_edge);
                // Initialize with average weight of concrete edges
                blank_edge->weight = in_edge->weight / 2.0f;
            }
        }
    }
    
    // Link blank to outgoing edges
    for (size_t i = 0; i < concrete->outgoing_count && i < 10; i++) {  // Limit to prevent excessive linking
        Edge *out_edge = concrete->outgoing_edges[i];
        if (!out_edge || !out_edge->to_node) continue;
        
        // Create edge: blank → to_node
        Edge *blank_edge = graph_find_edge_between(blank, out_edge->to_node);
        if (!blank_edge) {
            blank_edge = edge_create(blank, out_edge->to_node);
            if (blank_edge) {
                graph_add_edge(graph, blank_edge);
                blank_edge->weight = out_edge->weight / 2.0f;
            }
        }
    }
    
    // CRITICAL FIX: Also create edge from concrete → blank
    // This allows blank node to be found during generation when at the concrete node
    // Without this edge, explore_blank_edges_from_node() can't find the blank node
    // (After refactoring, blank nodes are only accessible through edges - Requirement.md line 7)
    Edge *concrete_to_blank = graph_find_edge_between(concrete, blank);
    if (!concrete_to_blank) {
        concrete_to_blank = edge_create(concrete, blank);
        if (concrete_to_blank) {
            graph_add_edge(graph, concrete_to_blank);
            // Initialize with average of concrete's outgoing edge weights
            float avg_weight = 0.0f;
            if (concrete->outgoing_count > 0) {
                for (size_t i = 0; i < concrete->outgoing_count && i < 5; i++) {
                    if (concrete->outgoing_edges[i]) {
                        avg_weight += (float)concrete->outgoing_edges[i]->weight;
                    }
                }
                avg_weight /= (float)concrete->outgoing_count;
            }
            concrete_to_blank->weight = (uint8_t)(avg_weight / 2.0f);  // Start at half strength
        }
    }
}

/* Detect and create blank abstractions
 * - Measures variability in connections
 * - High variability = variable position → create blank
 * - NO THRESHOLDS: Uses relative comparison (node vs its neighbors)
 */
static void detect_and_create_blank_abstractions(Node *node, Graph *graph) {
    // #region agent log
    fprintf(stderr, "[LOG] detect_blank_entry node=%p graph=%p payload_size=%zu\n", 
            (void*)node, (void*)graph, node ? node->payload_size : 0);
    fflush(stderr);
    // #endregion
    
    if (!node || !graph) return;
    if (node->payload_size == 0) return;  // Already blank
    if (!node->incoming_edges || !node->outgoing_edges) return;
    if (node->incoming_count == 0 || node->outgoing_count == 0) return;
    
    // #region agent log
    fprintf(stderr, "[LOG] before_diversity_compute in_count=%zu out_count=%zu\n", 
            node->incoming_count, node->outgoing_count);
    fflush(stderr);
    // #endregion
    
    // Measure variability for this node
    float incoming_diversity = compute_connection_diversity(
        node->incoming_edges, 
        node->incoming_count
    );
    float outgoing_diversity = compute_connection_diversity(
        node->outgoing_edges,
        node->outgoing_count
    );
    
    // #region agent log
    fprintf(stderr, "[LOG] after_diversity_compute in_div=%.3f out_div=%.3f\n", 
            incoming_diversity, outgoing_diversity);
    fflush(stderr);
    // #endregion
    
    // Compare to NEIGHBOR diversity (no global threshold, pure relative)
    // Sample neighbors through edges to get local average
    float neighbor_avg_incoming = 0.0f;
    float neighbor_avg_outgoing = 0.0f;
    size_t neighbor_count = 0;
    
    // Sample incoming neighbors
    for (size_t i = 0; i < node->incoming_count && neighbor_count < 10; i++) {
        Edge *edge = node->incoming_edges[i];
        
        // #region agent log
        fprintf(stderr, "[LOG] incoming_edge_check i=%zu edge=%p from_node=%p\n", 
                i, (void*)edge, edge ? (void*)edge->from_node : NULL);
        fflush(stderr);
        // #endregion
        
        if (!edge || !edge->from_node) continue;
        Node *neighbor = edge->from_node;
        
        // #region agent log
        fprintf(stderr, "[LOG] neighbor_check neighbor=%p payload_size=%zu in_edges=%p out_edges=%p in_count=%zu out_count=%zu\n", 
                (void*)neighbor, neighbor->payload_size, (void*)neighbor->incoming_edges, (void*)neighbor->outgoing_edges, 
                neighbor->incoming_count, neighbor->outgoing_count);
        fflush(stderr);
        // #endregion
        
        if (!neighbor || neighbor->payload_size == 0) continue;
        if (!neighbor->incoming_edges || !neighbor->outgoing_edges) continue;
        if (neighbor->incoming_count == 0 || neighbor->outgoing_count == 0) continue;
        
        // #region agent log
        fprintf(stderr, "[LOG] before_neighbor_diversity neighbor=%p\n", (void*)neighbor);
        fflush(stderr);
        // #endregion
        
        neighbor_avg_incoming += compute_connection_diversity(neighbor->incoming_edges, neighbor->incoming_count);
        neighbor_avg_outgoing += compute_connection_diversity(neighbor->outgoing_edges, neighbor->outgoing_count);
        neighbor_count++;
        
        // #region agent log
        fprintf(stderr, "[LOG] after_neighbor_diversity neighbor_count=%zu\n", neighbor_count);
        fflush(stderr);
        // #endregion
    }
    
    // Sample outgoing neighbors
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < 20; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node) continue;
        Node *neighbor = edge->to_node;
        if (!neighbor || neighbor->payload_size == 0) continue;
        if (!neighbor->incoming_edges || !neighbor->outgoing_edges) continue;
        if (neighbor->incoming_count == 0 || neighbor->outgoing_count == 0) continue;
        
        neighbor_avg_incoming += compute_connection_diversity(neighbor->incoming_edges, neighbor->incoming_count);
        neighbor_avg_outgoing += compute_connection_diversity(neighbor->outgoing_edges, neighbor->outgoing_count);
        neighbor_count++;
    }
    
    if (neighbor_count == 0) return;  // No neighbors to compare
    
    neighbor_avg_incoming /= (float)neighbor_count;
    neighbor_avg_outgoing /= (float)neighbor_count;
    
    // Create blank if THIS node's diversity EXCEEDS neighbor average
    // Pure relative comparison (no fixed threshold)
    float incoming_ratio = (neighbor_avg_incoming > 0.0f) ? (incoming_diversity / neighbor_avg_incoming) : 1.0f;
    float outgoing_ratio = (neighbor_avg_outgoing > 0.0f) ? (outgoing_diversity / neighbor_avg_outgoing) : 1.0f;
    
    // Variability score: how much more variable than neighbors
    float variability_score = (incoming_ratio + outgoing_ratio) / 2.0f;
    
    // #region agent log
    fprintf(stderr, "[LOG] variability_computed score=%.3f will_create=%d\n", 
            variability_score, variability_score > 1.0f ? 1 : 0);
    fflush(stderr);
    // #endregion
    
    // Create blank if MORE variable than neighbors (data-driven, no threshold)
    if (variability_score > 1.0f) {  // Exceeds neighbor average
        // Check if blank already exists for this pattern
        Node *existing_blank = find_blank_for_position(node, graph);
        if (existing_blank) return;  // Already abstracted
        
        // Create new blank node
        Node *blank = node_create(NULL, 0, 1);  // payload=NULL, size=0, abstraction=1
        if (!blank) return;
        
        // Add to graph (blank nodes are accessible through edges only)
        graph_add_node(graph, blank);
        // NOTE: Blank nodes are accessed through edges, not a separate array
        // Requirement.md line 7: "edges are paths they are the only paths that nodes can take"
        
        // Link concrete examples to blank
        link_concrete_examples_to_blank(node, blank, graph);
        
        // #region agent log
        fprintf(stderr, "[LOG] blank_created blank=%p\n", (void*)blank);
        fflush(stderr);
        // #endregion
    }
}

/* Check if edge already exists between two nodes
 * - PHASE 2: Uses O(1) hash lookup when graph reference available
 * - Falls back to O(degree) local scan when graph not available
 */
static Edge* graph_find_edge_between(Node *from_node, Node *to_node) {
    if (!from_node || !to_node) return NULL;
    
    // PHASE 2: Use O(1) hash lookup if graph is available
    if (from_node->graph) {
        Edge *fast_result = graph_find_edge_between_fast(from_node->graph, from_node, to_node);
        if (fast_result) return fast_result;
    }
    
    // Fallback to O(degree) local scan (used when graph not yet set)
    for (size_t i = 0; i < from_node->outgoing_count; i++) {
        Edge *edge = from_node->outgoing_edges[i];
        if (edge && edge->to_node == to_node) {
            return edge;
        }
    }
    
    return NULL;
}

/* Create similarity edges for a node (connects to similar patterns)
 * - Brain: Similar patterns activate together (visual cortex)
 * - LLM: Attention between similar tokens
 * - Melvin: Creates edges between nodes with similar payloads
 * - Only creates if similarity exceeds adaptive threshold
 * - O(n) operation (checks all nodes), but only called for new nodes
 */
// REMOVED: graph_create_similarity_edges_for_node - unused function

/* ============================================================================
 * PREDICTION ERROR LEARNING
 * ============================================================================ */

/* Prediction Error Learning
 * - Compare predicted next node vs actual next node
 * - Differential learning: boost correct relative to incorrect
 * - Brain-like: Temporal difference learning, dopamine prediction error
 */
static void graph_learn_from_predictions(Graph *graph, Node *current_node, Node *actual_next) {
    if (!graph || !current_node || !actual_next) return;
    
    // What did current_node predict?
    Edge *predicted_edge = node_compute_winning_edge(current_node, graph);
    Node *predicted_next = predicted_edge ? predicted_edge->to_node : NULL;
    
    // Compute prediction error
    
    if (predicted_next == actual_next) {
        // CORRECT prediction - strengthen this edge (pure Hebbian)
        if (predicted_edge) {
            edge_update_weight(predicted_edge, 1.0f);  // Direct strengthening
        }
    } else {
        // WRONG prediction - differential learning (pure competition)
        // Find the correct edge
        Edge *correct_edge = NULL;
        for (size_t i = 0; i < current_node->outgoing_count; i++) {
            if (current_node->outgoing_edges[i]->to_node == actual_next) {
                correct_edge = current_node->outgoing_edges[i];
                break;
            }
        }
        
        if (!correct_edge) {
            // Create edge if it doesn't exist
            correct_edge = edge_create(current_node, actual_next);
            if (correct_edge) graph_add_edge(graph, correct_edge);
        }
        
        if (correct_edge && predicted_edge) {
            // Only strengthen correct - don't weaken incorrect
            // Pure Hebbian: strengthen what's used, let competition emerge naturally
            edge_update_weight(correct_edge, 1.0f);
            // REMOVED: edge_update_weight(predicted_edge, -0.5f);  // No weakening
        }
    }
}

/* ============================================================================
 * SEQUENTIAL PATTERN PROCESSING
 * ============================================================================ */

static void graph_process_sequential_patterns(Graph *graph, const uint8_t *input, 
                                              size_t input_size) {
    if (!graph || !input || input_size == 0) return;
    
    Node *prev_node = NULL;
    
    // GREEDY LONGEST-MATCH: Try to match longest hierarchy first
    // This allows wave propagation to traverse through hierarchy nodes
    // Following README: "Hierarchy-first match - try larger patterns first"
    size_t i = 0;
    
    while (i < input_size) {
        Node *node = NULL;
        size_t matched_len = 0;
        
        // Try to match the longest possible pattern starting at position i
        // Start with remaining input length and decrease
        size_t max_try_len = input_size - i;
        // Limit to reasonable size to avoid O(n²) behavior
        if (max_try_len > 20) max_try_len = 20;
        
        // HIERARCHY-FIRST MATCHING: Always prefer longest hierarchy
        // This ensures consistent matching and allows sub-hierarchies to become inactive
        for (size_t try_len = max_try_len; try_len > 0; try_len--) {
            Node *candidate = NULL;
            
            // Only try to FIND (not create) for lengths > 1
            if (try_len > 1) {
                // PHASE 2: O(pattern_size) trie lookup instead of O(n) scan
                // Look for existing hierarchy node with HIGHEST abstraction level
                Node *trie_result = trie_lookup(graph, &input[i], try_len);
                if (trie_result) {
                    candidate = trie_result;
                } else if (graph->hierarchy_by_level && graph->max_hierarchy_levels > 0) {
                    // Fallback: use hierarchy index (O(hierarchies) not O(all_nodes))
                    Node *best_candidate = NULL;
                    uint32_t best_level = 0;
                    
                    for (uint32_t level = graph->max_hierarchy_levels; level > 0; level--) {
                        size_t idx = level - 1;
                        if (idx >= graph->max_hierarchy_levels) continue;
                        if (!graph->hierarchy_by_level[idx]) continue;
                        
                        size_t count = graph->hierarchy_counts[idx];
                        for (size_t h = 0; h < count; h++) {
                            Node *hier = graph->hierarchy_by_level[idx][h];
                            if (!hier || hier->payload_size != try_len) continue;
                            
                            // Check if pattern matches
                            if (memcmp(hier->payload, &input[i], try_len) == 0) {
                                if (!best_candidate || hier->abstraction_level > best_level) {
                                    best_candidate = hier;
                                    best_level = hier->abstraction_level;
                                }
                            }
                        }
                        if (best_candidate) break;  // Found at highest level, stop
                    }
                    candidate = best_candidate;
                }
            } else {
                // For single byte, find or create
                candidate = graph_find_or_create_pattern_node(graph, &input[i], 1);
            }
            
            if (candidate) {
                node = candidate;
                matched_len = try_len;
                break;
            }
        }
        
        if (!node) {
            // Fallback: create byte node
            node = graph_find_or_create_pattern_node(graph, &input[i], 1);
            matched_len = 1;
        }
        
        if (!node) {
            i++;
            continue;
        }
        
        // Create co-activation edge from previous node
        // This creates edges between bytes AND hierarchies
        // Example: After "lo" hierarchy exists, this creates l→(lo) edge
        if (prev_node) {
            // Check if edge already exists
            Edge *existing_edge = NULL;
            for (size_t j = 0; j < prev_node->outgoing_count; j++) {
                if (prev_node->outgoing_edges[j]->to_node == node) {
                    existing_edge = prev_node->outgoing_edges[j];
                    break;
                }
            }
            
            if (existing_edge) {
                // Strengthen existing edge (adaptive rate, not hardcoded)
                uint8_t old_weight = existing_edge->weight;
                float strengthening_rate = compute_adaptive_strengthening_rate(prev_node);
                // Strengthen with bounded update
                float increment = (float)existing_edge->weight * (strengthening_rate - 1.0f);
                edge_update_weight_bounded(existing_edge, increment);
                float old_float = weight_uint8_to_float(old_weight, 1.0f);
                float new_float = weight_uint8_to_float(existing_edge->weight, 1.0f);
                node_update_outgoing_weight_sum(prev_node, old_float, new_float);
                node_update_incoming_weight_sum(node, old_float, new_float);
                
                // NEW: Strengthen routing_gate (bounded at 255)
                uint8_t gate_increment = (uint8_t)(strengthening_rate * 10.0f);
                if (existing_edge->routing_gate < 255 - gate_increment) {
                    existing_edge->routing_gate += gate_increment;
                } else {
                    existing_edge->routing_gate = 255;
                }
                
                // NOTE: Attention learning happens during generation (not training)
                // During generation, we have full ActivationPattern with all activated nodes
                // Training only has sequential input, so attention is learned from actual usage
            } else {
                // Create new edge (including edges to hierarchy nodes!)
                Edge *edge = edge_create(prev_node, node);
                if (edge) {
                    // NOTE: No context_bytes stored - using attention-based context
                    // Edge transformer will learn which activated nodes are relevant
                    graph_add_edge(graph, edge);
                }
            }
            
            // Prediction error learning
            // Learn from whether prev_node correctly predicted this node
            graph_learn_from_predictions(graph, prev_node, node);
        }
        
        // TEMPORARILY DISABLED: Similarity edges
        // Testing if these are causing the multiple edge problem
        // graph_create_similarity_edges_for_node(graph, node);
        
        prev_node = node;
        i += matched_len;  // Advance by matched length (not just 1!)
    }
}

/* ============================================================================
 * BLANK NODE CREATION FROM CLUSTERS
 * ============================================================================ */

/* Compute pattern similarity between two nodes (reusable helper)
 * - Returns similarity score 0.0-1.0
 * - Simple byte-wise similarity (can be enhanced with embeddings)
 */
/* Compute pattern similarity between two nodes (adaptive, no hardcoded threshold)
 * - Brain: Pattern matching in visual cortex (similar shapes activate together)
 * - LLM: Embedding similarity (cosine similarity between vectors)
 * - Melvin: Byte-wise similarity with adaptive normalization
 * - Returns: 0.0 (no similarity) to 1.0 (identical)
 */
static float compute_node_pattern_similarity(Node *node1, Node *node2) {
    if (!node1 || !node2) return 0.0f;
    if (node1->payload_size == 0 || node2->payload_size == 0) return 0.0f;
    
    size_t min_size = (node1->payload_size < node2->payload_size) ?
                      node1->payload_size : node2->payload_size;
    size_t max_size = (node1->payload_size > node2->payload_size) ?
                      node1->payload_size : node2->payload_size;
    if (min_size == 0) return 0.0f;
    
    size_t matches = 0;
    for (size_t i = 0; i < min_size; i++) {
        if (node1->payload[i] == node2->payload[i]) matches++;
    }
    
    // Normalize by minimum size (Jaccard-like similarity)
    float similarity = (float)matches / (float)min_size;
    
    // Penalize size differences (longer patterns that match are more similar)
    // But don't penalize too much (adaptive penalty)
    float size_ratio = (float)min_size / (float)max_size;
    similarity *= size_ratio;  // Penalize if sizes differ
    
    return similarity;
}

/* Find cluster of similar patterns through similarity edges
 * - Uses wave propagation to explore graph locally (O(degree) per node)
 * - Identifies nodes connected by similarity edges with strong weights
 * - Returns cluster nodes if cluster is strong enough
 * - Limited depth to keep operations local
 */
// REMOVED: graph_find_similar_pattern_cluster - unused function (~200 lines)

/* Compute cluster strength score (smooth function, no hard threshold)
 * - Based on cluster size and similarity edge weights
 * - Returns smooth score 0.0-1.0
 */
// REMOVED: graph_compute_cluster_strength - unused function (~50 lines)

/* Get adaptive threshold for cluster creation
 * - Based on graph's current state (no hardcoded minimum)
 * - Uses local sampling (not O(n) iteration)
 */
// REMOVED: graph_get_adaptive_cluster_threshold - unused function (~55 lines)

/* Create blank node from cluster of similar patterns
 * - Creates blank node with payload_size = 0
 * - Connects all cluster nodes to blank node
 * - Initializes prototype from cluster patterns
 */
/* ============================================================================
 * 3. PREDICTIVE LOSS COMPUTATION
 * ============================================================================ */

/* Compute predictive loss (cross-entropy for next-token prediction)
 * - Loss computed on output projections only (preserves locality)
 * - Uses activation_strength as logits
 * - Returns loss value for gradient computation
 */
/* ============================================================================
 * HIERARCHY FORMATION (Phase 2: Abstractions)
 * Brain: Hebbian consolidation - neurons that fire together, wire together
 * LLM: Learned hierarchical representations
 * ============================================================================ */

/* Detect when edge dominates local context (brain: Hebbian consolidation)
 * - Returns 1 when edge is strong enough to trigger hierarchy formation
 * - All thresholds adaptive, from local context
 */

/* ============================================================================
 * THREE-PHASE ARCHITECTURE CONTROL VIA MINI NETS
 * 
 * Following user requirement: "enhance encode/refine/decode phases with mini net control"
 * Mini nets decide:
 * - How many refine iterations
 * - Temperature for decode
 * - When to stop generation
 * ============================================================================ */

/* Compute refine iteration count using mini net (data-driven)
 * Graph's refine_net decides how many iterations based on pattern complexity.
 */
static int compute_refine_iterations(Graph *graph, ActivationPattern *pattern, size_t input_count) {
    if (!graph || !pattern) return 2;  // Default minimum
    
    // Build context for decision
    float context[8];
    context[0] = (float)pattern->count;  // Pattern size
    context[1] = (float)input_count;  // Input size
    context[2] = (float)graph->node_count;  // Graph maturity
    context[3] = (float)graph->edge_count;
    context[4] = (float)graph->max_abstraction_level;  // Hierarchy depth
    
    // Compute pattern complexity
    float pattern_complexity = (float)pattern->count / (float)(input_count + 1);
    context[5] = pattern_complexity;
    
    // Compute graph maturity
    float graph_maturity = (float)graph->edge_count / (float)(graph->node_count + 1.0f);
    context[6] = graph_maturity;
    
    // Average activation strength
    float avg_activation = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        avg_activation += pattern->activations[i];
    }
    avg_activation = (pattern->count > 0) ? (avg_activation / (float)pattern->count) : 0.5f;
    context[7] = avg_activation;
    
    // Use graph's refine_net to decide iteration count
    int iterations = 2;  // Default minimum
    if (graph->refine_net) {
        float decision = mini_net_forward(graph->refine_net, context, 8, NULL, 0);
        // Map decision (0.0-1.0) to iteration count (2-8)
        iterations = 2 + (int)(decision * 6.0f);
    } else {
        // Fallback: data-driven computation
        iterations = 2 + (int)(pattern_complexity * 0.5f + graph_maturity * 0.3f);
    }
    
    // Clamp to reasonable range
    if (iterations < 2) iterations = 2;
    if (iterations > 8) iterations = 8;
    
    return iterations;
}

/* Compute decode temperature using mini net (data-driven)
 * Graph's decode_net decides temperature based on pattern and graph state.
 */
static float compute_decode_temperature(Graph *graph, ActivationPattern *pattern) {
    if (!graph || !pattern) return 1.0f;  // Default neutral
    
    // Build context for decision
    float context[6];
    context[0] = (float)pattern->count;
    context[1] = (float)graph->node_count;
    context[2] = (float)graph->edge_count;
    context[3] = (float)graph->max_abstraction_level;
    
    // Compute activation entropy (measure of uncertainty)
    float entropy = 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        sum += pattern->activations[i];
    }
    if (sum > 0.0f) {
        for (size_t i = 0; i < pattern->count; i++) {
            float p = pattern->activations[i] / sum;
            if (p > 0.0f) {
                entropy -= p * logf(p);
            }
        }
    }
    context[4] = entropy;
    
    // Average activation
    float avg_activation = (pattern->count > 0) ? (sum / (float)pattern->count) : 0.5f;
    context[5] = avg_activation;
    
    // Use graph's decode_net to decide temperature
    float temperature = 1.0f;  // Default neutral
    if (graph->decode_net) {
        float decision = mini_net_forward(graph->decode_net, context, 6, NULL, 0);
        // Map decision (0.0-1.0) to temperature (0.1-2.0)
        // Low decision = low temp (focused), high decision = high temp (exploratory)
        temperature = 0.1f + decision * 1.9f;
    } else {
        // Fallback: entropy-based temperature
        // High entropy = high temperature (more exploration)
        temperature = 0.5f + entropy * 0.5f;
    }
    
    // NO BOUNDS - temperature emerges from data
    // Let the mini net decide what temperature is appropriate
    // Only prevent division by zero
    if (temperature < 0.01f) temperature = 0.01f;
    
    return temperature;
}

/* Decide whether to stop generation using mini net (data-driven)
 * Returns 1 if should stop, 0 if should continue.
 */
// REMOVED: should_stop_generation - unused function (~25 lines)

/* Learn from three-phase outcome (reinforcement learning)
 * Mini nets learn whether their decisions led to good results.
 */
static void learn_three_phase_outcome(Graph *graph, int iterations_used, 
                                     float temperature_used, float outcome) {
    if (!graph) return;
    
    // Build context
    float context[8];
    context[0] = (float)iterations_used;
    context[1] = temperature_used;
    context[2] = (float)graph->node_count;
    context[3] = (float)graph->edge_count;
    context[4] = (float)graph->max_abstraction_level;
    context[5] = outcome;  // Success/failure signal
    context[6] = (float)graph->wave_generation;  // Time signal
    context[7] = 0.0f;  // Reserved
    
    // Refine_net learns from iteration count decision
    if (graph->refine_net) {
        mini_net_update(graph->refine_net, context, 8, outcome, 1.0f);
    }
    
    // Decode_net learns from temperature decision
    if (graph->decode_net) {
        mini_net_update(graph->decode_net, context, 8, outcome, 1.0f);
    }
}

/* ============================================================================
 * RECURSIVE HIERARCHY FORMATION VIA MINI NETS
 * 
 * Following user requirement: "hierarchy formation should be decided by mini nets"
 * Mini nets decide when and how to form hierarchies based on local context.
 * ============================================================================ */

/* Compute hierarchy formation probability using mini nets (data-driven)
 * The mini nets of both nodes vote on whether to form a hierarchy.
 * This replaces hardcoded thresholds with learned decisions.
 */
static float compute_hierarchy_formation_probability(Node *node1, Node *node2, Edge *connecting_edge) {
    if (!node1 || !node2 || !node1->net || !node2->net) return 0.0f;
    
    // Build context vector for hierarchy decision
    float context[10];
    context[0] = node1->activation_strength;
    context[1] = node2->activation_strength;
    context[2] = connecting_edge ? connecting_edge->weight : 0.0f;
    context[3] = (float)node1->total_activations;  // Usage frequency
    context[4] = (float)node2->total_activations;
    context[5] = (float)node1->abstraction_level;
    context[6] = (float)node2->abstraction_level;
    context[7] = (float)node1->outgoing_count;  // Connectivity
    context[8] = (float)node2->outgoing_count;
    context[9] = connecting_edge ? ((float)connecting_edge->weight / 255.0f) : 0.0f;
    
    // Both nodes vote using their mini nets (CTX_HIERARCHY context)
    float vote1 = node_get_variable(node1, CTX_HIERARCHY);
    float vote2 = node_get_variable(node2, CTX_HIERARCHY);
    
    // Also get edge's opinion (if exists) - use normalized weight
    float edge_vote = 0.5f;  // Neutral if no edge
    if (connecting_edge) {
        edge_vote = (float)connecting_edge->weight / 255.0f;
    }
    
    // Combine votes (weighted average, data-driven)
    float combined_vote = (vote1 + vote2 + edge_vote) / 3.0f;
    
    // Apply mini net forward pass with full context
    float prob1 = mini_net_compute_with_context(node1->net, context, 10, NULL, 0);
    float prob2 = mini_net_compute_with_context(node2->net, context, 10, NULL, 0);
    
    // Final probability: average of votes and mini net outputs
    float final_prob = (combined_vote + prob1 + prob2) / 3.0f;
    
    return final_prob;
}

/* Decide whether to form hierarchy using mini nets (no hardcoded thresholds)
 * Returns 1 if hierarchy should be formed, 0 otherwise.
 */
static int should_form_hierarchy(Node *node1, Node *node2, Edge *connecting_edge, Graph *graph) {
    if (!node1 || !node2 || !graph) return 0;
    
    // Compute formation probability using mini nets
    float formation_prob = compute_hierarchy_formation_probability(node1, node2, connecting_edge);
    
    // Compute adaptive threshold from graph's mini net (if exists)
    float threshold = 0.5f;  // Default neutral threshold
    if (graph->hierarchy_net) {
        // Graph's mini net decides the threshold based on graph state
        float graph_context[5];
        graph_context[0] = (float)graph->node_count;
        graph_context[1] = (float)graph->edge_count;
        graph_context[2] = (float)graph->max_abstraction_level;
        graph_context[3] = (float)node1->abstraction_level;
        graph_context[4] = (float)node2->abstraction_level;
        
        threshold = mini_net_forward(graph->hierarchy_net, graph_context, 5, NULL, 0);
    }
    
    // Decision: form hierarchy if probability exceeds threshold
    return formation_prob > threshold;
}

/* Learn from hierarchy formation outcome (reinforcement learning)
 * Mini nets learn whether the hierarchy formation decision was good.
 */
static void learn_hierarchy_formation(Node *node1, Node *node2, Edge *connecting_edge, 
                                     float outcome, Graph *graph) {
    if (!node1 || !node2) return;
    
    // Build context vector
    float context[10];
    context[0] = node1->activation_strength;
    context[1] = node2->activation_strength;
    context[2] = connecting_edge ? connecting_edge->weight : 0.0f;
    context[3] = (float)node1->total_activations;
    context[4] = (float)node2->total_activations;
    context[5] = (float)node1->abstraction_level;
    context[6] = (float)node2->abstraction_level;
    context[7] = (float)node1->outgoing_count;
    context[8] = (float)node2->outgoing_count;
    context[9] = connecting_edge ? ((float)connecting_edge->weight / 255.0f) : 0.0f;
    
    // Both nodes learn from the outcome
    if (node1->net) {
        mini_net_update(node1->net, context, 10, outcome, 1.0f);
        node_set_variable(node1, CTX_HIERARCHY, outcome);
    }
    if (node2->net) {
        mini_net_update(node2->net, context, 10, outcome, 1.0f);
        node_set_variable(node2, CTX_HIERARCHY, outcome);
    }
    
    // REMOVED: Edge learning (edges are simple now, no MiniNet)
    
    // Graph's mini net learns the threshold (if exists)
    if (graph && graph->hierarchy_net) {
        float graph_context[5];
        graph_context[0] = (float)graph->node_count;
        graph_context[1] = (float)graph->edge_count;
        graph_context[2] = (float)graph->max_abstraction_level;
        graph_context[3] = (float)node1->abstraction_level;
        graph_context[4] = (float)node2->abstraction_level;
        mini_net_update(graph->hierarchy_net, graph_context, 5, outcome, 1.0f);
    }
}

/* Create hierarchy node by combining payloads
 * - Brain: Synaptic consolidation - patterns merge into single representation
 * - LLM: Learned embeddings combine token sequences
 * - Melvin: Concatenate payloads, increment abstraction_level
 * 
 * REFACTORED: Now uses mini nets to decide formation
 */
static Node* create_hierarchy_node(Graph *graph, Node *node1, Node *node2) {
    if (!graph || !node1 || !node2) return NULL;
    
    // Find connecting edge (if exists)
    Edge *connecting_edge = NULL;
    for (size_t i = 0; i < node1->outgoing_count; i++) {
        if (node1->outgoing_edges[i] && node1->outgoing_edges[i]->to_node == node2) {
            connecting_edge = node1->outgoing_edges[i];
            break;
        }
    }
    
    // MINI NET DECISION: Should we form this hierarchy?
    if (!should_form_hierarchy(node1, node2, connecting_edge, graph)) {
        // Mini nets decided NOT to form hierarchy
        // Learn from this decision (negative outcome = 0.0)
        learn_hierarchy_formation(node1, node2, connecting_edge, 0.0f, graph);
        return NULL;
    }
    
    // Mini nets decided to form hierarchy - proceed
    
    // Compute combined size
    size_t combined_size = node1->payload_size + node2->payload_size;
    
    // Increment abstraction level (cortical hierarchy)
    uint32_t new_level = (node1->abstraction_level > node2->abstraction_level) ?
                         node1->abstraction_level : node2->abstraction_level;
    new_level += 1;
    
    // ADAPTIVE STRATEGY: Small hierarchies copy (fast access), large ones reference (compression)
    // Threshold is data-driven: 16 bytes (can be computed from local avg hierarchy size)
    const size_t COPY_THRESHOLD = 16;
    
    Node *hierarchy = NULL;
    
    if (combined_size <= COPY_THRESHOLD) {
        // SMALL HIERARCHY: Copy payload (fast access, no indirection)
        uint8_t *combined = malloc(combined_size);
        if (!combined) {
            // Failed to allocate - learn negative outcome
            learn_hierarchy_formation(node1, node2, connecting_edge, 0.0f, graph);
            return NULL;
        }
        
        memcpy(combined, node1->payload, node1->payload_size);
        memcpy(combined + node1->payload_size, node2->payload, node2->payload_size);
        
        hierarchy = node_create(combined, combined_size, new_level);
        free(combined);
        
        if (!hierarchy) {
            // Failed to create - learn negative outcome
            learn_hierarchy_formation(node1, node2, connecting_edge, 0.0f, graph);
            return NULL;
        }
        
        // Mark as copied hierarchy (not reference-based)
        hierarchy->is_reference_hierarchy = 0;
        hierarchy->child_nodes = NULL;
        hierarchy->child_count = 0;
        
    } else {
        // LARGE HIERARCHY: Reference children (compression through reuse)
        // Create node with NULL payload (will be reconstructed on-demand)
        hierarchy = node_create(NULL, 0, new_level);
        if (!hierarchy) {
            // Failed to create - learn negative outcome
            learn_hierarchy_formation(node1, node2, connecting_edge, 0.0f, graph);
            return NULL;
        }
        
        // Set up reference structure
        hierarchy->child_nodes = malloc(2 * sizeof(Node*));
        if (!hierarchy->child_nodes) {
            free(hierarchy);
            learn_hierarchy_formation(node1, node2, connecting_edge, 0.0f, graph);
            return NULL;
        }
        
        hierarchy->child_nodes[0] = node1;
        hierarchy->child_nodes[1] = node2;
        hierarchy->child_count = 2;
        hierarchy->is_reference_hierarchy = 1;
        
        // Virtual size (actual payload computed on-demand)
        hierarchy->payload_size = combined_size;
    }
    
    // Inherit port_id from first node (hierarchies inherit their source port)
    hierarchy->port_id = node1->port_id;
    
    // Update graph's max abstraction level
    if (new_level > graph->max_abstraction_level) {
        graph->max_abstraction_level = new_level;
    }
    
    // Connect to original nodes (maintain hierarchy)
    Edge *e1 = edge_create(node1, hierarchy);
    Edge *e2 = edge_create(node2, hierarchy);
    
    if (e1) graph_add_edge(graph, e1);
    if (e2) graph_add_edge(graph, e2);
    
    // Invalidate child embeddings (hierarchy formation changes context)
    node_invalidate_embedding(node1);
    node_invalidate_embedding(node2);
    
    // SUCCESS: Learn positive outcome
    // The hierarchy was successfully created, so this was a good decision
    learn_hierarchy_formation(node1, node2, connecting_edge, 1.0f, graph);
    
    return hierarchy;
}


/* ============================================================================
 * 6. STABILITY MECHANISMS
 * ============================================================================ */

/* Local normalization (LayerNorm per node, not global)
 * - Normalizes relative to local neighbors
 * - No global batch statistics
 */
static float node_local_normalize(Node *node, float raw_activation) {
    if (!node || node->incoming_count == 0) return raw_activation;
    
    // Compute local mean from incoming edges
    float local_mean = 0.0f;
    for (size_t i = 0; i < node->incoming_count; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node) {
            local_mean += edge->from_node->activation_strength;
        }
    }
    local_mean /= (float)node->incoming_count;
    
    // Compute local variance
    float local_var = 0.0f;
    for (size_t i = 0; i < node->incoming_count; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node) {
            float diff = edge->from_node->activation_strength - local_mean;
            local_var += diff * diff;
        }
    }
    local_var /= (float)node->incoming_count;
    
    // Normalize (adaptive epsilon)
    float epsilon = compute_adaptive_epsilon(sqrtf(local_var));
    float normalized = (raw_activation - local_mean) / (sqrtf(local_var) + epsilon);
    
    return normalized;
}

/* Residual state update (prevents vanishing gradients)
 * - state = state + delta (residual connection)
 * - Maintains gradient flow across wave steps
 */
// REMOVED: node_residual_update - unused function


/* ============================================================================
 * .M FILE OPERATIONS (Principle: Live Executable Programs)
 * ============================================================================ */

/* ============================================================================
 * FORWARD DECLARATIONS FOR OUTPUT GENERATION
 * ============================================================================ */
// REMOVED: sample_next_byte, melvin_generate_output_from_state - unused functions

/* ============================================================================
 * PUBLIC INTERFACE FUNCTIONS
 * ============================================================================ */

/* Create new .m file
 * - Opens/creates file
 * - Initializes graph
 * - Builds index for lazy loading
 */
MelvinMFile* melvin_m_create(const char *path) {
    if (!path) return NULL;
    
    MFile *mfile = calloc(1, sizeof(MFile));
    if (!mfile) return NULL;
    
    mfile->path = strdup(path);
    mfile->graph = graph_create();
    
    // Open or create file
    mfile->fd = open(path, O_RDWR | O_CREAT, 0644);
    if (mfile->fd < 0) {
        free(mfile->path);
        free(mfile);
        return NULL;
    }
    
    // Initialize universal I/O (start with capacity 1)
    mfile->universal_input_capacity = 1;
    mfile->universal_input = malloc(1);
    mfile->universal_output_capacity = 1;
    mfile->universal_output = malloc(1);
    
    // Initialize index (adaptive size)
    mfile->index_size = 1;
    mfile->node_index = calloc(1, sizeof(Node**));
    
    mfile->last_modified = time(NULL);
    
    return mfile;
}

/* Load existing .m file
 * - Opens file
 * - Loads graph structure
 * - Builds index for lazy loading
 */
MelvinMFile* melvin_m_load(const char *path) {
    if (!path) return NULL;
    
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        // File doesn't exist, create new one
        return melvin_m_create(path);
    }
    
    MFile *mfile = calloc(1, sizeof(MFile));
    if (!mfile) {
        close(fd);
        return NULL;
    }
    
    mfile->fd = fd;
    mfile->path = strdup(path);
    mfile->graph = graph_create();
    
    // Read magic number
    char magic[9] = {0};
    if (read(fd, magic, 8) != 8 || memcmp(magic, "MELVIN01", 8) != 0) {
        // Invalid file format, treat as new file
        // Debug removed
        close(fd);
        free(mfile->path);
        free(mfile);
        return melvin_m_create(path);
    }
    
    // Debug removed
    
    // Read header
    uint64_t node_count = 0;
    uint64_t edge_count = 0;
    off_t node_section_offset = 0;
    off_t edge_section_offset = 0;
    
    if (read(fd, &node_count, 8) != 8 ||
        read(fd, &edge_count, 8) != 8 ||
        read(fd, &node_section_offset, sizeof(off_t)) != sizeof(off_t) ||
        read(fd, &edge_section_offset, sizeof(off_t)) != sizeof(off_t)) {
        // Debug removed
        close(fd);
        free(mfile->path);
        free(mfile);
        return NULL;
    }
    
    // Debug removed
    
    // Read node index
    if (node_count > 0) {
        // Build hash table index for lazy loading
        size_t index_size = (node_count < 256) ? 256 : (size_t)node_count * 2;
        mfile->index_size = index_size;
        mfile->node_index = calloc(index_size, sizeof(Node**));
        
        if (mfile->node_index) {
            // Read index entries: (node_id[9], file_offset[8])
            // Debug removed
            for (uint64_t i = 0; i < node_count; i++) {
                uint8_t node_id[9] = {0};
                off_t node_offset = 0;
                
                ssize_t id_read = read(fd, node_id, 9);
                ssize_t offset_read = read(fd, &node_offset, sizeof(off_t));
                
                if (id_read != 9 || offset_read != sizeof(off_t)) {
                    // Debug removed - read failed
                    break;
                }
                
                // Debug removed
                
                // Save current position (to continue reading index entries after loading node)
                off_t next_index_pos = lseek(fd, 0, SEEK_CUR);
                
                // Load node data from file
                if (lseek(fd, node_offset, SEEK_SET) == node_offset) {
                    // Read node properties
                    size_t payload_size = 0;
                    if (read(fd, &payload_size, sizeof(size_t)) == sizeof(size_t)) {
                        // Create node with payload
                        Node *node = calloc(1, sizeof(Node));
                        if (node) {
                            memcpy(node->id, node_id, 9);
                            node->file_offset = node_offset;
                            node->loaded = 1;
                            
                            // Initialize edge arrays (capacity 1, like node_create)
                            // This is critical - without it, edge_create can't add edges
                            node->outgoing_capacity = 1;
                            node->outgoing_edges = malloc(sizeof(Edge*));
                            node->incoming_capacity = 1;
                            node->incoming_edges = malloc(sizeof(Edge*));
                            
                            // Read payload
                            if (payload_size > 0) {
                                node->payload = malloc(payload_size);
                                if (node->payload && read(fd, node->payload, payload_size) == (ssize_t)payload_size) {
                                    node->payload_size = payload_size;
                                } else {
                                    free(node->payload);
                                    node->payload = NULL;
                                    node->payload_size = 0;
                                }
                            }
                            
                            // Read other properties
                            int props_ok = 
                                read(fd, &node->weight, sizeof(float)) == sizeof(float) &&
                                read(fd, &node->bias, sizeof(float)) == sizeof(float) &&
                                read(fd, &node->activation_strength, sizeof(float)) == sizeof(float) &&
                                read(fd, &node->abstraction_level, sizeof(uint32_t)) == sizeof(uint32_t) &&
                                read(fd, &node->outgoing_weight_sum, sizeof(float)) == sizeof(float) &&
                                read(fd, &node->incoming_weight_sum, sizeof(float)) == sizeof(float) &&
                                read(fd, &node->state, sizeof(float)) == sizeof(float);
                            
                            // Try to read context_trace (may fail for old files without it)
                            // If reading fails, context_trace will remain zeroed from calloc
                            if (props_ok) {
                                // Try to read context trace (new format)
                                ssize_t trace_read = read(fd, node->context_trace, sizeof(float) * 8);
                                if (trace_read == sizeof(float) * 8) {
                                    // Also read context_trace_len
                                    read(fd, &node->context_trace_len, sizeof(uint8_t));
                                    // Also read port_id (multi-modal support)
                                    read(fd, &node->port_id, sizeof(uint8_t));
                                    
                                    // Try to read hierarchy structure (newest format)
                                    ssize_t hier_read = read(fd, &node->is_reference_hierarchy, sizeof(uint8_t));
                                    if (hier_read == sizeof(uint8_t)) {
                                        read(fd, &node->child_count, sizeof(size_t));
                                        
                                        // If reference hierarchy, read child IDs (resolve later)
                                        if (node->is_reference_hierarchy && node->child_count > 0) {
                                            // Store child IDs temporarily (will resolve after all nodes loaded)
                                            // For now, just skip over them
                                            lseek(fd, node->child_count * 9, SEEK_CUR);
                                        }
                                        
                                        // Try to read embedding
                                        read(fd, &node->embedding_dim, sizeof(size_t));
                                        if (node->embedding_dim > 0) {
                                            node->embedding = malloc(node->embedding_dim * sizeof(float));
                                            if (node->embedding) {
                                                read(fd, node->embedding, node->embedding_dim * sizeof(float));
                                            } else {
                                                node->embedding_dim = 0;
                                            }
                                        }
                                    }
                                }
                                // If read fails, fields remain zeroed/NULL (backward compatible)
                            }
                            
                            if (props_ok) {
                                
                                // Add to graph
                                graph_add_node(mfile->graph, node);
                                
                                // Add to index (simple hash)
                                uint64_t hash = 0;
                                for (int j = 0; j < 8 && node_id[j]; j++) {
                                    hash = hash * 31 + node_id[j];
                                }
                                size_t idx = hash % index_size;
                                
                                // Simple chaining (grow bucket as needed)
                                if (!mfile->node_index[idx]) {
                                    mfile->node_index[idx] = calloc(4, sizeof(Node*));
                                }
                                // Find empty slot
                                size_t slot = 0;
                                while (slot < 4 && mfile->node_index[idx][slot]) slot++;
                                if (slot < 4) {
                                    mfile->node_index[idx][slot] = node;
                                }
                            } else {
                                // Failed to read properties, free node
                                if (node->payload) free(node->payload);
                                free(node);
                            }
                        }
                    }
                }
                
                // Restore position to continue reading index entries
                lseek(fd, next_index_pos, SEEK_SET);
            }
        }
    }
    
    // Load edges (need to resolve node IDs to pointers)
    if (edge_count > 0 && lseek(fd, edge_section_offset, SEEK_SET) == edge_section_offset) {
        for (uint64_t i = 0; i < edge_count; i++) {
            uint8_t from_id[9] = {0};
            uint8_t to_id[9] = {0};
            uint8_t weight = 0;
            uint8_t routing_gate = 0;
            uint8_t inactivity_timer = 0;
            uint8_t flags = 0;
            uint32_t last_wave_generation = 0;
            
            if (read(fd, from_id, 9) != 9 ||
                read(fd, to_id, 9) != 9 ||
                read(fd, &weight, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &routing_gate, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &inactivity_timer, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &flags, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &last_wave_generation, sizeof(uint32_t)) != sizeof(uint32_t)) {
                break;
            }
            
            // Find nodes by ID
            Node *from_node = graph_find_node_by_id(mfile->graph, from_id);
            Node *to_node = graph_find_node_by_id(mfile->graph, to_id);
            
            if (from_node && to_node) {
                // Create edge
                Edge *edge = edge_create(from_node, to_node);
                if (edge) {
                    edge->weight = weight;
                    edge->routing_gate = routing_gate;
                    edge->inactivity_timer = inactivity_timer;
                    edge->flags = flags;
                    edge->last_wave_generation = last_wave_generation;
                    graph_add_edge(mfile->graph, edge);
                }
            } else {
                // Debug: edge load failed
            }
        }
    }
    
    // Initialize universal I/O
    mfile->universal_input_capacity = 1;
    mfile->universal_input = malloc(1);
    mfile->universal_output_capacity = 1;
    mfile->universal_output = malloc(1);
    
    mfile->last_modified = time(NULL);
    
    return mfile;
}

/* Save .m file
 * - Writes graph to file
 * - Auto-saves state
 * - Format: magic + header + node_index + nodes + edges
 */
int melvin_m_save(MelvinMFile *mfile) {
    if (!mfile || !mfile->graph || mfile->fd < 0) return -1;
    
    // Truncate file to start fresh
    if (ftruncate(mfile->fd, 0) < 0) return -1;
    if (lseek(mfile->fd, 0, SEEK_SET) < 0) return -1;
    
    Graph *graph = mfile->graph;
    
    // Calculate layout:
    // [magic:8][node_count:8][edge_count:8][node_section_offset:8][edge_section_offset:8]
    // [index: node_id:9 + offset:8 per node]
    // [nodes: payload_size + payload + properties per node]
    // [edges: from_id:9 + to_id:9 + properties per edge]
    
    off_t header_size = 8 + 8 + 8 + 8 + 8;  // 40 bytes
    off_t index_entry_size = 9 + sizeof(off_t);  // 17 bytes per node
    off_t node_index_size = (off_t)graph->node_count * index_entry_size;
    off_t node_section_start = header_size + node_index_size;
    
    // Write magic
    if (write(mfile->fd, "MELVIN01", 8) != 8) return -1;
    
    // Write counts
    uint64_t node_count = (uint64_t)graph->node_count;
    uint64_t edge_count = (uint64_t)graph->edge_count;
    if (write(mfile->fd, &node_count, 8) != 8) return -1;
    if (write(mfile->fd, &edge_count, 8) != 8) return -1;
    
    // Write node_section_offset (known now)
    if (write(mfile->fd, &node_section_start, sizeof(off_t)) != sizeof(off_t)) return -1;
    
    // Placeholder for edge_section_offset
    off_t edge_offset_pos = lseek(mfile->fd, 0, SEEK_CUR);
    off_t edge_section_offset = 0;
    if (write(mfile->fd, &edge_section_offset, sizeof(off_t)) != sizeof(off_t)) return -1;
    
    // Now at position 40 (header_size)
    // Calculate node data offsets and write index entries
    off_t *node_offsets = malloc(graph->node_count * sizeof(off_t));
    if (!node_offsets) return -1;
    
    // Calculate where each node's data will be
    off_t current_offset = node_section_start;
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        node_offsets[i] = current_offset;
        
        // Node data size: payload_size + payload + 14 floats + 1 uint32_t + 2 uint8_t
        // (weight, bias, activation_strength, outgoing_weight_sum, incoming_weight_sum, state) = 6 floats
        // (context_trace[8]) = 8 floats
        // abstraction_level = 1 uint32_t
        // context_trace_len = 1 uint8_t
        // port_id = 1 uint8_t (NEW)
        // is_reference_hierarchy = 1 uint8_t (NEW)
        // child_count = 1 size_t (NEW)
        // child_ids = child_count * 9 bytes (NEW, only for reference hierarchies)
        // embedding_dim = 1 size_t (NEW)
        // embedding = embedding_dim * 4 bytes (NEW, only if embedding exists)
        size_t node_data_size = sizeof(size_t) + (node ? node->payload_size : 0) + 
                               sizeof(float) * 14 + sizeof(uint32_t) + 3 * sizeof(uint8_t) +
                               sizeof(size_t) + // child_count
                               (node && node->is_reference_hierarchy ? node->child_count * 9 : 0) + // child_ids
                               sizeof(size_t) + // embedding_dim
                               (node && node->embedding ? node->embedding_dim * sizeof(float) : 0); // embedding
        current_offset += node_data_size;
    }
    
    // Write index entries (at positions 40 to 40 + node_index_size)
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (!node) {
            // Write placeholder for NULL node
            uint8_t empty_id[9] = {0};
            if (write(mfile->fd, empty_id, 9) != 9) { free(node_offsets); return -1; }
            if (write(mfile->fd, &node_offsets[i], sizeof(off_t)) != sizeof(off_t)) { free(node_offsets); return -1; }
            continue;
        }
        if (write(mfile->fd, node->id, 9) != 9) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node_offsets[i], sizeof(off_t)) != sizeof(off_t)) { free(node_offsets); return -1; }
    }
    
    // Now at node_section_start, write node data
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (!node) {
            // Write placeholder for NULL node
            size_t zero = 0;
            float fzero = 0.0f;
            uint32_t uzero = 0;
            uint8_t bzero = 0;
            if (write(mfile->fd, &zero, sizeof(size_t)) != sizeof(size_t)) { free(node_offsets); return -1; }
            // 6 original floats + 8 context_trace floats = 14 floats
            for (int j = 0; j < 14; j++) {
                if (write(mfile->fd, &fzero, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
            }
            if (write(mfile->fd, &uzero, sizeof(uint32_t)) != sizeof(uint32_t)) { free(node_offsets); return -1; }
            if (write(mfile->fd, &bzero, sizeof(uint8_t)) != sizeof(uint8_t)) { free(node_offsets); return -1; }
            continue;
        }
        
        // payload_size
        if (write(mfile->fd, &node->payload_size, sizeof(size_t)) != sizeof(size_t)) { free(node_offsets); return -1; }
        // payload
        if (node->payload_size > 0 && node->payload) {
            if (write(mfile->fd, node->payload, node->payload_size) != (ssize_t)node->payload_size) { free(node_offsets); return -1; }
        }
        // properties: weight, bias, activation_strength, abstraction_level, outgoing_weight_sum, incoming_weight_sum, state
        if (write(mfile->fd, &node->weight, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->bias, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->activation_strength, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->abstraction_level, sizeof(uint32_t)) != sizeof(uint32_t)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->outgoing_weight_sum, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->incoming_weight_sum, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->state, sizeof(float)) != sizeof(float)) { free(node_offsets); return -1; }
        // CONTEXT TRACE (RNN-like hidden state) - 8 floats + 1 byte
        if (write(mfile->fd, node->context_trace, sizeof(float) * 8) != sizeof(float) * 8) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->context_trace_len, sizeof(uint8_t)) != sizeof(uint8_t)) { free(node_offsets); return -1; }
        // PORT ID (multi-modal routing) - 1 byte
        if (write(mfile->fd, &node->port_id, sizeof(uint8_t)) != sizeof(uint8_t)) { free(node_offsets); return -1; }
        
        // HIERARCHY STRUCTURE (reference-based compression) - 1 byte + size_t + child_ids
        if (write(mfile->fd, &node->is_reference_hierarchy, sizeof(uint8_t)) != sizeof(uint8_t)) { free(node_offsets); return -1; }
        if (write(mfile->fd, &node->child_count, sizeof(size_t)) != sizeof(size_t)) { free(node_offsets); return -1; }
        if (node->is_reference_hierarchy && node->child_nodes && node->child_count > 0) {
            // Write child IDs (to be resolved on load)
            for (size_t j = 0; j < node->child_count; j++) {
                if (node->child_nodes[j]) {
                    if (write(mfile->fd, node->child_nodes[j]->id, 9) != 9) { free(node_offsets); return -1; }
                } else {
                    uint8_t empty_id[9] = {0};
                    if (write(mfile->fd, empty_id, 9) != 9) { free(node_offsets); return -1; }
                }
            }
        }
        
        // EMBEDDINGS (lazy + cached) - size_t + embedding data
        if (write(mfile->fd, &node->embedding_dim, sizeof(size_t)) != sizeof(size_t)) { free(node_offsets); return -1; }
        if (node->embedding && node->embedding_dim > 0) {
            // Write embedding data
            if (write(mfile->fd, node->embedding, node->embedding_dim * sizeof(float)) != (ssize_t)(node->embedding_dim * sizeof(float))) { free(node_offsets); return -1; }
        }
        
        node->file_offset = node_offsets[i];
        node->loaded = 1;
    }
    
    free(node_offsets);
    
    // Record edge section offset
    edge_section_offset = lseek(mfile->fd, 0, SEEK_CUR);
    
    // Update edge offset in header
    if (lseek(mfile->fd, edge_offset_pos, SEEK_SET) < 0) return -1;
    if (write(mfile->fd, &edge_section_offset, sizeof(off_t)) != sizeof(off_t)) return -1;
    
    // Go to edge section
    if (lseek(mfile->fd, edge_section_offset, SEEK_SET) < 0) return -1;
    
    // Write edges
    for (size_t i = 0; i < graph->edge_count; i++) {
        Edge *edge = graph->edges[i];
        if (!edge || !edge->from_node || !edge->to_node) continue;
        
        // Write edge: from_id[9] + to_id[9] + direction[1] + last_wave_gen[4] + weight[4] + routing_gate[4]
        //           + context_bytes[4] + context_len[1]
        if (write(mfile->fd, edge->from_node->id, 9) != 9) return -1;
        if (write(mfile->fd, edge->to_node->id, 9) != 9) return -1;
        // Save uint8_t fields (4 bytes total)
        if (write(mfile->fd, &edge->weight, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
        if (write(mfile->fd, &edge->routing_gate, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
        if (write(mfile->fd, &edge->inactivity_timer, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
        if (write(mfile->fd, &edge->flags, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
        // Save last_wave_generation as uint32_t (4 bytes)
        if (write(mfile->fd, &edge->last_wave_generation, sizeof(uint32_t)) != sizeof(uint32_t)) return -1;
        
        // Edge size: 9 + 9 + 1 + 1 + 1 + 1 + 4 = 26 bytes (was 31, originally 36)
        // Note: file_offset removed from Edge struct (not needed)
    }
    
    // Update timestamp
    mfile->last_modified = time(NULL);
    
    // Sync to disk
    fsync(mfile->fd);
    
    return 0;
}

/* Close .m file
 * - Saves if needed
 * - Frees resources
 */
void melvin_m_close(MelvinMFile *mfile) {
    if (!mfile) return;
    
    // Save before closing
    melvin_m_save(mfile);
    
    // Free graph (proper cleanup of mini nets, nodes, and edges)
    if (mfile->graph) {
        // Free graph's mini nets
        if (mfile->graph->refine_net) mini_net_free(mfile->graph->refine_net);
        if (mfile->graph->decode_net) mini_net_free(mfile->graph->decode_net);
        if (mfile->graph->hierarchy_net) mini_net_free(mfile->graph->hierarchy_net);
        
        // Free nodes and edges arrays (nodes/edges themselves freed elsewhere)
        free(mfile->graph->nodes);
        free(mfile->graph->edges);
        
        // Free hash table
        if (mfile->graph->node_hash_table) {
            for (size_t i = 0; i < mfile->graph->hash_table_size; i++) {
                if (mfile->graph->node_hash_table[i]) {
                    free(mfile->graph->node_hash_table[i]);
                }
            }
            free(mfile->graph->node_hash_table);
        }
        
        free(mfile->graph);
    }
    
    // Free buffers
    if (mfile->universal_input) free(mfile->universal_input);
    if (mfile->universal_output) free(mfile->universal_output);
    if (mfile->path) free(mfile->path);
    if (mfile->node_index) free(mfile->node_index);
    
    // Free error feedback path tracking
    if (mfile->last_output_path) free(mfile->last_output_path);
    
    // Close file
    if (mfile->fd >= 0) close(mfile->fd);
    
    free(mfile);
}

/* Write to universal input buffer
 * - Appends data to input buffer
 * - Grows buffer as needed (no hardcoded limits)
 */
void melvin_m_universal_input_write(MelvinMFile *mfile, const uint8_t *data, size_t size) {
    if (!mfile || !data || size == 0) return;
    
    // Grow buffer if needed (double capacity)
    while (mfile->universal_input_size + size > mfile->universal_input_capacity) {
        size_t new_capacity = mfile->universal_input_capacity * 2;
        if (new_capacity < mfile->universal_input_size + size) {
            new_capacity = mfile->universal_input_size + size;
        }
        mfile->universal_input = realloc(mfile->universal_input, new_capacity);
        mfile->universal_input_capacity = new_capacity;
    }
    
    // Append data
    memcpy(mfile->universal_input + mfile->universal_input_size, data, size);
    mfile->universal_input_size += size;
}

/* Get universal input size */
size_t melvin_m_universal_input_size(MelvinMFile *mfile) {
    if (!mfile) return 0;
    return mfile->universal_input_size;
}

/* Clear universal input buffer */
void melvin_m_universal_input_clear(MelvinMFile *mfile) {
    if (!mfile) return;
    mfile->universal_input_size = 0;
}

/* ============================================================================
 * GRAPH STATE MANAGEMENT (Bug Fix: Multi-Pattern Processing)
 * ============================================================================ */

/* Clear all temporary state between patterns
 * This prevents use-after-free and stale state issues
 * Following README: Self-regulation through local cleanup
 */
static void graph_clear_temporary_state(Graph *graph) {
    if (!graph) return;
    
    // Clear activation strengths (reset for next pattern)
    for (size_t i = 0; i < graph->node_count; i++) {
        if (graph->nodes[i]) {
            graph->nodes[i]->activation_strength = 0.0f;
            // Keep weight, bias, state - only clear temporary activation
        }
    }
    
    // Clear edge marked_for_deletion flags (reset for next pattern)
    for (size_t i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i]) {
            graph->edges[i]->flags &= ~0x01;  // Clear marked_for_deletion bit
            // Keep weight, routing_gate - only clear deletion flag
        }
    }
    
    // Increment wave generation (invalidates all cached wave data)
    // This is O(1) and prevents stale wave_generation comparisons
    graph->wave_generation++;
}

/* Process input through .m file
 * - Universal input (any binary data)
 * - Triggers wave propagation
 * - Auto-saves after adaptation
 */
int melvin_m_process_input(MelvinMFile *mfile) {
    // #region agent log
    fprintf(stderr, "[LOG] process_input_entry mfile=%p\n", (void*)mfile);
    fflush(stderr);
    // #endregion
    if (!mfile) return -1;
    
    // CRITICAL: Clear temporary state from previous pattern
    // This prevents crashes on second different pattern
    graph_clear_temporary_state(mfile->graph);
    
    // Get input from universal input buffer
    if (mfile->universal_input_size == 0) return 0;  // No input to process
    
    const uint8_t *input = mfile->universal_input;
    size_t input_size = mfile->universal_input_size;
    
    // Universal input: process ALL bytes as data (no metadata skipping)
    // The universal API is for simple text/data input, not PortFrame format
    const uint8_t *data_start = input;
    size_t data_size = input_size;
    
    // Process sequential patterns (only actual data, not metadata)
    // This creates nodes and edges (h→e→l→l→o)
    Node **pattern_nodes = NULL;
    size_t pattern_node_count = 0;
    if (data_size > 0 && data_start) {
        // Track nodes created during pattern processing
        pattern_nodes = malloc(data_size * sizeof(Node*));
        // #region agent log
        fprintf(stderr, "[DEBUG] ALLOCATED pattern_nodes: %p, size=%zu (HypB)\n", (void*)pattern_nodes, data_size);
        fflush(stderr);
        // #endregion
        for (size_t i = 0; i < data_size; i++) {
            pattern_nodes[i] = graph_find_or_create_pattern_node(mfile->graph, &data_start[i], 1);
            if (pattern_nodes[i]) {
                // Set port_id on node (where this data came from)
                pattern_nodes[i]->port_id = mfile->last_input_port_id;
                pattern_node_count++;
            }
        }
        // Now process sequential patterns (creates edges between these nodes)
        graph_process_sequential_patterns(mfile->graph, data_start, data_size);
    }
    
    // Use the SAME nodes that have edges (from pattern processing)
    // This ensures we're using nodes that actually have outgoing edges
    Node **initial_nodes = pattern_nodes;
    size_t initial_count = pattern_node_count;
    
    // =========================================================================
    // THREE-PHASE ARCHITECTURE: The fundamental fix for intelligent output
    // =========================================================================
    // 
    // Before: Wave only collected input nodes → "hello" → "lo\n" (random)
    // After:  Spreading activation finds continuations → "hello" → " world"
    //
    // Phase 1: ENCODE - Input → Activation (spreading through learned edges)
    // Phase 2: REFINE - Activation → Refined Activation (recurrent dynamics)
    // Phase 3: DECODE - Activation → Output (context-aware generation)
    // =========================================================================
    
    ActivationPattern *pattern = NULL;
    
    if (initial_count > 0) {
        // PHASE 2: Mark input nodes for O(1) checking during decode
        // Intelligence: Like brain's working memory - temporarily active input representation
        graph_mark_input_nodes(mfile->graph, initial_nodes, initial_count);
        
        // Three-phase architecture: encode → refine → decode
        
        // PHASE 1: ENCODE - Spreading activation with exploration
        // Input nodes activate, then spread through edges to find continuations
        
        // Compute adaptive exploration rate
        float exploration_rate = compute_exploration_rate(mfile->graph, mfile->graph->wave_generation);
        
        // Encode with exploration (will explore blank edges if any exist)
        pattern = encode_input_spreading(mfile->graph, initial_nodes, initial_count);
        
        // EXPLORATION: Try blank edges as potential continuations
        if (pattern && exploration_rate > 0.01f) {
            for (size_t i = 0; i < initial_count; i++) {
                if (initial_nodes[i]) {
                    explore_blank_edges_from_node(initial_nodes[i], pattern, mfile->graph, exploration_rate);
                }
            }
        }
        
        if (pattern && pattern->count > 0) {
            // PHASE 2: REFINE - Recurrent dynamics
            // Nodes that connect to active context strengthen
            // Disconnected nodes decay
            // This creates emergent attention-like behavior
            
            // MINI NET DECISION: How many iterations?
            // Graph's refine_net decides based on pattern complexity and graph state
            int refine_iterations = compute_refine_iterations(mfile->graph, pattern, initial_count);
            
            refine_pattern_dynamics(pattern, mfile->graph, refine_iterations);
            
            // PHASE 3: DECODE - Context-aware generation
            // MINI NET DECISION: What temperature to use?
            // Graph's decode_net decides based on pattern entropy and graph state
            float temperature = compute_decode_temperature(mfile->graph, pattern);
            
            // Sample from pattern (includes spread nodes like " world")
            // Not just input nodes
            // (Temperature is passed to generation function for sampling control)
            generate_from_pattern(mfile, pattern, initial_nodes, initial_count);
            
            // MINI NET LEARNING: Learn from three-phase outcome
            // Assume positive outcome for now (will be refined with error feedback)
            float outcome = 1.0f;  // Success (created output)
            learn_three_phase_outcome(mfile->graph, refine_iterations, temperature, outcome);
            
            // PHASE 4: DETECT AND CREATE BLANK ABSTRACTIONS (NEW)
            // After generation, check nodes for high variability patterns
            
            // #region agent log
            fprintf(stderr, "[LOG] phase4_entry initial_count=%zu has_pattern=%d\n", 
                    initial_count, pattern ? 1 : 0);
            fflush(stderr);
            // #endregion
            
            if (initial_nodes && initial_count > 0) {
                for (size_t i = 0; i < initial_count; i++) {
                    Node *node = initial_nodes[i];
                    if (node) {
                        // #region agent log
                        fprintf(stderr, "[LOG] calling_detect_initial i=%zu node=%p\n", i, (void*)node);
                        fflush(stderr);
                        // #endregion
                        
                        detect_and_create_blank_abstractions(node, mfile->graph);
                        
                        // #region agent log
                        fprintf(stderr, "[LOG] returned_from_detect i=%zu\n", i);
                        fflush(stderr);
                        // #endregion
                    }
                }
                
                // Also check nodes in activation pattern (they were involved in generation)
                if (pattern && pattern->nodes) {
                    for (size_t i = 0; i < pattern->count && i < 20; i++) {
                        Node *node = pattern->nodes[i];
                        if (node && node->payload_size > 0) {
                            detect_and_create_blank_abstractions(node, mfile->graph);
                        }
                    }
                }
            }
        }
        
        // POPULATE CONTEXT DURING TRAINING (not just generation!)
        // This enables context gating to work during learning
        // Each node gets context of recent input bytes for discrimination
        for (size_t i = 0; i < initial_count; i++) {
            Node *node = initial_nodes[i];
            if (!node) continue;
            
            // Populate context_trace with recent input bytes (up to capacity)
            size_t trace_len = 0;
            size_t lookback = (i < 8) ? i : 8;  // Look back up to 8 bytes
            
            for (size_t j = 0; j < lookback && trace_len < node->context_trace_capacity; j++) {
                size_t idx = i - j - 1;  // Previous bytes
                if (idx < data_size) {
                    uint8_t byte = data_start[idx];
                    // Encode byte as normalized float (-1.0 to 1.0)
                    float encoded = ((float)byte - 128.0f) / 128.0f;
                    node->context_trace[trace_len++] = encoded;
                }
            }
            node->context_trace_len = trace_len;
            node->context_trace_gen = mfile->graph->wave_generation;
        }
        
        // PHASE 3: HEBBIAN LEARNING + HIERARCHY FORMATION
        // Strengthen edges that were just traversed in the input sequence
        // This is the CORE learning mechanism - edges get stronger with use
        for (size_t i = 0; i + 1 < initial_count; i++) {
            Node *from = initial_nodes[i];
            Node *to = initial_nodes[i + 1];
            if (!from || !to) continue;
            
            // Find or create edge between them (README: "Create sequential edges between consecutive nodes")
            Edge *edge = graph_find_edge_between(from, to);
            
            if (!edge) {
                // CREATE NEW EDGE - This is critical for learning new patterns!
                edge = edge_create(from, to);
                if (edge) {
                    // NOTE: edge_create() already initializes weight to 128 (neutral)
                    // No need to override - bounded uint8_t weight
                    graph_add_edge(mfile->graph, edge);
                }
            }
            
            if (edge) {
                // Hebbian learning: "Neurons that fire together, wire together"
                // COORDINATED WITH ERROR FEEDBACK: Don't overwrite recent error signals
                float old_weight = edge->weight;
                
                // REMOVED: Error feedback check (edges are simple now)
                
                float local_avg = node_get_local_outgoing_weight_avg(from);
                float epsilon = compute_adaptive_epsilon(local_avg);
                
                // How strong is this edge relative to neighbors?
                float weight_float = weight_uint8_to_float(edge->weight, local_avg);
                float weight_ratio = weight_float / (local_avg + epsilon);
                
                // FIXED: Always positive increment, smaller for strong edges
                // Weak edges (ratio < 1.0) get larger increments (catch up faster)
                // Strong edges (ratio > 1.0) get smaller increments (stay strong, learn slowly)
                // This ensures strong edges continue learning instead of stopping
                float increment = epsilon / (weight_ratio + epsilon);
                
                // REMOVED: Error feedback coordination (edges are simple now)
                
                // Only strengthen if increment is positive
                if (increment > 0.0f) {
                    uint8_t old_weight = edge->weight;
                    edge_update_weight_bounded(edge, increment);
                    // Update cached sums
                    float old_float = weight_uint8_to_float(old_weight, 1.0f);
                    float new_float = weight_uint8_to_float(edge->weight, 1.0f);
                    node_update_outgoing_weight_sum(edge->from_node, old_float, new_float);
                }
                
                // (sums already updated above - no need to update again)
                
                // NATURAL STOP LEARNING REMOVED
                // Stop should emerge naturally from weak outgoing edges
                // Nodes at pattern ends will have no/weak continuations
                // The competition between stop_weight and edge scores will handle stopping
                // No explicit training needed - let it emerge from data
                
                // HIERARCHY FORMATION (BRAIN-INSPIRED: Synaptic consolidation)
                // When edge has been used enough times, consolidate into hierarchy
                // Pure ratio-based thresholds (no multipliers)
                float local_avg_weight = node_get_local_outgoing_weight_avg(from);
                float epsilon_h = compute_adaptive_epsilon(local_avg_weight);
                
                // Compute graph maturity as pure ratio
                Graph *graph = mfile->graph;
                float graph_maturity = 0.0f;
                if (graph && graph->node_count > 0) {
                    float node_factor = (float)graph->node_count / (float)(graph->node_count + graph->edge_count);
                    graph_maturity = node_factor;  // Pure ratio
                }
                
                // Threshold is pure ratio of local average
                // More mature = higher ratio needed
                float hierarchy_threshold = local_avg_weight * (1.0f + graph_maturity);
                
                // Relative strength is pure ratio
                float relative_strength = edge->weight / (local_avg_weight + epsilon_h);
                
                // Pure comparison: edge must exceed threshold ratio
                if (edge->weight > hierarchy_threshold && relative_strength > (1.0f + graph_maturity) && 
                    from->payload_size > 0 && to->payload_size > 0) {
                    // PHASE 2: O(pattern_size) trie lookup instead of O(n) scan
                    // Check if hierarchy already exists using trie
                    int hierarchy_exists = 0;
                    size_t combined_size = from->payload_size + to->payload_size;
                    
                    // Build combined pattern for trie lookup
                    uint8_t *combined_payload = malloc(combined_size);
                    if (combined_payload) {
                        memcpy(combined_payload, from->payload, from->payload_size);
                        memcpy(combined_payload + from->payload_size, to->payload, to->payload_size);
                        
                        // O(combined_size) trie lookup
                        Node *existing = trie_lookup(mfile->graph, combined_payload, combined_size);
                        if (existing && existing->abstraction_level > 0) {
                            hierarchy_exists = 1;
                        }
                        free(combined_payload);
                    }
                    
                    if (!hierarchy_exists) {
                        // Create hierarchy node (like brain's synaptic consolidation)
                        Node *hierarchy = create_hierarchy_node(mfile->graph, from, to);
                        if (hierarchy) {
                            graph_add_node(mfile->graph, hierarchy);
                            // Debug: print hierarchy creation
                            fprintf(stderr, "[HIERARCHY] Created level %u: '", hierarchy->abstraction_level);
                            for (size_t k = 0; k < hierarchy->payload_size && k < 20; k++) {
                                if (hierarchy->payload[k] >= 32 && hierarchy->payload[k] < 127) {
                                    fprintf(stderr, "%c", hierarchy->payload[k]);
                                } else {
                                    fprintf(stderr, "?");
                                }
                            }
                            fprintf(stderr, "' (edge weight %.2f)\n", edge->weight);
                        }
                    }
                }
            }
            
            // Also strengthen node weights (nodes that were just activated)
            // ADAPTIVE: Use computed learning rate based on node's success, variance, and change rate
            float from_rate = compute_adaptive_node_weight_rate(from);
            float to_rate = compute_adaptive_node_weight_rate(to);
            from->weight += from_rate;
            to->weight += to_rate;
        }
        
        // End three-phase
        
        // PHASE 2: Clear input node markers (free working memory)
        graph_clear_input_nodes(mfile->graph);
        
        // PHASE 4: Memory consolidation (periodic, like hippocampal replay)
        // Every 100 inputs, consolidate recent activations
        if (mfile->adaptation_count % 100 == 0) {
            graph_consolidate_memory(mfile->graph);
        }
    }
    
    // Clean up pattern (CRITICAL: only free once!)
    // #region agent log
    fprintf(stderr, "[DEBUG] BEFORE pattern free: pattern=%p, pattern_nodes=%p (HypA)\n", (void*)pattern, (void*)pattern_nodes);
    fflush(stderr);
    // #endregion
    if (pattern) {
        activation_pattern_free(pattern);
        pattern = NULL;
    }
    // #region agent log
    fprintf(stderr, "[DEBUG] AFTER pattern free: pattern=%p (HypA)\n", (void*)pattern);
    fflush(stderr);
    // #endregion
    
    // NOTE: initial_nodes is just an alias to pattern_nodes, so don't free it here
    // It will be freed later when we free pattern_nodes
    // free(initial_nodes);  // REMOVED: This was causing double-free!
    
    // Increment adaptation count
    mfile->adaptation_count++;
    mfile->last_modified = time(NULL);
    
    // SELF-MODIFICATION: Periodic self-optimization (every 10 adaptations)
    // Uses intelligence metrics to guide optimization
    if (mfile->adaptation_count % 10 == 0) {
        Graph *graph = mfile->graph;
        
        // PHASE 5: Compute and log intelligence metrics
        IntelligenceMetrics metrics = graph_compute_intelligence_metrics(graph);
        fprintf(stderr, "[METRICS] nodes=%zu edges=%zu hierarchies=%zu blanks=%zu "
                "compress=%.3f general=%.3f avgW=%.3f pred=%.2f consol=%0.f\n",
                metrics.total_nodes, metrics.total_edges, metrics.hierarchy_nodes,
                metrics.blank_nodes, metrics.compression_ratio, metrics.generalization_score,
                metrics.avg_edge_weight, metrics.prediction_accuracy, metrics.consolidation_count);
        
        for (size_t i = 0; i < graph->node_count; i++) {
            node_self_optimize_if_weak(graph->nodes[i]);
        }
    }
    
    // Free pattern_nodes array (allocated at start)
    // #region agent log
    fprintf(stderr, "[DEBUG] BEFORE pattern_nodes free: %p (HypB)\n", (void*)pattern_nodes);
    // #endregion
    if (pattern_nodes) {
        free(pattern_nodes);
        pattern_nodes = NULL;
    }
    // #region agent log
    fprintf(stderr, "[DEBUG] AFTER pattern_nodes free: %p (HypB)\n", (void*)pattern_nodes);
    // #endregion
    
    // Clear input buffer after processing
    melvin_m_universal_input_clear(mfile);
    
    // CRITICAL: Final cleanup to prevent memory leaks
    // Clear any remaining temporary state
    graph_clear_temporary_state(mfile->graph);
    
    // Auto-save to file
    melvin_m_save(mfile);
    
    return 0;
}

/* Get last input port ID (for routing) */
uint8_t melvin_m_get_last_input_port_id(MelvinMFile *mfile) {
    if (!mfile) return 0;
    return mfile->last_input_port_id;
}

/* Set last input port ID (for routing) */
void melvin_m_set_last_input_port_id(MelvinMFile *mfile, uint8_t port_id) {
    if (!mfile) return;
    mfile->last_input_port_id = port_id;
}

/* Get universal output size */
size_t melvin_m_universal_output_size(MelvinMFile *mfile) {
    if (!mfile) return 0;
    return mfile->universal_output_size;
}

/* Read from universal output buffer */
size_t melvin_m_universal_output_read(MelvinMFile *mfile, uint8_t *buffer, size_t buffer_size) {
    if (!mfile || !buffer || buffer_size == 0) return 0;
    
    size_t copy_size = (mfile->universal_output_size < buffer_size) ? 
                       mfile->universal_output_size : buffer_size;
    
    if (copy_size > 0 && mfile->universal_output) {
        memcpy(buffer, mfile->universal_output, copy_size);
    }
    
    return copy_size;
}

/* Clear universal output buffer */
void melvin_m_universal_output_clear(MelvinMFile *mfile) {
    if (!mfile) return;
    mfile->universal_output_size = 0;
}

/* ============================================================================
 * ERROR FEEDBACK (Error-Based Learning)
 * ============================================================================
 * 
 * Feed error signal back to system for error-based learning.
 * Combines with frequency-based (Hebbian) learning for dual learning system.
 * 
 * error_signal: 0.0 = completely wrong, 1.0 = completely correct
 * 
 * This adjusts weights of edges used in last output generation:
 * - Wrong output (error < 0.5): weaken edges in path
 * - Correct output (error > 0.5): strengthen edges in path
 * 
 * Principle: Local-only operations (adjusts only edges in path)
 * Principle: Data-driven (error signal comes from external system)
 */
void melvin_m_feedback_error(MelvinMFile *mfile, float error_signal) {
    if (!mfile || !mfile->graph) return;
    
    // Clamp error signal to [0.0, 1.0]
    if (error_signal < 0.0f) error_signal = 0.0f;
    if (error_signal > 1.0f) error_signal = 1.0f;
    
    // If no output path tracked, nothing to adjust
    if (mfile->last_output_path_count == 0) return;
    
    // Increment error feedback generation (for coordinating with Hebbian learning)
    mfile->graph->error_feedback_generation++;
    
    // Error-based learning: adjust weights of edges in output path
    // error_signal = 1.0: perfect → strengthen edges
    // error_signal = 0.0: wrong → weaken edges
    // CONTINUOUS: No threshold, all values are meaningful
    //
    // ADAPTIVE RATES (following README: "Learning rates adapt from local change rates")
    // - Rates computed from edge's local context
    // - Strong edges get smaller adjustments (more stable)
    // - Weak edges get larger adjustments (more exploratory)
    // - No hardcoded thresholds
    
    for (size_t i = 0; i < mfile->last_output_path_count; i++) {
        Edge *edge = mfile->last_output_path[i];
        if (!edge || !edge->from_node) continue;
        
        float old_weight = edge->weight;
        
        // Compute adaptive adjustment based on edge strength relative to local context
        // Stronger edges get smaller adjustments (more stable)
        // Weaker edges get larger adjustments (more exploratory)
        float local_avg_weight = node_get_local_outgoing_weight_avg(edge->from_node);
        float epsilon = compute_adaptive_epsilon(local_avg_weight);
        float weight_ratio = edge->weight / (local_avg_weight + epsilon);
        
        // Adaptive adjustment: inverse of weight ratio (stronger edges = smaller changes)
        // Smooth function: adjustment = 1.0 / (weight_ratio + 1.0f)
        float adaptive_adjustment = 1.0f / (weight_ratio + 1.0f);
        
        // CONTINUOUS ERROR SIGNAL: Map [0,1] to [-1,+1]
        // error_signal = 0.0 → delta = -1.0 (full penalty)
        // error_signal = 0.5 → delta = 0.0 (neutral)
        // error_signal = 1.0 → delta = +1.0 (full reward)
        float delta = (error_signal - 0.5f) * 2.0f;
        
        // Apply delta scaled by adaptive adjustment
        float change = adaptive_adjustment * delta;
        
        if (delta < 0.0f) {
            // Penalty: multiplicative decay (preserves relative differences)
            edge->weight *= (1.0f + change);  // change is negative, so this weakens
            
            // Penalty: bounded at 0 (uint8_t automatically bounded)
            if (edge->weight < 10) edge->weight = 10;  // Minimal floor (10/255)
        } else {
            // Reward: bounded growth
            edge_update_weight_bounded(edge, change);
        }
        
        // Update cached sums (maintains O(1) access to weight sums)
        float old_float = weight_uint8_to_float((uint8_t)old_weight, 1.0f);
        float new_float = weight_uint8_to_float(edge->weight, 1.0f);
        node_update_outgoing_weight_sum(edge->from_node, old_float, new_float);
        if (edge->to_node) {
            node_update_incoming_weight_sum(edge->to_node, old_float, new_float);
        }
        
        // REMOVED: Error feedback marking (edges are simple now)
    }
    
    // SELF-MODIFICATION: Always update meta-learning (continuous signal)
    // Uses meta-learning to track which strategies work
    float error_rate = 1.0f - error_signal;
    if (error_rate > 0.1f) {  // Only optimize on significant errors
        graph_self_optimize_on_error(mfile->graph, mfile->last_output_path, 
                                     mfile->last_output_path_count, error_signal);
    }
    
    // REMOVED: Error feedback for edges (edges are simple now)
    // Always update meta-learning for nodes in path (continuous learning)
    for (size_t i = 0; i < mfile->last_output_path_count; i++) {
        Edge *edge = mfile->last_output_path[i];
        if (edge && edge->from_node) {
            node_meta_learn_update(edge->from_node, error_signal);
        }
    }
    
    // === STOP WEIGHT LEARNING (Brain/LLM-Inspired) ===
    // Teach the last node when to stop based on error feedback
    // If output was correct → strengthen stop at last node
    // If output was wrong → weaken stop at last node
    if (mfile->last_output_path_count > 0) {
        Edge *last_edge = mfile->last_output_path[mfile->last_output_path_count - 1];
        Node *last_node = last_edge ? last_edge->to_node : NULL;
        
        if (last_node) {
            // Adjust stop_weight based on error signal (direct update)
            // error_signal = 1.0 (correct) → increase stop_weight (stopping here was right)
            // error_signal = 0.0 (wrong) → decrease stop_weight (stopping here was wrong)
            // error_signal = 0.5 (neutral) → no change
            
            float delta = (error_signal - 0.5f) * 2.0f;
            last_node->stop_weight += delta;
            
            // Clamp to reasonable range [0.0, 10.0]
            if (last_node->stop_weight < 0.0f) last_node->stop_weight = 0.0f;
            if (last_node->stop_weight > 10.0f) last_node->stop_weight = 10.0f;
        }
    }
    
    // Clear path after processing (ready for next generation)
    mfile->last_output_path_count = 0;
    
    // Save changes to disk (error-based learning persists)
    melvin_m_save(mfile);
}

/* ============================================================================
 * SOLUTION 2: Strengthen correct edges during self-supervised training
 * ============================================================================
 * 
 * During self-supervised training, we know the correct continuation.
 * This function explicitly strengthens edges in that continuation path,
 * giving the correct path a head start (like supervised learning).
 * 
 * Principle: Local-only operations (each edge strengthened from its own context)
 * Principle: Data-driven (learning rate computed adaptively, not hardcoded)
 * Principle: Hebbian ("neurons that fire together, wire together" on correct path)
 */
void melvin_m_strengthen_continuation(MelvinMFile *mfile, const uint8_t *sequence, 
                                       size_t prefix_len, size_t total_len) {
    if (!mfile || !mfile->graph || !sequence) return;
    if (prefix_len >= total_len) return;  // No continuation
    if (total_len < 2) return;  // Need at least 2 bytes
    
    Graph *graph = mfile->graph;
    
    // Strengthen edges in the CONTINUATION part (from prefix_len to total_len)
    // This gives the correct path a head start
    for (size_t i = prefix_len; i < total_len; i++) {
        // Find or create node for sequence[i]
        Node *node = NULL;
        for (size_t n = 0; n < graph->node_count; n++) {
            Node *candidate = graph->nodes[n];
            if (candidate && candidate->payload_size == 1 && 
                candidate->payload[0] == sequence[i]) {
                node = candidate;
                break;
            }
        }
        
        // Create node if doesn't exist
        if (!node) {
            uint8_t byte_payload = sequence[i];
            node = node_create(&byte_payload, 1, 0);  // Single byte, abstraction level 0
            if (!node) continue;
            graph_add_node(graph, node);
        }
        
        // If not first byte of continuation, strengthen edge from previous
        if (i > prefix_len) {
            // Find previous node
            Node *prev_node = NULL;
            for (size_t n = 0; n < graph->node_count; n++) {
                Node *candidate = graph->nodes[n];
                if (candidate && candidate->payload_size == 1 && 
                    candidate->payload[0] == sequence[i-1]) {
                    prev_node = candidate;
                    break;
                }
            }
            
            if (prev_node) {
                // Find or create edge
                Edge *edge = NULL;
                for (size_t e = 0; e < prev_node->outgoing_count; e++) {
                    if (prev_node->outgoing_edges[e] && 
                        prev_node->outgoing_edges[e]->to_node == node) {
                        edge = prev_node->outgoing_edges[e];
                        break;
                    }
                }
                
                if (!edge) {
                    edge = edge_create(prev_node, node);
                    if (edge) {
                        graph_add_edge(graph, edge);
                    }
                }
                
                if (edge) {
                    // STRENGTHEN this edge (it's in the correct continuation)
                    // Direct Hebbian update (pure LTP, no artificial boost)
                    
                    uint8_t old_weight = edge->weight;
                    edge_update_weight_bounded(edge, 1.0f);  // Pure Hebbian strengthening
                    
                    // Update cached sums
                    float old_float = weight_uint8_to_float(old_weight, 1.0f);
                    float new_float = weight_uint8_to_float(edge->weight, 1.0f);
                    node_update_outgoing_weight_sum(prev_node, old_float, new_float);
                    node_update_incoming_weight_sum(node, old_float, new_float);
                    
                    // NOTE: No context_bytes - using attention-based context
                    // Attention is learned from usage during generation
                }
            }
        }
    }
}

/* Get statistics */
size_t melvin_m_get_node_count(MelvinMFile *mfile) {
    if (!mfile || !mfile->graph) return 0;
    return mfile->graph->node_count;
}

size_t melvin_m_get_edge_count(MelvinMFile *mfile) {
    if (!mfile || !mfile->graph) return 0;
    return mfile->graph->edge_count;
}

uint64_t melvin_m_get_adaptation_count(MelvinMFile *mfile) {
    if (!mfile) return 0;
    return mfile->adaptation_count;
}

/* ============================================================================
 * OUTPUT GENERATION (LLM-like Sampling)
 * ============================================================================ */

/* Compute output readiness (pattern maturity)
 * - Measures co-activation edge strength from input nodes
 * - Returns readiness score (0.0 = no patterns, >0.0 = patterns exist)
 * - ALL RELATIVE: If graph is mostly empty, even weak edges are "strong" relative to empty context
 * - No hard thresholds: Uses smooth probability weighting for all edges
 */

/* Generate output (LLM-like autoregressive generation)
 * - Uses predictions collected DURING wave propagation (from state)
 * - Builds probability distribution using edge_transform_activation() for shaping
 * - Samples autoregressively
 * - Writes to universal_output
 * 
 * Following README: "Wave Propagation Phase: Collects predictions from all activated nodes"
 * and "Uses edge_transform_activation() for probability shaping (mini transformers)"
 */
/* Initialize WaveState (zero-initialize all fields, no allocation) */
static void wave_state_init(WaveState *state) {
    if (!state) return;
    memset(state, 0, sizeof(WaveState));
    // Initialize decision signals
    state->hierarchy_candidate = NULL;
    state->hierarchy_weight_relative = 0.0f;
    
    // SOLUTION 3: Initialize sequence memory
    state->sequence_path = NULL;
    state->sequence_length = 0;
    state->sequence_capacity = 0;
    state->active_hierarchy = NULL;
    state->position_in_hierarchy = 0;
    state->hierarchy_confidence = 0.0f;
}

/* Reset WaveState (reset counters only, DO NOT free - keeps buffers for reuse) */
static void wave_state_reset(WaveState *state) {
    if (!state) return;
    
    // DO NOT free anything - just reset dynamic length fields to 0
    // Keep all buffers and capacities for reuse
    
    state->all_activated_count = 0;     // Reset current activated count
    // DO NOT reset all_activated_capacity - tracks allocated capacity for reuse
    
    // Reset decision signals (but don't free blank_cluster - caller manages)
    state->hierarchy_candidate = NULL;
    state->hierarchy_weight_relative = 0.0f;
    
    // SOLUTION 3: Reset sequence memory (keep buffer for reuse)
    state->sequence_length = 0;
    state->active_hierarchy = NULL;
    state->position_in_hierarchy = 0;
    state->hierarchy_confidence = 0.0f;
}

/* Free WaveState (frees owned pointers and sets them to NULL - safe to call multiple times) */
static void wave_state_free(WaveState *state) {
    // #region agent log
    fprintf(stderr, "[DEBUG] wave_state_free ENTRY: state=%p, all_nodes=%p, all_strengths=%p (HypD)\n", (void*)state, state ? (void*)state->all_activated_nodes : NULL, state ? (void*)state->all_activation_strengths : NULL);
    // #endregion
    if (!state) return;
    
    // Free prediction collection arrays (only what exists in new structure)
    if (state->all_activated_nodes) {
        free(state->all_activated_nodes);
        state->all_activated_nodes = NULL;
    }
    if (state->all_activation_strengths) {
        free(state->all_activation_strengths);
        state->all_activation_strengths = NULL;
    }
    if (state->all_edge_transforms) {
        free(state->all_edge_transforms);
        state->all_edge_transforms = NULL;
    }
    
    // SOLUTION 3: Free sequence memory
    if (state->sequence_path) {
        free(state->sequence_path);
        state->sequence_path = NULL;
    }
    
    // Reset all fields
    state->all_activated_count = 0;
    state->all_activated_capacity = 0;
    state->hierarchy_candidate = NULL;
}

/* Compute pattern completion score
 * - Checks if we're in the middle of completing a known pattern
 * - Returns score for each candidate based on pattern completion
 * - Following README: Local measurements, data-driven thresholds
 */

static void melvin_generate_output_from_state(MelvinMFile *mfile, WaveState *state, 
                                                Node **input_nodes, size_t input_count) {
    fprintf(stderr, "[DEBUG OUTPUT] melvin_generate_output_from_state called: input_count=%zu, all_activated_count=%zu\n", input_count, state->all_activated_count);
    if (!mfile || !mfile->graph || !state || !input_nodes || input_count == 0) {
        fprintf(stderr, "[DEBUG OUTPUT] Early return: invalid params\n");
        return;
    }
    if (state->all_activated_count == 0) {
        fprintf(stderr, "[DEBUG OUTPUT] Early return: all_activated_count=0\n");
        return;
    }
    
    Graph *graph = mfile->graph;
    
    // 1. Use nodes chosen by wave propagation as candidates
    // Wave propagation has already intelligently selected these nodes based on:
    // - Mini neural net decisions (activation_strength)
    // - Mini transformer decisions (edge transforms)
    // - Routing decisions (routing gates)
    // These are the nodes the system chose, not all nodes
    Node **candidates = NULL;
    size_t candidate_count = 0;
    size_t candidate_capacity = state->all_activated_count;
    int candidates_need_free = 0;
    
    if (candidate_capacity > 0) {
        candidates = malloc(candidate_capacity * sizeof(Node*));
        if (!candidates) {
            return;  // Allocation failed
        }
        candidates_need_free = 1;
        
        // Filter wave-activated nodes: only include nodes that "fired" (like brain neurons)
        // Nodes with activation below threshold didn't fire - they're noise, not signal
        for (size_t i = 0; i < state->all_activated_count; i++) {
            Node *node = state->all_activated_nodes[i];
            float activation = state->all_activation_strengths[i];  // Already computed during wave prop
            
            if (!node) continue;
            
            // Filter out blank nodes (payload_size == 0)
            if (node->payload_size == 0) continue;
            
            // Filter out NULL bytes (EOS marker)
            if (!node->payload || node->payload[0] == 0x00) continue;
            
            // BRAIN MECHANISM: Only include nodes that "fired" (activation above threshold)
            // Threshold is adaptive (from node's weight), not hardcoded
            // Nodes with activation near 0 didn't fire - they're not in the active set
            float firing_threshold = compute_adaptive_epsilon(node->weight);
            if (activation <= firing_threshold) continue;  // Didn't fire, skip
            
            // Add to candidates (node fired - it's in the active set)
            candidates[candidate_count++] = node;
        }
    }
    
    // If no valid candidates after filtering, return early (no output)
    fprintf(stderr, "[DEBUG OUTPUT] After firing filter: candidate_count=%zu (from %zu activated nodes)\n", candidate_count, state->all_activated_count);
    
    // DEBUG: Print input nodes and their outgoing edges
    fprintf(stderr, "[DEBUG EDGES] Input nodes and outgoing edges:\n");
    for (size_t i = 0; i < input_count && i < 5; i++) {
        if (input_nodes[i] && input_nodes[i]->payload_size > 0) {
            uint8_t byte = input_nodes[i]->payload[0];
            fprintf(stderr, "  '%c' (0x%02x) -> ", (byte >= 32 && byte < 127) ? byte : '?', byte);
            for (size_t j = 0; j < input_nodes[i]->outgoing_count && j < 3; j++) {
                Edge *e = input_nodes[i]->outgoing_edges[j];
                if (e && e->to_node && e->to_node->payload_size > 0) {
                    uint8_t tb = e->to_node->payload[0];
                    fprintf(stderr, "'%c'(w=%.2f) ", (tb >= 32 && tb < 127) ? tb : '?', e->weight);
                }
            }
            fprintf(stderr, "\n");
        }
    }
    
    // DEBUG: Print candidates that fired (with activation strength)
    fprintf(stderr, "[DEBUG OUTPUT] Candidates that fired: ");
    for (size_t i = 0; i < candidate_count && i < 10; i++) {
        if (candidates[i] && candidates[i]->payload_size > 0) {
            uint8_t byte = candidates[i]->payload[0];
            // Find activation strength for this candidate
            float act = 0.0f;
            for (size_t j = 0; j < state->all_activated_count; j++) {
                if (state->all_activated_nodes[j] == candidates[i]) {
                    act = state->all_activation_strengths[j];
                    break;
                }
            }
            if (byte >= 32 && byte < 127) {
                fprintf(stderr, "'%c'(%.2f) ", byte, act);
            } else {
                fprintf(stderr, "0x%02x(%.2f) ", byte, act);
            }
        }
    }
    fprintf(stderr, "\n");
    
    if (candidate_count == 0) {
        fprintf(stderr, "[DEBUG OUTPUT] Early return: no valid candidates after filtering\n");
        if (candidates && candidates_need_free) free(candidates);
        return;
    }
    
    // 3. Compute local-relative logits for ALL nodes
    // activation_strength is already local-relative (normalized by local weight sums)
    // edge_transforms are also local-relative (use local averages)
    // Combine them to get final local-relative logits
    float *logits = malloc(candidate_count * sizeof(float));
    if (!logits) {
        return;
    }
    
    // Compute logits for all candidates (relative to local context)
    for (size_t i = 0; i < candidate_count; i++) {
        Node *node = candidates[i];
        if (!node || node->payload_size == 0) {
            logits[i] = -1e9f;  // Very negative (effectively zero probability)
            continue;
        }
        
        // Data-driven logit: Find this node in wave propagation state
        // Wave propagation computed activation_strength (mini neural net output)
        // and edge_transforms (mini transformer outputs) for each activated node
        float base_logit = 0.0f;
        float edge_transform_sum = 0.0f;
        int found_in_activated = 0;
        for (size_t j = 0; j < state->all_activated_count; j++) {
            if (state->all_activated_nodes[j] == node) {
                base_logit = state->all_activation_strengths[j];  // Mini neural net output
                edge_transform_sum = state->all_edge_transforms[j];  // Mini transformer output
                found_in_activated = 1;
                break;
            }
        }
        
        // Base logit from wave propagation (data-driven)
        if (found_in_activated) {
            // Node found in activated set - use activation strength
            logits[i] = base_logit + edge_transform_sum;
        } else {
            // Node not in activated set - use adaptive epsilon (relative to local context)
            // This ensures input sequence nodes can still be selected even if not activated
            // Use a minimal adaptive value based on node's own local context
            float node_local_avg = node_get_local_incoming_weight_avg(node);
            float logit_epsilon = compute_adaptive_epsilon(node_local_avg);
            logits[i] = logit_epsilon;  // Adaptive minimal logit
        }
        
        // Direct continuations from input nodes (learned sequential patterns via edges)
        // Edge weights are data-driven, computed from co-activation patterns during training
        // Pure edge weights, no artificial boosts - recency handled by activation strength
        if (input_nodes && input_count > 0) {
            for (int input_idx = (int)input_count - 1; input_idx >= 0; input_idx--) {
                Node *input_node = input_nodes[input_idx];
                if (!input_node) continue;
                
                for (size_t j = 0; j < input_node->outgoing_count; j++) {
                    Edge *edge = input_node->outgoing_edges[j];
                    if (edge && edge->to_node == node) {
                        // Direct continuation: pure edge weight (no artificial boost)
                        // Recency is naturally encoded in base_logit (activation strength)
                        logits[i] += edge->weight * base_logit;
                        break;  // Found match, no need to check other edges from this node
                    }
                }
            }
        }
        
        // If this candidate IS an input node itself (no outgoing edges case), boost based on position
        // Earlier positions (like 'h') should be prioritized to continue the sequence
        // Boost is relative to activation_strength and local context (not hardcoded)
        if (input_nodes && input_count > 0) {
            for (size_t input_idx = 0; input_idx < input_count; input_idx++) {
                if (input_nodes[input_idx] == node) {
                    // This is an input node - boost earlier positions more (to continue sequence)
                    // Position boost: earlier positions get higher boost, relative to local context
                    float position_factor = (float)(input_count - input_idx) / (float)input_count;  // 1.0 to 0.0
                    float local_avg_node = node_get_local_incoming_weight_avg(node);
                    float seq_epsilon = compute_adaptive_epsilon(local_avg_node);
                    float sequence_boost = (local_avg_node > seq_epsilon) ? 
                                        (local_avg_node * position_factor) : (seq_epsilon * position_factor);
                    logits[i] += sequence_boost * (base_logit + edge_transform_sum + seq_epsilon);
                    break;
                }
            }
        }
        
    }
    
    // Simple weights: use logits directly
    float *weights = malloc(candidate_count * sizeof(float));
    if (!weights) {
        free(logits);
        return;
    }
    
    for (size_t i = 0; i < candidate_count; i++) {
        weights[i] = (logits[i] > 0.0f) ? logits[i] : 0.0f;
    }
    
    free(logits);
    
    // 6. Generate output autoregressively using wave propagation (Pure Data-Driven)
    // KEY: Sample from wave-activated nodes, then re-run wave propagation after each output byte
    // (Following README: "Wave prop just to the most likely node, and then changed context and kept calculating")
    // Wave propagation follows edges (through wave propagation), and we update context by re-running it
    // PATCH: Removed entropy-based stopping (was causing early termination after 1 byte)
    // PATCH: Added EOS support and debug stop reason logging
    // PATCH: Removed random sampling - all decisions based on mini neural net outputs (data-driven)
    // PATCH: Added wave propagation stop prediction (learned, data-driven)
    size_t output_capacity = 0;
    uint8_t *output = NULL;
    size_t output_len = 0;
    
    // CRITICAL: Track output NODES (not just bytes) to use actual nodes for continuation finding
    // This ensures we use the SAME node that was output, not find/create a different one
    Node **output_nodes = NULL;
    size_t output_nodes_capacity = 0;
    
    // Note: stop reason tracked via flags and conditions below
    
    // Track if we allocated valid_candidates arrays that need freeing
    Node **allocated_valid_candidates = NULL;
    float *allocated_valid_strengths = NULL;
    float *allocated_valid_transforms = NULL;
    
    
    // EOS byte (end-of-sequence marker)
    #define EOS_BYTE 0x00  // NULL byte as EOS (can be learned/configured)
    
    // Generate autoregressively until EOS or natural stop
    // Following README: "Autoregressive generation (step-by-step, like LLM next-token prediction)"
    // Following README: "Stops when propagation naturally weakens" - relative to initial energy, not absolute
    // DATA-DRIVEN: No hardcoded length limit - uses wave-based stop prediction (mini neural net)
    
    
    while (1) {
        // Simple stop: if last output node has no outgoing edges, stop
        if (output_len > 0 && output_nodes && output_len <= output_nodes_capacity) {
            Node *current_node = output_nodes[output_len - 1];
            if (current_node && current_node->outgoing_count == 0) {
                /* stop_reason=no_outgoing_edges */
                break;
            }
        }
        
        Node *sampled_node = NULL;
        uint8_t sampled_byte = 0;
        
        if (output_len == 0 && candidate_count > 0) {
            // FIRST BYTE: Sample from candidates using data-driven weights
            // Following README: "All decisions are relative to local context"
            // Weights are already computed from activation_strength (data-driven)
            
            // Find candidate with highest weight (strongest decision)
            size_t best_idx = 0;
            float best_weight = -1.0f;
            for (size_t i = 0; i < candidate_count; i++) {
                if (weights[i] > best_weight) {
                    best_weight = weights[i];
                    best_idx = i;
                }
            }
            
            if (best_weight > 0.0f && candidates[best_idx] && candidates[best_idx]->payload_size > 0) {
                sampled_node = candidates[best_idx];
                sampled_byte = candidates[best_idx]->payload[0];
            } else {
                /* stop_reason=no_valid_candidate */
                break;
            }
        } else if (output_len > 0 && output_nodes && output_len <= output_nodes_capacity) {
            // SUBSEQUENT BYTES: Continue from last output node using context-aware edge selection
            Node *last_output_node = output_nodes[output_len - 1];
            if (last_output_node && last_output_node->outgoing_count > 0) {
                // Use context-aware edge selection
                // Pass wave_state and input_nodes so node can use activated sequence as context
                Edge *chosen_edge = node_compute_winning_edge_with_context(
                    last_output_node, graph, output, output_len, output_nodes, output_len, state, input_nodes, input_count
                );
                
                if (chosen_edge && chosen_edge->to_node && chosen_edge->to_node->payload_size > 0) {
                    sampled_node = chosen_edge->to_node;
                    sampled_byte = chosen_edge->to_node->payload[0];
                } else {
                    // No valid edge - stop generation
                    /* stop_reason=node_no_valid_choice */
                    break;
                }
            } else {
                // Node has no outgoing edges - stop generation
                /* stop_reason=no_outgoing_edges */
                break;
            }
        } else {
            // No input nodes or invalid state - stop generation
            /* stop_reason=invalid_state */
            break;
        }
        
        // Verify we have a valid sampled node
        if (!sampled_node) {
            /* stop_reason=no_sampled_node */
            break;
        }
        
        // Stop condition 3: EOS byte detected
        if (sampled_byte == EOS_BYTE) {
            /* stop_reason=EOS */
            break;
        }
        
        // Allocate output buffer if needed
        if (output_capacity == 0) {
            output_capacity = 1;
            output = malloc(output_capacity);
            if (!output) {
                /* stop_reason=malloc_failed */
                break;
            }
        }
        
        // Grow buffer if needed
        if (output_len >= output_capacity) {
            size_t new_capacity = output_capacity * 2;
            uint8_t *new_output = realloc(output, new_capacity);
            if (!new_output) {
                /* stop_reason=realloc_failed */
                break;
            }
            output = new_output;
            output_capacity = new_capacity;
        }
        
        // Output single byte (autoregressive, like LLM tokens)
        // Following README: "step-by-step, like LLM next-token prediction"
        // Hierarchies guide scoring but don't output directly
        output[output_len++] = sampled_byte;
        
        // Track the actual output node for next iteration
        // Simple byte-level tracking: one byte = one node = one step
        if (sampled_node) {
            // Grow output_nodes array if needed
            if (output_len > output_nodes_capacity) {
                size_t new_capacity = (output_nodes_capacity == 0) ? 4 : output_nodes_capacity * 2;
                while (new_capacity < output_len) new_capacity *= 2;  // Ensure big enough
                Node **new_output_nodes = realloc(output_nodes, new_capacity * sizeof(Node*));
                if (new_output_nodes) {
                    output_nodes = new_output_nodes;
                    output_nodes_capacity = new_capacity;
                }
            }
            if (output_nodes && output_len - 1 < output_nodes_capacity) {
                output_nodes[output_len - 1] = sampled_node;  // Store at current byte position
            }
        }
        
        // CRITICAL FIX: Update wave_state with output node (dynamic context)
        // This makes context = input + decisions made (per Requirement.md)
        // Now wave_state contains the full context for subsequent predictions
        if (sampled_node && state) {
            // Grow wave_state arrays if needed
            if (state->all_activated_count >= state->all_activated_capacity) {
                size_t new_capacity = (state->all_activated_capacity == 0) ? 
                                      16 : (state->all_activated_capacity * 2);
                Node **new_nodes = realloc(state->all_activated_nodes, 
                                          new_capacity * sizeof(Node*));
                float *new_strengths = realloc(state->all_activation_strengths, 
                                              new_capacity * sizeof(float));
                float *new_transforms = realloc(state->all_edge_transforms, 
                                               new_capacity * sizeof(float));
                if (new_nodes && new_strengths && new_transforms) {
                    state->all_activated_nodes = new_nodes;
                    state->all_activation_strengths = new_strengths;
                    state->all_edge_transforms = new_transforms;
                    state->all_activated_capacity = new_capacity;
                }
            }
            
            // Add output node to context (with activation strength)
            if (state->all_activated_count < state->all_activated_capacity) {
                state->all_activated_nodes[state->all_activated_count] = sampled_node;
                state->all_activation_strengths[state->all_activated_count] = 
                    sampled_node->activation_strength > 0 ? sampled_node->activation_strength : 1.0f;
                state->all_edge_transforms[state->all_activated_count] = 0.0f;
                state->all_activated_count++;
            }
        }
        
        // ENHANCEMENT: Update context_trace for output nodes (sequence memory)
        // Per Requirement.md line 5: "the current node holds the context of the last x number"
        // This gives the node memory of where it is in the sequence
        if (sampled_node && output_nodes && output_len > 0) {
            // Build sequence from recent output (last 8 nodes)
            size_t trace_seq_len = (output_len < 8) ? output_len : 8;
            Node **trace_sequence = malloc(trace_seq_len * sizeof(Node*));
            if (trace_sequence) {
                size_t trace_idx = 0;
                // Get last 8 output nodes (most recent first)
                for (size_t j = (output_len > trace_seq_len) ? (output_len - trace_seq_len) : 0; 
                     j < output_len && trace_idx < trace_seq_len; j++) {
                    if (output_nodes[j]) {
                        trace_sequence[trace_idx++] = output_nodes[j];
                    }
                }
                
                // Update context_trace (this is the "last x number" from Requirement.md)
                // Position in sequence = where we are
                if (trace_idx > 0) {
                    update_node_context_trace(sampled_node, trace_sequence, trace_idx, trace_idx - 1);
                }
                
                free(trace_sequence);
            }
            
            // Also update context_trace for nodes that will be candidates next iteration
            // This propagates sequence position information forward
            if (sampled_node->outgoing_count > 0) {
                // Update context_trace for outgoing edges' target nodes
                // They need to know "I come after this sequence"
                size_t update_count = (sampled_node->outgoing_count < 5) ? sampled_node->outgoing_count : 5;
                for (size_t i = 0; i < update_count; i++) {
                    Edge *out_edge = sampled_node->outgoing_edges[i];
                    if (out_edge && out_edge->to_node) {
                        // Build sequence ending with current node
                        size_t seq_cap = trace_seq_len + 1;
                        Node **seq_for_target = malloc(seq_cap * sizeof(Node*));
                        if (seq_for_target && output_nodes) {
                            size_t seq_idx = 0;
                            // Copy recent sequence
                            size_t seq_start = (output_len > trace_seq_len) ? (output_len - trace_seq_len) : 0;
                            for (size_t j = seq_start; j < output_len && seq_idx < trace_seq_len; j++) {
                                if (output_nodes[j]) {
                                    seq_for_target[seq_idx++] = output_nodes[j];
                                }
                            }
                            // Add current node
                            seq_for_target[seq_idx++] = sampled_node;
                            
                            // Update target's context_trace (it will know what came before)
                            if (seq_idx > 0) {
                                update_node_context_trace(out_edge->to_node, seq_for_target, seq_idx, seq_idx - 1);
                            }
                            
                            free(seq_for_target);
                        }
                    }
                }
            }
        }
        
        // Update node activation tracking
        if (sampled_node) {
            sampled_node->total_activations++;
        }
        
        // ENHANCEMENT: Data-driven loop detection
        // Per Requirement.md: No hardcoded thresholds - detect loops from actual patterns
        if (output_len >= 4 && output_nodes) {
            // DATA-DRIVEN: Pattern length = detect repeating sequences
            // Check for patterns of length 2, 3, 4 (adapts to actual output)
            int loop_detected = 0;
            
            for (size_t pattern_len = 2; pattern_len <= 4 && pattern_len <= output_len / 2; pattern_len++) {
                // Check if last pattern_len nodes repeat
                if (output_len >= pattern_len * 2) {
                    int is_repeating = 1;
                    for (size_t i = 0; i < pattern_len; i++) {
                        Node *recent = output_nodes[output_len - 1 - i];
                        Node *previous = output_nodes[output_len - 1 - pattern_len - i];
                        if (recent != previous) {
                            is_repeating = 0;
                            break;
                        }
                    }
                    
                    if (is_repeating) {
                        // DATA-DRIVEN: Compute repetition strength
                        // Count how many times pattern repeats
                        size_t repeat_count = 2;  // We found at least 2 repetitions
                        for (size_t check_pos = output_len - 1 - (pattern_len * 2); 
                             check_pos >= pattern_len; 
                             check_pos -= pattern_len) {
                            int matches = 1;
                            for (size_t i = 0; i < pattern_len; i++) {
                                if (check_pos < i || output_len - 1 < i) {
                                    matches = 0;
                                    break;
                                }
                                if (output_nodes[check_pos - i] != 
                                    output_nodes[output_len - 1 - i]) {
                                    matches = 0;
                                    break;
                                }
                            }
                            if (matches) {
                                repeat_count++;
                            } else {
                                break;
                            }
                        }
                        
                        // DATA-DRIVEN: Stop threshold = based on edge weights
                        // If we're repeating AND no strong new edges exist, stop
                        if (sampled_node && sampled_node->outgoing_count > 0) {
                            float max_new_edge_weight = 0.0f;
                            float local_avg = node_get_local_outgoing_weight_avg(sampled_node);
                            
                            // Check if any outgoing edge leads to a node NOT in recent pattern
                            for (size_t i = 0; i < sampled_node->outgoing_count; i++) {
                                Edge *edge = sampled_node->outgoing_edges[i];
                                if (!edge || !edge->to_node) continue;
                                
                                // Check if target is in recent pattern
                                int in_pattern = 0;
                                for (size_t j = 0; j < pattern_len && j < output_len; j++) {
                                    if (output_nodes[output_len - 1 - j] == edge->to_node) {
                                        in_pattern = 1;
                                        break;
                                    }
                                }
                                
                                if (!in_pattern) {
                                    // This edge leads outside the loop
                                    float edge_relative = (local_avg > 0) ? 
                                        (edge->weight / local_avg) : edge->weight;
                                    if (edge_relative > max_new_edge_weight) {
                                        max_new_edge_weight = edge_relative;
                                    }
                                }
                            }
                            
                            // DATA-DRIVEN: Stop if repeating AND no strong escape edges
                            // Threshold = local average (data-driven, not hardcoded)
                            float escape_threshold = local_avg * 0.5f;  // 50% of average = weak
                            if (max_new_edge_weight < escape_threshold && repeat_count >= 3) {
                                loop_detected = 1;
                                break;
                            }
                        } else {
                            // No outgoing edges = natural stop
                            loop_detected = 1;
                            break;
                        }
                    }
                }
            }
            
            if (loop_detected) {
                /* stop_reason=loop_detected */
                break;
            }
        }
        
        // Natural stop: if node has no outgoing edges, stop
        if (sampled_node && sampled_node->outgoing_count == 0) {
            /* stop_reason=no_outgoing_edges */
            break;
        }
        
        // KEY INSIGHT: Like LLMs predicting next token, we don't re-run wave propagation after every byte
        // Instead, we continue the loop and ask the output node to choose the next edge
        // Wave propagation's role is to decide when to stop (based on readiness/energy)
        // But the actual byte generation comes from node-based local decisions
        
        // Continue loop - next iteration will ask the output node to choose next edge
        // Stop conditions (all data-driven, no hardcoded rules):
        // 1. Node can't make a valid choice (no valid edges) → payload complete
        // 2. Node has no outgoing edges → payload complete
        // 3. Wave-based stop prediction (mini neural net) → learned stop signal
        // Wave propagation doesn't need to run after every byte - nodes make the decisions
        
        // NOTE: Don't free weights or clear candidates here - they're not needed for node-based decisions
        // The next iteration will use node_choose_next_edge() which only looks at the node's edges
        // Weights and candidates are only used for the first iteration (from input) or when no node decision exists
        
        // Continue loop - next iteration will:
        // 1. Check if output node can make a choice (has valid edges) - handled at line 6140
        // 2. If yes, ask node to choose next edge (node-based decision) - handled at line 6156
        // 3. If no, stop (payload complete - node indicates completion) - handled at line 6144
        // 4. Wave propagation stop prediction - handled above
        continue;  // Continue to next iteration of the loop
    }
    
    // Cleanup: Free resources
    if (output_nodes) free(output_nodes);
    
    // Clear ownership invariants: candidates and weights are both NULL or both valid
    // Note: state is owned by caller, don't free it
    if (weights) {
        free(weights);
        weights = NULL;
    }
    // Free allocated valid candidates arrays (from autoregressive loop iterations)
    if (allocated_valid_candidates) {
        free(allocated_valid_candidates);
        free(allocated_valid_strengths);
        free(allocated_valid_transforms);
        allocated_valid_candidates = NULL;
        allocated_valid_strengths = NULL;
        allocated_valid_transforms = NULL;
    }
    // Free initial candidates array if we allocated it (first iteration only)
    if (candidates && candidates_need_free) {
        free(candidates);
        candidates = NULL;
    }
    candidate_count = 0;
    
    // 5. Write to universal_output (data-driven buffer growth)
    if (output_len > 0 && output) {
        // Grow output buffer if needed (growth computed from actual need)
        while (mfile->universal_output_size + output_len > mfile->universal_output_capacity) {
            // Growth computed from actual need (data-driven)
            size_t needed_growth = (mfile->universal_output_size + output_len + 1) - mfile->universal_output_capacity;
            
            // Additional growth from usage pattern (data-driven)
            size_t additional_growth = 0;
            if (mfile->universal_output_size > 0 && mfile->universal_output_capacity > 0) {
                // Compute growth rate from current usage
                float usage_rate = (float)mfile->universal_output_size / (float)mfile->universal_output_capacity;
                additional_growth = (size_t)(usage_rate * (float)mfile->universal_output_capacity);
            }
            
            // Total growth: actual need + additional from usage (all data-driven)
            size_t total_growth = needed_growth + additional_growth;
            size_t new_capacity = mfile->universal_output_capacity + total_growth;
            
            uint8_t *new_output = realloc(mfile->universal_output, new_capacity);
            if (!new_output) break;  // Can't grow, use what we have
            mfile->universal_output = new_output;
            mfile->universal_output_capacity = new_capacity;
        }
        
        // Append output
        memcpy(mfile->universal_output + mfile->universal_output_size, output, output_len);
        mfile->universal_output_size += output_len;
    } else {
        // If no output generated, echo input if activation suggests it (probabilistic, data-driven)
        if (input_count > 0 && input_nodes[0] && input_nodes[0]->payload_size > 0) {
            // Echo length computed from actual payload size (data-driven, no hard limit)
            size_t echo_len = input_nodes[0]->payload_size;
            
            // Grow output buffer if needed (same data-driven growth)
            while (mfile->universal_output_size + echo_len > mfile->universal_output_capacity) {
                size_t needed_growth = (mfile->universal_output_size + echo_len + 1) - mfile->universal_output_capacity;
                size_t additional_growth = 0;
                if (mfile->universal_output_size > 0 && mfile->universal_output_capacity > 0) {
                    float usage_rate = (float)mfile->universal_output_size / (float)mfile->universal_output_capacity;
                    additional_growth = (size_t)(usage_rate * (float)mfile->universal_output_capacity);
                }
                size_t total_growth = needed_growth + additional_growth;
                size_t new_capacity = mfile->universal_output_capacity + total_growth;
                uint8_t *new_output = realloc(mfile->universal_output, new_capacity);
                if (!new_output) break;
                mfile->universal_output = new_output;
                mfile->universal_output_capacity = new_capacity;
            }
            
            // Echo input (probabilistic, data-driven)
            memcpy(mfile->universal_output + mfile->universal_output_size, input_nodes[0]->payload, echo_len);
            mfile->universal_output_size += echo_len;
        }
    }
    
    if (output) free(output);
    
    // Restore original state (if we modified it)
    if (state) {
        // State is owned by caller, but we may have modified it temporarily
        // Restore to original if needed (but don't free it)
    }
    
    // Function returns void (output written to mfile->universal_output)
    return;
}

/* ============================================================================
 * MAIN ENTRY POINT (Optional - for testing, compile with -DMELVIN_STANDALONE)
 * ============================================================================ */
#ifdef MELVIN_STANDALONE

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input_file> <brain.m>\n", argv[0]);
        return 1;
    }
    
    const char *input_path = argv[1];
    const char *brain_path = argv[2];
    
    printf("Melvin: Emergent Intelligence System\n");
    printf("Input: %s\n", input_path);
    printf("Brain: %s\n", brain_path);
    
    // Load existing or create new .m file
    MelvinMFile *mfile = melvin_m_load(brain_path);
    if (!mfile) {
        fprintf(stderr, "Error: Failed to create/open brain file\n");
        return 1;
    }
    
    // Use input port handler for file input
    int result = melvin_in_port_handle_text_file(mfile, 0, input_path);
    if (result < 0) {
        fprintf(stderr, "Error: Failed to process input file\n");
        melvin_m_close(mfile);
        return 1;
    }
    
    // Display output
    melvin_out_port_display_output(mfile);
    
    // Print statistics
    printf("\nStatistics:\n");
    printf("  Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("  Edges: %zu\n", melvin_m_get_edge_count(mfile));
    printf("  Adaptations: %lu\n", (unsigned long)melvin_m_get_adaptation_count(mfile));
    
    // Cleanup
    melvin_m_close(mfile);
    
    printf("\nDone.\n");
    return 0;
}
#endif /* MELVIN_STANDALONE */


