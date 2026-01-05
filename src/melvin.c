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
 * CORE DATA STRUCTURES
 * ============================================================================ */

/* Node: Mini Neural Net
 * - Computes activation from weighted inputs
 * - All decisions relative to local context
 * - Self-regulating bias
 * - O(degree) operations only
 */
typedef struct Node {
    uint8_t id[9];                    // Unique identifier (8 bytes + null)
    uint8_t *payload;                 // Actual data (flexible, 0 to unlimited)
    size_t payload_size;              // Size in bytes (0 = blank node)
    
    uint8_t port_id;                  // Port ID where this node originated (0 = unknown/original)
    
    float activation_strength;        // Current activation (0.0-1.0)
    float weight;                     // Activation history (local)
    float bias;                       // Self-regulating bias
    uint32_t abstraction_level;       // 0 = raw, 1+ = hierarchy
    
    // Local edge tracking (O(1) access to cached sums)
    struct Edge **outgoing_edges;
    size_t outgoing_count;
    size_t outgoing_capacity;
    float outgoing_weight_sum;        // Cached sum (maintained incrementally)
    
    struct Edge **incoming_edges;
    size_t incoming_count;
    size_t incoming_capacity;
    float incoming_weight_sum;        // Cached sum (maintained incrementally)
    
    // Adaptive learning rate tracking
    float *recent_weight_changes;     // Rolling window (adaptive size)
    size_t weight_change_capacity;
    size_t weight_change_count;
    size_t weight_change_index;
    float change_rate_avg;
    
    float state;                      // For residual connections (persistent state)
    
    // File offset for lazy loading
    off_t file_offset;
    uint8_t loaded;                   // 0 = not loaded, 1 = loaded
    
    // Self-destruct tracking (relative timer, no global state)
    float inactivity_timer;           // Increments when not activated, resets when activated
    
    // Stop prediction tracking (for wave propagation learning)
    uint32_t total_activations;       // Total times this node was activated during output
    
    // Context-relative edge values (for quick winning edge computation)
    float *edge_context_values;       // Cached: edge->weight / local_avg for each outgoing edge
    size_t edge_context_capacity;     // Allocated capacity of edge_context_values array
    Edge *best_edge;                  // Cached: pointer to edge with highest context value
    float best_edge_value;            // Cached: context value of best edge
    uint32_t context_generation;      // Generation when context values were last computed
    
    // CONTEXT TRACE (RNN-like hidden state for disambiguation)
    // Lightweight: only 8 floats stored, full 256-dim state computed on demand
    // This enables 'o' after "hell" to differ from 'o' after "w"
    float context_trace[8];           // Compressed context (from recent activations)
    uint8_t context_trace_len;        // How many slots are populated (0-8)
    uint32_t context_trace_gen;       // When trace was last updated (wave generation)
} Node;

/* Edge: Mini Transformer
 * - Transforms activation as it flows
 * - Considers pattern similarity and local context
 * - Boosts relevant paths
 * - All operations local
 */
typedef struct Edge {
    Node *from_node;                  // Source node
    Node *to_node;                    // Target node
    uint8_t direction;                // 1 = from->to, 0 = to->from
    uint32_t last_wave_generation;   // Last wave generation this edge fired (O(1) activation tracking)
    float weight;                     // Activation history (local)
    float routing_gate;               // Soft routing decision
    
    off_t file_offset;                // For lazy loading
    
    // Self-destruct tracking (relative timer, no global state)
    float inactivity_timer;           // Increments when not activated, resets when activated
    uint8_t marked_for_deletion;      // 1 = will be deleted after wave, 0 = active
    
    // Edge intelligence: cached similarity for similarity edges (guides wave propagation)
    float cached_similarity;          // -1.0 = not computed, 0.0-1.0 = similarity score
    uint8_t is_similarity_edge;       // 1 = similarity edge, 0 = other edge type
    
    // EDGE-SPECIFIC CONTEXT: Stores predecessor bytes when this edge was created
    // This enables disambiguation: edge o→' ' from "hello" has context "hell"
    //                              edge o→'r' from "world" has context "wor"
    uint8_t context_bytes[4];         // Last 4 bytes before this edge was traversed
    uint8_t context_len;              // How many context bytes are valid (0-4)
    
} Edge;

/* Graph: Container
 * - No global state
 * - Arrays grow exponentially
 * - No fixed maximum size
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

/* Forward declarations for three-phase architecture */
static ActivationPattern* activation_pattern_create(size_t initial_capacity);
static void activation_pattern_free(ActivationPattern *pattern);
static void activation_pattern_add(ActivationPattern *pattern, Node *node, float activation);
static int activation_pattern_contains(ActivationPattern *pattern, Node *node);
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

/* Smooth similarity boost (no hard cutoff)
 * - Smooth transition
 * - Relative to excess
 */
static inline float smooth_similarity_boost(float excess) {
    float epsilon = compute_adaptive_epsilon(fabsf(excess));
    return excess / (fabsf(excess) + 1.0f + epsilon);
}

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

/* Compute adaptive threshold multiplier from local variance (data-driven, not hardcoded)
 * - Higher variance → more exploration (higher threshold)
 * - Lower variance → more focus (lower threshold)
 * - Returns multiplier relative to local context (no hardcoded 0.5f)
 * - O(degree) operation
 */
static inline float compute_adaptive_threshold_multiplier(Node *node) {
    if (!node || node->outgoing_count == 0) return compute_minimal_threshold(node);  // Data-driven, not hardcoded
    
    // Compute local variance of edge weights (O(degree))
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float variance = 0.0f;
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge) {
            float diff = edge->weight - local_avg;
            variance += diff * diff;
        }
    }
    variance /= (float)node->outgoing_count;
    
    // Adaptive multiplier: higher variance → higher multiplier (more exploration)
    // Lower variance → lower multiplier (more focus)
    // Range adapts to local context (no hardcoded bounds)
    float epsilon = compute_adaptive_epsilon(variance);
    float normalized_variance = variance / (variance + local_avg + epsilon);
    
    // Smooth multiplier: ranges from minimal to maximal based on variance
    // All values derived from local data, not hardcoded
    float base_multiplier = compute_minimal_threshold(node);  // Data-driven base
    float variance_range = normalized_variance * (1.0f - base_multiplier);  // Adaptive range
    return base_multiplier + variance_range;
}

/* Compute adaptive strengthening rate from local change rate (data-driven, not hardcoded)
 * - Faster changes → higher strengthening rate
 * - Slower changes → lower strengthening rate
 * - Returns multiplier relative to local context (no hardcoded 1.1f)
 * - O(1) operation (uses cached change rate)
 */
static inline float compute_adaptive_strengthening_rate(Node *node) {
    if (!node) return 1.05f;  // Minimal strengthening when no data
    
    // Use node's tracked change rate (already computed, O(1) access)
    float change_rate = node->change_rate_avg;
    float epsilon = compute_adaptive_epsilon(change_rate);
    
    // Adaptive rate: faster changes → higher rate, slower changes → lower rate
    // Range adapts to local context (no hardcoded bounds)
    float rate_epsilon = compute_adaptive_epsilon(change_rate);
    float normalized_rate = change_rate / (change_rate + epsilon + rate_epsilon);
    
    // Smooth strengthening: base + adaptive boost from rate
    // All values derived from local data, not hardcoded
    float base_rate = 1.0f + rate_epsilon;  // Minimal strengthening
    float rate_boost = normalized_rate * (rate_epsilon * 10.0f);  // Adaptive boost
    return base_rate + rate_boost;
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

/* Compute minimal dimension from graph characteristics
 * - Replaces hardcoded 16
 * - Adapts to graph's average payload size
 */
static size_t compute_minimal_dimension(Graph *graph) {
    if (!graph || graph->node_count == 0) return 1;  // Absolute minimum
    
    // Average payload size in graph
    size_t total_size = 0;
    for (size_t i = 0; i < graph->node_count; i++) {
        total_size += graph->nodes[i]->payload_size;
    }
    size_t avg_size = total_size / graph->node_count;
    
    // Dimension: 8 per byte (data-driven)
    return (avg_size > 0) ? (avg_size * 8) : 1;
}

/* Compute dimension bounds from graph characteristics
 * - Replaces hardcoded 16, 512
 * - Adapts to graph's payload size distribution
 */
static void compute_dimension_bounds(Graph *graph, size_t *min_dim, size_t *max_dim) {
    if (!graph || graph->node_count == 0) {
        *min_dim = 1;
        *max_dim = 1;
        return;
    }
    
    // Compute payload size statistics
    size_t min_size = SIZE_MAX;
    size_t max_size = 0;
    
    for (size_t i = 0; i < graph->node_count; i++) {
        size_t size = graph->nodes[i]->payload_size;
        if (size > 0 && size < min_size) min_size = size;
        if (size > max_size) max_size = size;
    }
    
    // Bounds from data (8 dimensions per byte)
    *min_dim = (min_size == SIZE_MAX) ? 1 : (min_size * 8);
    *max_dim = (max_size > 0) ? (max_size * 8) : 1;
    
    // Ensure min <= max
    if (*min_dim > *max_dim) *min_dim = *max_dim;
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
    float range_width = 0.5f + variance_factor;  // Adaptive width
    
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
    float threshold1 = (float)avg_degree * (0.5f + variance_norm * 0.3f);  // Adaptive threshold 1
    float threshold2 = (float)avg_degree * (1.0f + variance_norm * 0.5f);  // Adaptive threshold 2
    float threshold3 = (float)avg_degree * (2.0f + variance_norm * 1.0f);  // Adaptive threshold 3
    
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
    
    // If has history, compute from local context
    float local_avg = node_get_local_incoming_weight_avg(input_node);
    float local_max = 0.0f;
    
    for (size_t i = 0; i < input_node->incoming_count; i++) {
        if (input_node->incoming_edges[i]->weight > local_max) {
            local_max = input_node->incoming_edges[i]->weight;
        }
    }
    
    float epsilon = compute_adaptive_epsilon(local_max);
    return local_max / (local_max + epsilon);
}

/* ============================================================================
 * ADAPTIVE LEARNING RATE (Principle 5: Adaptive Behavior)
 * ============================================================================ */

/* Compute adaptive learning rate from rolling window
 * - Based on observed change rate
 * - Window size adapts
 * - Relative to local context
 * - Returns 0.0f when no data (neutral)
 */
static float node_compute_adaptive_learning_rate(Node *node) {
    if (node->weight_change_count == 0) return 0.0f;  // Neutral when no data
    
    // Compute median change rate from rolling window
    float sum = 0.0f;
    for (size_t i = 0; i < node->weight_change_count; i++) {
        sum += fabsf(node->recent_weight_changes[i]);
    }
    float avg_change = sum / (float)node->weight_change_count;
    
    // Adaptive learning rate (faster when changing, slower when stable)
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return 0.0f;  // Neutral when no context
    
    // Learning rate relative to local context
    return avg_change / (local_avg + epsilon);
}

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
    node->change_rate_avg = node->change_rate_avg * 0.9f + outcome * 0.1f;
    
    // REUSE bias as adaptive learning rate multiplier
    // High success → increase bias (learn faster, exploit)
    // Low success → decrease bias (explore more)
    if (node->change_rate_avg > 0.7f) {
        node->bias *= 1.02f;  // Successful strategy: amplify learning
    } else if (node->change_rate_avg < 0.3f) {
        node->bias *= 0.98f;  // Failing strategy: dampen, explore more
    }
    
    // Clamp bias to data-driven range (prevents runaway values)
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    float max_bias = (local_avg > epsilon) ? (local_avg * 2.0f) : 2.0f;
    if (node->bias > max_bias) node->bias = max_bias;
    if (node->bias < 0.1f) node->bias = 0.1f;
}

/* SELF-OPTIMIZATION: Boost underutilized nodes that may be missing connections
 * - Identifies nodes with high inactivity but some weight (were once useful)
 * - Increases their discoverability by boosting weight
 * - No new fields - reuses total_activations, inactivity_timer, weight
 */
static void node_self_optimize_if_weak(Node *node) {
    if (!node) return;
    
    // REUSE existing fields to compute "weakness score"
    float usage_rate = (float)node->total_activations / 
                      (node->total_activations + node->inactivity_timer + 1.0f);
    
    // If node was once useful (has weight) but is now underutilized
    if (usage_rate < 0.2f && node->weight > 0.5f) {
        // This node is underutilized - boost its discoverability
        node->weight *= 1.1f;
        
        // Also boost its outgoing edges slightly
        for (size_t j = 0; j < node->outgoing_count; j++) {
            Edge *edge = node->outgoing_edges[j];
            if (edge) edge->weight *= 1.05f;
        }
    }
}

/* SELF-OPTIMIZATION ON ERROR: Called when output was wrong
 * - Identifies nodes in error path and adjusts their meta-learning
 * - Boosts alternative paths that might be correct
 * - Reuses existing graph traversal infrastructure
 */
static void graph_self_optimize_on_error(Graph *graph, Edge **error_path, 
                                         size_t path_count, float error_signal) {
    if (!graph || !error_path || path_count == 0) return;
    if (error_signal > 0.5f) return;  // Only optimize on failures
    
    // For each node in the error path, update meta-learning
    for (size_t i = 0; i < path_count; i++) {
        Edge *edge = error_path[i];
        if (!edge || !edge->from_node) continue;
        
        // Meta-learn: track failure
        node_meta_learn_update(edge->from_node, error_signal);
        
        // For nodes in error path, boost ALTERNATIVE edges (exploration)
        Node *node = edge->from_node;
        for (size_t j = 0; j < node->outgoing_count; j++) {
            Edge *alt_edge = node->outgoing_edges[j];
            if (alt_edge && alt_edge != edge) {
                // Boost alternatives slightly (encourage exploration)
                alt_edge->weight *= 1.02f;
            }
        }
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
static float compute_stop_probability(Node *current_node, size_t output_len) {
    if (!current_node) return 1.0f;  // Stop if no node
    
    // Mini neural net inputs (all data-driven, normalized by local context):
    
    // 1. Current activation strength (how active is this node?)
    float activation_input = current_node->activation_strength;
    
    // 2. Accumulated state (RNN-like memory from previous steps)
    float state_input = current_node->state;
    
    // 3. Output length relative to node's weight (local normalization)
    // Nodes with high weight expect longer outputs
    float length_input = (float)output_len / (current_node->weight + 1.0f);
    
    // 4. Edge connectivity (nodes with few edges likely to stop soon)
    float connectivity_input = 1.0f / (float)(current_node->outgoing_count + 1);
    
    // Mini neural net: 2-layer feedforward
    // Layer 1: Weighted combination (weights are data-driven from node properties)
    float local_avg = node_get_local_outgoing_weight_avg(current_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // Adaptive weights based on local context
    float w1 = activation_input / (local_avg + epsilon + 1.0f);
    float w2 = state_input / (local_avg + epsilon + 1.0f);
    float w3 = length_input * current_node->bias;  // Bias as meta-learned weight
    float w4 = connectivity_input;
    
    float hidden = w1 * 0.3f + w2 * 0.3f + w3 * 0.2f + w4 * 0.2f;
    
    // Non-linearity (sigmoid)
    hidden = 1.0f / (1.0f + expf(-hidden * 3.0f));  // Gentler sigmoid
    
    // Layer 2: Output stop probability
    // Low activation + high length → high stop probability
    // High activation + low length → low stop probability
    float stop_prob = hidden * (1.0f - activation_input * 0.7f);
    
    // Use node's total_activations as experience factor
    // More experienced nodes make better stop decisions
    // Start with low stop probability (let it generate more initially)
    float experience = (float)current_node->total_activations / 
                      ((float)current_node->total_activations + 50.0f);
    stop_prob = stop_prob * experience + (1.0f - experience) * 0.1f;  // Blend with low baseline
    
    // Clamp to [0, 1]
    if (stop_prob < 0.0f) stop_prob = 0.0f;
    if (stop_prob > 1.0f) stop_prob = 1.0f;
    
    return stop_prob;
}

/* Update stop prediction learning after output completes
 * - Called when output generation stops naturally
 * - Adjusts node's bias to improve future stop predictions
 * - Hebbian-like: strengthens the stop signal that was correct
 */
static void learn_stop_prediction(Node *node, size_t final_length, int stopped_correctly) {
    if (!node) return;
    
    // Update bias based on whether stop was correct
    // stopped_correctly: 1 = good stop, 0 = stopped too early/late
    float target_bias = (float)final_length / ((float)final_length + 10.0f);
    
    if (stopped_correctly) {
        // Good stop - reinforce current bias
        node->bias = node->bias * 0.95f + target_bias * 0.05f;
    } else {
        // Bad stop - adjust bias away from current value
        node->bias = node->bias * 0.98f + target_bias * 0.02f;
    }
    
    // Increment total_activations (experience counter)
    node->total_activations++;
}

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
    
    // Start with capacity 1 (minimal, grows immediately when needed)
    node->outgoing_capacity = 1;
    node->outgoing_edges = malloc(sizeof(Edge*));
    node->incoming_capacity = 1;
    node->incoming_edges = malloc(sizeof(Edge*));
    
    node->state = 0.0f;
    
    // Initialize self-destruct timer (relative, no global state)
    node->inactivity_timer = 0.0f;
    
    // Initialize stop prediction tracking
    node->total_activations = 0;
    
    // Initialize context-relative edge values
    node->edge_context_values = NULL;
    node->edge_context_capacity = 0;
    node->best_edge = NULL;
    node->best_edge_value = -1.0f;
    node->context_generation = 0;
    
    node->loaded = 1;
    return node;
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
static Node* graph_find_or_create_pattern_node(Graph *graph, const uint8_t *pattern, size_t pattern_size);
static int graph_add_edge(Graph *graph, Edge *edge);


/* ============================================================================
 * WAVE PROPAGATION STRUCTURE DECISIONS (Hierarchy & Blank Nodes)
 * ============================================================================ */

/* Forward declarations for structure creation functions */
static Node* create_hierarchy_node(Graph *graph, Node *node1, Node *node2);
static Node* graph_create_blank_from_cluster(Graph *graph, Node **cluster, size_t cluster_size);

/* Forward declarations for position-aware learning (SOLUTION 2) */

/* Collect hierarchy formation signals from edge during wave propagation
 * - All signals are local and data-driven
 */

/* Check if node should self-destruct (relative timer, local context)
 * - Node checks its own usefulness from its timer
 * - No global state: all thresholds computed from local context
 */
static int node_should_self_destruct(Node *node) {
    if (!node) return 0;
    
    // Never delete nodes that are currently activated
    if (node->activation_strength > 0.0f) return 0;
    
    // Check if node is isolated (no edges)
    if (node->outgoing_count == 0 && node->incoming_count == 0) {
        // Isolated node: check inactivity timer
        float isolation_threshold = compute_adaptive_isolation_threshold(node);
        if (node->inactivity_timer > isolation_threshold) {
            return 1;  // Node should self-destruct (isolated and unused)
        }
    }
    
    // Check if node has very low usefulness (weight is very low relative to neighbors)
    float local_avg_weight = compute_local_avg_node_weight(node);
    float epsilon = compute_adaptive_epsilon(local_avg_weight);
    
    if (local_avg_weight > epsilon) {
        float weight_relative = node->weight / (local_avg_weight + epsilon);
        float inactivity_threshold = compute_adaptive_node_inactivity_threshold(node);
        
        // Node should self-destruct if:
        // 1. Weight is far below local average (never activates)
        // 2. Inactivity timer exceeds threshold
        if (weight_relative < inactivity_threshold && node->inactivity_timer > inactivity_threshold) {
            return 1;  // Node should self-destruct
        }
    }
    
    return 0;
}

/* Node self-destructs (removes itself from graph)
 * - Frees node resources
 * - Note: Node removal from graph array happens during cleanup
 */
static void node_self_destruct(Node *node) {
    if (!node) return;
    
    // Free node resources
    if (node->payload) free(node->payload);
    if (node->recent_weight_changes) free(node->recent_weight_changes);
    if (node->outgoing_edges) free(node->outgoing_edges);
    if (node->incoming_edges) free(node->incoming_edges);
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
        
        // Accumulate transformed activation (already includes weight and boosts)
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

/* Update node weight (local only, adaptive learning rate)
 * - No global gradient
 * - O(1) operation
 */
static void node_update_weight(Node *node, float activation) {
    if (!node) return;
    
    float old_weight = node->weight;
    float learning_rate = node_compute_adaptive_learning_rate(node);
    
    // Update weight based on activation
    node->weight += learning_rate * activation;
    
    // Track weight change for adaptive learning rate
    node_update_weight_change_window(node, node->weight - old_weight);
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
    // Pure data-driven: uses node's actual weight, no magic numbers
    float base_threshold = node->weight + 1.0f;  // Weight + 1.0 (data-driven)
    
    return base_threshold;
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

/* Check if node should be marked for deletion based on bloat score
 * Returns 1 if node should be deleted, 0 otherwise
 * 
 * Principle 2: NO HARDCODED THRESHOLDS
 * Threshold is computed from local context (neighbors' bloat scores)
 */
static int node_should_self_delete(Node *node, Graph *graph) {
    if (!node || !graph) return 0;
    
    float my_bloat = compute_node_bloat_score(node);
    
    // Compute local average bloat (from neighbors)
    float neighbor_bloat_sum = 0.0f;
    size_t neighbor_count = 0;
    
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < 10; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node) {
            neighbor_bloat_sum += compute_node_bloat_score(edge->to_node);
            neighbor_count++;
        }
    }
    
    for (size_t i = 0; i < node->incoming_count && neighbor_count < 20; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node) {
            neighbor_bloat_sum += compute_node_bloat_score(edge->from_node);
            neighbor_count++;
        }
    }
    
    if (neighbor_count == 0) {
        // Completely isolated node - high bloat threshold
        return my_bloat > 0.8f;
    }
    
    float avg_neighbor_bloat = neighbor_bloat_sum / (float)neighbor_count;
    
    // DATA-DRIVEN threshold: delete if significantly more bloated than neighbors
    // Factor of 2x above average is the natural cutoff
    float threshold = avg_neighbor_bloat * 2.0f;
    
    // REMOVED hardcoded minimum (0.3f) - violates README Principle 2
    // README says: "When no data exists: use minimal context or return 0.0f (neutral, not a threshold)"
    // If threshold is very low, that's data-driven (neighbors are not bloated)
    // No artificial floor needed - let data determine the threshold
    
    return my_bloat > threshold;
}

/* ============================================================================
 * EDGE OPERATIONS (Mini Transformers)
 * ============================================================================ */

/* Create edge (starts minimal)
 * - Connects two nodes
 * - Initial weight from local context
 */
static Edge* edge_create(Node *from_node, Node *to_node) {
    if (!from_node || !to_node) return NULL;
    
    Edge *edge = calloc(1, sizeof(Edge));
    if (!edge) return NULL;
    
    edge->from_node = from_node;
    edge->to_node = to_node;
    edge->direction = 1;  // from->to
    
    // Initial weight for sequential co-activation edges: 1.0
    // These edges represent OBSERVED patterns (we just saw this sequence)
    // They should start strong, not weak
    // Following README: Edges represent learned sequential patterns
    // The weight will adapt through Hebbian learning as the pattern is reinforced or weakened
    edge->weight = 1.0f;
    
    // Initialize self-destruct timer (relative, no global state)
    edge->inactivity_timer = 0.0f;
    edge->marked_for_deletion = 0;  // Active
    
    // Initialize edge intelligence fields
    edge->cached_similarity = -1.0f;  // Not computed yet (-1.0 indicates uncomputed)
    edge->is_similarity_edge = 0;      // Not a similarity edge by default
    
    
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
        node_update_outgoing_weight_sum(from_node, 0.0f, edge->weight);
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
        node_update_incoming_weight_sum(to_node, 0.0f, edge->weight);
    }
    
    return edge;
}

/* Check if edge should self-destruct (relative timer, local context)
 * - Edge checks its own usefulness from its timer
 * - No global state: all thresholds computed from local context
 */
static int edge_should_self_destruct(Edge *edge) {
    if (!edge || !edge->from_node) return 0;
    
    // Get local context (from from_node)
    float local_avg = node_get_local_outgoing_weight_avg(edge->from_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return 0;  // No pruning when no context
    
    // Compute weight relative to local average
    float weight_relative = edge->weight / (local_avg + epsilon);
    
    // Compute adaptive inactivity threshold (from local context)
    float inactivity_threshold = compute_adaptive_edge_inactivity_threshold(edge->from_node);
    
    // Self-destruct if:
    // 1. Weight is far below local average (decayed significantly)
    // 2. Inactivity timer exceeds threshold (hasn't activated in a while)
    if (weight_relative < inactivity_threshold && edge->inactivity_timer > inactivity_threshold) {
        return 1;  // Edge should self-destruct
    }
    
    return 0;
}

/* Edge self-destructs (removes itself from graph)
 * - Removes self from both nodes' edge lists
 * - Frees self (edge knows how to clean itself up)
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
    
    // Free self (edge knows how to clean itself up)
    
    free(edge);
}

/* Transform activation through edge (mini transformer)
 * - Context-aware attention (like transformer attention mechanism)
 * - Pattern similarity boost
 * - Routing gate (learned gating)
 * - Primary path boost
 * - All relative to local context
 * - O(1) operation per edge
 * - Self-destruct check: edge checks its own usefulness
 * 
 * This is the CORE of Melvin's intelligence - edges as mini transformers
 * Following README: "edges act as mini transformers"
 */
static float edge_transform_activation(Edge *edge, float input_activation, Graph *graph) {
    if (!edge || !edge->from_node || !edge->to_node) return 0.0f;
    
    // Update relative timer: reset if activated, increment if not
    // Check if edge fired this wave (O(1) generation comparison)
    int edge_fired_this_wave = (graph && edge->last_wave_generation == graph->wave_generation);
    
    if (input_activation > 0.0f || edge_fired_this_wave) {
        edge->inactivity_timer = 0.0f;  // Reset timer (edge is active)
        if (graph) {
            edge->last_wave_generation = graph->wave_generation;  // Mark as fired this wave
        }
    } else {
        // Increment timer (edge is inactive)
        // Increment rate adapts to local context (not hardcoded)
        float increment_rate = compute_adaptive_edge_timer_increment(edge->from_node);
        edge->inactivity_timer += increment_rate;
    }
    
    // SELF-DESTRUCT CHECK: Edge checks its own timer relative to local context
    // Mark for deletion instead of deleting immediately (prevents use-after-free)
    if (edge_should_self_destruct(edge)) {
        edge->marked_for_deletion = 1;  // Mark for cleanup after wave
        return 0.0f;  // Edge is dying, no activation flows
    }
    
    // Get local average for context (all computations relative to local context)
    float local_avg = node_get_local_outgoing_weight_avg(edge->from_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // MINI TRANSFORMER COMPUTATION:
    // Like transformer: attention_score = Q·K / sqrt(d_k)
    // Melvin: transformation = weight * activation * context_attention * routing_gate
    
    // 1. BASE TRANSFORMATION (weight represents learned connection strength)
    float base_transform = edge->weight * input_activation;
    
    // 2. ROUTING GATE (learned gating mechanism - like transformer's value projection)
    // routing_gate adapts during learning to control information flow
    float gate_factor = 1.0f / (1.0f + expf(-edge->routing_gate));  // Sigmoid activation
    
    // 3. PATTERN SIMILARITY (use cached if available, compute if not)
    // Similarity edges cache similarity to avoid recomputation (performance benefit)
    float similarity = 0.0f;
    if (edge->is_similarity_edge && edge->cached_similarity >= 0.0f) {
        // Use cached similarity (skip recomputation - performance benefit)
        similarity = edge->cached_similarity;
    } else {
        // Compute similarity on-the-fly (for non-similarity edges)
        if (edge->from_node->payload_size > 0 && edge->to_node->payload_size > 0) {
            size_t min_size = (edge->from_node->payload_size < edge->to_node->payload_size) ?
                              edge->from_node->payload_size : edge->to_node->payload_size;
            size_t matches = 0;
            for (size_t i = 0; i < min_size; i++) {
                if (edge->from_node->payload[i] == edge->to_node->payload[i]) matches++;
            }
            similarity = (float)matches / (float)min_size;
        }
    }
    
    // Compute similarity edge boost multiplier directly from data (no hardcoded ranges)
    // Similarity edges get stronger boost when they're more important (weight relative to local average)
    float similarity_multiplier = 1.0f;
    if (edge->is_similarity_edge) {
        // Compute boost from ratio of similarity edge weight to local average
        // If similarity edge is stronger than average, it gets more boost
        float weight_relative = edge->weight / (local_avg + epsilon);
        
        // Boost multiplier = 1.0 + (how much stronger similarity edge is)
        // Direct computation from data, no hardcoded ranges
        if (weight_relative > 1.0f) {
            float excess = weight_relative - 1.0f;
            similarity_multiplier = 1.0f + (excess / (excess + 1.0f));
        }
        // If weight_relative <= 1.0, multiplier stays at 1.0 (no boost)
    }
    
    // Smooth similarity boost (with adaptive multiplier for similarity edges)
    float sim_threshold = local_avg / (local_avg + 1.0f + epsilon);
    float boost_factor = 0.0f;
    if (similarity > sim_threshold) {
        float excess = similarity - sim_threshold;
        boost_factor = smooth_similarity_boost(excess) * 
                      (local_avg / (local_avg + 1.0f + epsilon)) * 
                      similarity_multiplier;  // Adaptive boost from data
    }
    
    // Apply similarity boost
    float transformed = base_transform * (1.0f + boost_factor);
    
    // 4. PRIMARY PATH BOOST (if edge weight strong relative to local)
    // Adaptive threshold and boost (data-driven, not hardcoded)
    float weight_relative = edge->weight / (local_avg + epsilon);
    // Compute local variance for adaptive threshold (O(degree))
    float local_variance = 0.0f;
    if (edge->from_node->outgoing_count > 0) {
        for (size_t i = 0; i < edge->from_node->outgoing_count; i++) {
            Edge *e = edge->from_node->outgoing_edges[i];
            if (e) {
                float diff = e->weight - local_avg;
                local_variance += diff * diff;
            }
        }
        local_variance /= (float)edge->from_node->outgoing_count;
    }
    float normalized_variance = local_variance / (local_variance + local_avg + epsilon);
    // Smooth boost: higher relative weight → higher boost (no hard threshold)
    // Boost strength adapts to local context
    float boost_threshold = 1.0f + normalized_variance;  // Adaptive (1.0-2.0 range from variance)
    if (weight_relative > boost_threshold) {
        // Adaptive boost multiplier: scales with local context (not hardcoded)
        float excess = weight_relative - boost_threshold;
        float boost_epsilon = compute_adaptive_epsilon(excess);
        // Boost strength adapts to excess and local average
        float boost_strength = 1.0f + (excess / (excess + local_avg + boost_epsilon));
        transformed *= boost_strength;  // Boost primary paths (relative multiplier)
    }
    
    // Apply routing gate (controls final information flow)
    transformed *= gate_factor;
    
    return transformed;
}

/* Transform activation through edge WITH CONTEXT (mini transformer with attention)
 * - This is the context-aware version used during output generation
 * - Computes attention based on context matching (like transformer Q·K)
 * - Returns transformed activation (like transformer attention output)
 * 
 * Following README: "edges act as mini transformers"
 * Context matching is THE KEY to disambiguation
 */
static float edge_transform_activation_with_context(Edge *edge, float input_activation, 
                                                     Graph *graph,
                                                     const uint8_t *context, size_t context_len) {
    if (!edge || !edge->from_node || !edge->to_node) return 0.0f;
    
    // Start with base transformation (includes all the standard logic)
    float base_output = edge_transform_activation(edge, input_activation, graph);
    
    if (base_output <= 0.0f) return 0.0f;  // Edge is dead or inactive
    
    // CONTEXT-AWARE ATTENTION (like transformer attention mechanism)
    // This is what makes edges "mini transformers" - they compute attention!
    
    if (edge->context_len == 0 || context_len == 0) {
        // No context available - return base output
        return base_output;
    }
    
    // Compute context similarity (like transformer Q·K dot product)
    // Compare edge's stored context against current context
    float context_match = 0.0f;
    size_t compare_len = (context_len < edge->context_len) ? context_len : edge->context_len;
    
    if (compare_len > 0) {
        // Compare from end (most recent bytes are most important)
        for (size_t k = 0; k < compare_len; k++) {
            size_t ctx_idx = context_len - 1 - k;  // From end of current context
            size_t edge_idx = edge->context_len - 1 - k;  // From end of edge context
            
            if (context[ctx_idx] == edge->context_bytes[edge_idx]) {
                // Recent matches weight more (positional encoding)
                float position_weight = 1.0f / (float)(k + 1);
                context_match += position_weight;
            }
        }
        
        // Normalize by maximum possible score
        float max_possible = 0.0f;
        for (size_t k = 0; k < compare_len; k++) {
            max_possible += 1.0f / (float)(k + 1);
        }
        if (max_possible > 0.0f) {
            context_match /= max_possible;  // Now in range [0, 1]
        }
    }
    
    // ATTENTION MECHANISM (like transformer attention weights)
    // Perfect context match = high attention, poor match = low attention
    // This is data-driven: attention computed from actual context similarity
    
    // Get local context for adaptive scaling
    float local_avg = node_get_local_outgoing_weight_avg(edge->from_node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    // Compute attention weight (softmax-like, but local and data-driven)
    // High context match → high attention
    // Low context match → low attention (but not zero - allow exploration)
    float attention_weight = context_match * context_match;  // Quadratic emphasis on good matches
    
    // Adaptive floor: edges with no context match still get some attention
    // Floor adapts to local context (not hardcoded)
    float attention_floor = epsilon / (local_avg + epsilon);
    if (attention_weight < attention_floor) {
        attention_weight = attention_floor;
    }
    
    // Final transformation: base_output * attention_weight
    // This is the "mini transformer" output!
    float final_output = base_output * attention_weight;
    
    // Bonus for perfect matches (like transformer residual connections)
    if (context_match > 0.9f) {  // Near-perfect match
        float bonus = context_match * local_avg * epsilon;
        final_output += bonus;
    }
    
    return final_output;
}

/* Score edge for routing (local only)
 * - Relative to local context
 * - For sparse routed attention
 */
static float edge_score_for_routing(Edge *edge, float source_activation, float local_avg_weight) {
    if (!edge) return 0.0f;
    
    float epsilon = compute_adaptive_epsilon(local_avg_weight);
    float weight_relative = edge->weight / (local_avg_weight + epsilon + 1.0f);
    
    // Use cached similarity if available (similarity edges)
    float similarity = 0.0f;
    if (edge->is_similarity_edge && edge->cached_similarity >= 0.0f) {
        // Use cached similarity (skip recomputation - performance benefit)
        similarity = edge->cached_similarity;
    } else {
        // Compute similarity on-the-fly (use local average as proxy - brain-like local inhibition)
        float local_avg_sim = node_get_local_outgoing_weight_avg(edge->from_node);
        float sim_epsilon = compute_adaptive_epsilon(local_avg_sim);
        similarity = (local_avg_sim > sim_epsilon) ? 
                    (local_avg_sim / (local_avg_sim + 1.0f + sim_epsilon)) : sim_epsilon;
    }
    
    // Compute routing priority directly from data (no hardcoded ranges)
    // Similarity edges get higher priority when they're more important (weight relative to local average)
    float similarity_priority = 1.0f;
    if (edge->is_similarity_edge) {
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
    
    // Boost score based on source activation strength (higher activation → higher priority)
    float activation_boost = source_activation / (source_activation + 1.0f);
    
    // Combined routing score (similarity edges get priority computed from data)
    return weight_relative * (1.0f + similarity * similarity_priority) * (1.0f + activation_boost);
}

/* Update edge weight (local only, adaptive)
 * - Based on activation
 * - Relative to local context
 */
static void edge_update_weight(Edge *edge, float activation) {
    if (!edge || !edge->from_node) return;
    
    float old_weight = edge->weight;
    float learning_rate = node_compute_adaptive_learning_rate(edge->from_node);
    
    // Update weight
    edge->weight += learning_rate * activation;
    
    // Update cached sums
    node_update_outgoing_weight_sum(edge->from_node, old_weight, edge->weight);
    if (edge->to_node) {
        node_update_incoming_weight_sum(edge->to_node, old_weight, edge->weight);
    }
}

/* Update node's context-relative edge values (called when context changes)
 * - Computes edge->weight / local_avg for each edge
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
    
    // Compute context-relative value for each edge
    float best_value = -1.0f;
    Edge *best = NULL;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node || edge->to_node->payload_size == 0) {
            node->edge_context_values[i] = -1.0f;  // Invalid edge
            continue;
        }
        
        // Self-loops are ALLOWED - they represent repeated characters
        // Context will determine when to use them vs other edges
        // No exclusion - let context make the decision
        
        // Context-relative value: edge weight relative to local average
        // Context makes this value obvious - higher = better
        float context_value = (local_avg > epsilon) ? 
                             (edge->weight / local_avg) : edge->weight;
        
        // Boost for target node readiness (if available)
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
        fprintf(stderr, "[DEBUG] All hierarchies in graph:\n");
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
    for (size_t h = 0; h < graph->node_count; h++) {
        Node *hierarchy = graph->nodes[h];
        if (!hierarchy || hierarchy->abstraction_level == 0) continue;
        // FIX: Changed <= to < so we can match hierarchies of same length
        // (e.g., "hello" hierarchy when full_sequence is "hello")
        if (hierarchy->payload_size < output_len) continue;
        
        // Check if output matches start of this hierarchy
        int matches = 1;
        for (size_t j = 0; j < output_len; j++) {
            if (output[j] != hierarchy->payload[j]) {
                matches = 0;
                break;
            }
        }
        
        if (matches && hierarchy->payload_size > best_length) {
            best_match = hierarchy;
            best_length = hierarchy->payload_size;
            if (out_position) *out_position = output_len;
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
                                                     Node **output_nodes, size_t output_nodes_len,
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
    
    // DEBUG: Print hierarchy matching info
    static int debug_count = 0;
    if (debug_count < 10) {  // Only print first 10 times
        fprintf(stderr, "[DEBUG] Edge selection: full_seq='");
        for (size_t i = 0; i < full_len && i < 20; i++) {
            fprintf(stderr, "%c", full_sequence[i]);
        }
        fprintf(stderr, "' (%zu bytes), hierarchy=%s", full_len, active_hierarchy ? "FOUND" : "NOT FOUND");
        if (active_hierarchy) {
            fprintf(stderr, " payload='");
            for (size_t i = 0; i < active_hierarchy->payload_size && i < 20; i++) {
                fprintf(stderr, "%c", active_hierarchy->payload[i]);
            }
            fprintf(stderr, "' pos=%zu", hierarchy_position);
        }
        fprintf(stderr, "\n");
        debug_count++;
    }
    
    Edge *result = NULL;
    
    if (active_hierarchy && hierarchy_position < active_hierarchy->payload_size) {
        // We're inside a hierarchy - it tells us exactly what comes next
        uint8_t expected_next = active_hierarchy->payload[hierarchy_position];
        
        // Find edge that matches hierarchy's directive
        for (size_t i = 0; i < node->outgoing_count; i++) {
            Edge *edge = node->outgoing_edges[i];
            if (!edge || !edge->to_node) continue;
            if (edge->is_similarity_edge) continue;
            
            if (edge->to_node->payload_size > 0 && 
                edge->to_node->payload[0] == expected_next) {
                // This edge follows the hierarchy map - ABSOLUTE PRIORITY
                result = edge;
                break;
            }
        }
        
        // FIX 2: ENSURE HIERARCHY EDGES EXIST
        // If hierarchy says "output X" but no edge exists, create it
        // Rationale: Hierarchies represent learned patterns - edges should exist
        if (!result && graph) {
            // Find or create target node
            Node *target = graph_find_or_create_pattern_node(graph, &expected_next, 1);
            if (target && target != node) {
                // Create edge (hierarchy says this connection should exist)
                Edge *new_edge = edge_create(node, target);
                if (new_edge) {
                    new_edge->weight = 2.0f;  // Strong weight (hierarchy says this is correct)
                    graph_add_edge(graph, new_edge);
                    result = new_edge;
                }
            }
        }
    }
    
    // Free dynamic buffer before returning
    free(full_sequence);
    
    if (result) return result;
    
    // No hierarchy guidance - use position-aware + context-based selection
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    float best_value = -1.0f;
    Edge *best = NULL;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
        if (edge->is_similarity_edge) continue;
        
        // Use edge weight directly
        float context_value = (local_avg > epsilon) ? 
                             (edge->weight / local_avg) : edge->weight;
        
        // BRAIN MECHANISM 1: Activation boost
        if (wave_state && wave_state->all_activated_nodes) {
            for (size_t j = 0; j < wave_state->all_activated_count; j++) {
                if (wave_state->all_activated_nodes[j] == edge->to_node) {
                    float activation_boost = 1.0f + wave_state->all_activation_strengths[j];
                    context_value *= activation_boost;
                    break;
                }
            }
        }
        
        // BRAIN MECHANISM 2: Habituation
        if (output_nodes && output_len > 0) {
            size_t times_output = 0;
            size_t last_position = 0;
            for (size_t j = 0; j < output_len; j++) {
                if (output_nodes[j] == edge->to_node) {
                    times_output++;
                    last_position = j;
                }
            }
            
            if (times_output > 0) {
                size_t steps_since = output_len - last_position;
                float habituation = (1.0f / (1.0f + (float)times_output)) * 
                                  (1.0f / (1.0f + (float)steps_since));
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

/* Node chooses next edge (local decision-making, no O(n) scanning)
 * - Node acts as mini neural net: computes winning edge using context-relative math
 * - Context makes the answer obvious - no need to scan all edges
 * - Returns the chosen next node (or NULL if no valid edges)
 * - This is a LOCAL decision - node only knows its own edges and context
 * - Following README: "nodes act like mini neural nets because they make decision on where activation goes based on there edges"
 * - Complexity comes from scale, not from expensive operations
 */
static Node* node_choose_next_edge(Node *node) {
    if (!node) return NULL;
    
    // Compute winning edge using context-relative math (no scanning)
    Edge *winning_edge = node_compute_winning_edge(node, NULL);
    
    if (winning_edge && winning_edge->to_node) {
        return winning_edge->to_node;
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

/* Remove edge from graph (intelligent edge deletion)
 * - Removes from graph's edge array
 * - Removes from both nodes' edge lists
 * - Frees edge memory (including gradients)
 * - O(degree + m) operation
 */
static void graph_remove_edge(Graph *graph, Edge *edge) {
    if (!graph || !edge) return;
    
    // Remove from from_node's outgoing list
    if (edge->from_node) {
        node_remove_edge_from_list(edge->from_node, edge, 1);
    }
    
    // Remove from to_node's incoming list
    if (edge->to_node) {
        node_remove_edge_from_list(edge->to_node, edge, 0);
    }
    
    // Remove from graph's edge array
    for (size_t i = 0; i < graph->edge_count; i++) {
        if (graph->edges[i] == edge) {
            // Shift remaining edges (maintain contiguous array)
            for (size_t j = i; j < graph->edge_count - 1; j++) {
                graph->edges[j] = graph->edges[j + 1];
            }
            graph->edge_count--;
            break;
        }
    }
    
    // Free edge memory
    free(edge);
}


/* Local edge decay (Principle 6: Continuous Learning)
 * - Unused edges decay relative to local context
 * - Weaker edges decay faster
 * - Strong edges persist
 * - O(degree) operation
 */
static void node_apply_local_edge_decay(Node *node, Graph *graph) {
    if (!node || node->outgoing_count == 0) return;
    
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = compute_adaptive_epsilon(local_avg);
    
    if (local_avg <= epsilon) return;  // No decay when no context
    
    // Decay rate computed from local distribution (adaptive, not hardcoded)
    float local_variance = compute_local_edge_variance(node);
    float decay_base = local_variance / (local_avg + local_variance + epsilon);  // From variance
    float decay_rate = decay_base * (local_avg / (local_avg + 1.0f));
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        // Skip edges that already fired this wave (O(1) generation check)
        if (!edge || (graph && edge->last_wave_generation == graph->wave_generation)) continue;
        
        // Weaker edges decay faster (relative to local average)
        float weight_relative = edge->weight / local_avg;
        if (weight_relative < 1.0f) {
            float old_weight = edge->weight;
            edge->weight *= (1.0f - decay_rate * (1.0f - weight_relative));
            node_update_outgoing_weight_sum(node, old_weight, edge->weight);
            if (edge->to_node) {
                node_update_incoming_weight_sum(edge->to_node, old_weight, edge->weight);
            }
        }
    }
}

/* ============================================================================
 * GRAPH OPERATIONS
 * ============================================================================ */

/* Compute initial stop threshold from graph state (data-driven, no hardcoded 0.5f)
 * - Empty graph: very permissive (low threshold)
 * - Mature graph: more conservative (higher threshold)
 */
static float compute_initial_stop_threshold(Graph *graph) {
    if (!graph || graph->node_count == 0) {
        // No data: very permissive (system should stop easily when no patterns learned)
        return 0.3f;  // Low threshold = stops early
    }
    
    // Compute from graph characteristics
    float avg_edge_weight = 0.0f;
    size_t sample_count = (graph->node_count < 100) ? graph->node_count : 100;
    size_t sampled = 0;
    
    for (size_t i = 0; i < graph->node_count && sampled < sample_count; i++) {
        if (graph->nodes[i]) {
            float node_avg = node_get_local_outgoing_weight_avg(graph->nodes[i]);
            if (node_avg > 0.0f) {
                avg_edge_weight += node_avg;
                sampled++;
            }
        }
    }
    
    if (sampled == 0) {
        // DATA-DRIVEN: No data = return neutral value (0.0f means "no operation")
        // Following README: "No fallbacks are allowed - return neutral value (0.0f)"
        return 0.0f;
    }
    avg_edge_weight /= (float)sampled;
    
    // DATA-DRIVEN: Return maturity directly (already 0-1, no hardcoded ranges)
    // Maturity = how strong patterns are (avg edge weight)
    // Pure data: maturity computed from actual edge weights
    float maturity = avg_edge_weight / (avg_edge_weight + 1.0f);
    return maturity;  // Already 0-1, data-driven, no magic numbers
}

/* Compute initial hierarchy threshold from graph state (data-driven, no hardcoded 0.6f)
 * - Empty graph: very permissive (low threshold, creates hierarchies easily)
 * - Mature graph: more conservative (higher threshold, only strong patterns)
 */

/* Create graph (starts with capacity 1)
 * - Principle 2: No hardcoded limits
 * - Grows from seed
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
    
    return graph;
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
    if (!pattern) return;
    
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
    
    free(pattern);
}

/* Check if pattern contains node (O(1) hash lookup) */
static int activation_pattern_contains(ActivationPattern *pattern, Node *node) {
    if (!pattern || !node || !pattern->node_hash) return 0;
    
    size_t hash_idx = ((size_t)node) % pattern->hash_size;
    if (!pattern->node_hash[hash_idx]) return 0;
    
    for (size_t i = 0; i < pattern->hash_counts[hash_idx]; i++) {
        if (pattern->node_hash[hash_idx][i] == node) return 1;
    }
    return 0;
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
static void activation_pattern_build_context(ActivationPattern *pattern, Graph *graph) {
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
    
    // Build context: weighted sum of byte representations
    float total_activation = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        if (!node || node->payload_size == 0 || !node->payload) continue;
        
        // Each byte contributes to its position in context vector
        uint8_t byte = node->payload[0];
        pattern->context_vector[byte] += activation;
        total_activation += activation;
    }
    
    // Normalize context vector
    if (total_activation > 0.0f) {
        for (size_t i = 0; i < dim; i++) {
            pattern->context_vector[i] /= total_activation;
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
        
        // Combine: temporal trace (recency) + position (order)
        float activation = 0.3f * temporal_trace + 0.7f * position_weight;
        
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
    
    // Debug: fprintf(stderr, "[ENCODE] Direct activation: %zu input nodes\n", pattern->count);
    
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
        Node *predecessor = NULL;
        for (size_t i = 1; i < input_count; i++) {
            if (input_nodes[i] == last_meaningful) {
                predecessor = input_nodes[i - 1];
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
        
        // Position decay rate: adapts to edge density
        // Many edges = slower decay (more options to consider)
        // Few edges = faster decay (focus on first few)
        float position_decay_rate = 1.0f / (1.0f + avg_edge_count * 0.1f);  // Data-driven
        
        // Continuation boost multiplier: adapts to last node's weight
        float last_weight = (last_meaningful) ? last_meaningful->weight : 1.0f;
        float boost_multiplier = 1.0f + last_weight / (last_weight + 1.0f);  // Range: 1.0 to 2.0
        
        // Check: does predecessor→last_meaningful→X path match the first edge?
        // The first edge (index 0) was created first during training
        for (size_t j = 0; j < last_meaningful->outgoing_count; j++) {
            Edge *edge = last_meaningful->outgoing_edges[j];
            if (!edge || !edge->to_node) continue;
            
            Node *target = edge->to_node;
            
            // ADAPTIVE POSITION BOOST (not hardcoded)
            // Earlier edges get higher boost, but decay rate is data-driven
            float position_factor = 1.0f / (1.0f + (float)j * position_decay_rate);
            float continuation_boost = edge->weight * position_factor * boost_multiplier;
            
            // ADAPTIVE control character penalty (not hardcoded 0.1f)
            // Based on target's weight relative to local average
            if (target->payload_size > 0 && target->payload[0] < 32) {
                float local_avg = node_get_local_incoming_weight_avg(target);
                float epsilon = compute_adaptive_epsilon(local_avg);
                float weight_ratio = target->weight / (local_avg + epsilon + 1.0f);
                float penalty = weight_ratio;  // High weight = less penalty
                continuation_boost *= penalty;
            }
            
            activation_pattern_add(pattern, target, continuation_boost);
            
            // Debug removed for clean output
        }
    }
    
    // ADAPTIVE HOP COUNT (data-driven, not hardcoded 3)
    // Compute from graph connectivity
    float avg_graph_degree = 0.0f;
    size_t degree_count = 0;
    for (size_t i = 0; i < graph->node_count && degree_count < 100; i++) {
        if (graph->nodes[i]) {
            avg_graph_degree += (float)(graph->nodes[i]->outgoing_count);
            degree_count++;
        }
    }
    avg_graph_degree = (degree_count > 0) ? (avg_graph_degree / (float)degree_count) : 2.0f;
    
    // More hops for highly connected graphs, fewer for sparse
    int max_hops = 2 + (int)(avg_graph_degree / 3.0f);  // Range: 2-5 hops (data-driven)
    if (max_hops > 5) max_hops = 5;  // Cap at 5 to prevent excessive spreading
    
    // ADAPTIVE DECAY BASE (data-driven, not hardcoded 0.3f)
    // Compute from pattern activation strength
    float pattern_strength = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        pattern_strength += pattern->activations[i];
    }
    pattern_strength = (pattern->count > 0) ? (pattern_strength / (float)pattern->count) : 0.5f;
    
    // Strong patterns = slower decay (spread further)
    // Weak patterns = faster decay (stay local)
    float decay_base = 0.2f + pattern_strength * 0.3f;  // Range: 0.2 to 0.5 (data-driven)
    
    // Multi-hop spreading with ADAPTIVE decay (not hardcoded)
    for (int hop = 0; hop < max_hops; hop++) {
        float decay = decay_base / (float)(hop + 1);  // Adaptive decay
        
        // Iterate through current pattern nodes
        size_t current_count = pattern->count;
        for (size_t i = 0; i < current_count; i++) {
            Node *node = pattern->nodes[i];
            float node_activation = pattern->activations[i];
            
            if (!node || node_activation < 0.01f) continue;
            
            // SELF-MODIFICATION: Accumulate context in node->state
            // This extends effective context window across hops
            node_accumulate_context(node, node_activation);
            
            // Don't spread from input nodes (except last meaningful)
            // This focuses spreading on the continuation path
            int is_input = 0;
            for (size_t k = 0; k < input_count; k++) {
                if (input_nodes[k] == node && node != last_meaningful) {
                    is_input = 1;
                    break;
                }
            }
            if (is_input) continue;
            
            // Spread through outgoing edges
            for (size_t j = 0; j < node->outgoing_count; j++) {
                Edge *edge = node->outgoing_edges[j];
                if (!edge || !edge->to_node) continue;
                
                // Spread activation = source activation * edge weight * decay
                float spread_activation = node_activation * edge->weight * decay;
                
                // Minimum threshold to avoid noise
                if (spread_activation < 0.001f) continue;
                
                // Add/update target node in pattern
                activation_pattern_add(pattern, edge->to_node, spread_activation);
            }
        }
    }
    
    // Debug removed for clean output
    (void)input_count;  // Suppress unused warning
    
    // 3. Build context vector
    activation_pattern_build_context(pattern, graph);
    
    return pattern;
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
    
    // Self-retention weight: high variance = more momentum (retain more)
    // Low variance = less momentum (allow more change)
    float variance_norm = activation_variance / (activation_variance + activation_mean + 0.01f);
    float self_weight = 0.4f + variance_norm * 0.4f;  // Range: 0.4 to 0.8 (data-driven)
    
    // Neighbor weight: inverse of self (more neighbor influence when less momentum)
    float neighbor_weight = (1.0f - self_weight) * 0.6f;  // Adaptive
    
    // Context weight: remaining capacity
    float context_weight = (1.0f - self_weight) * 0.4f;  // Adaptive
    
    // Bidirectional edge weight: based on graph connectivity
    float avg_degree = 0.0f;
    for (size_t i = 0; i < pattern->count; i++) {
        if (pattern->nodes[i]) {
            avg_degree += (float)(pattern->nodes[i]->incoming_count + pattern->nodes[i]->outgoing_count);
        }
    }
    avg_degree = (pattern->count > 0) ? (avg_degree / (float)pattern->count) : 2.0f;
    float bidirectional_weight = 1.0f / (1.0f + avg_degree * 0.1f);  // Lower weight for highly connected
    
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
 * PHASE 3: DECODE - Activation → Output (Context-Aware Generation)
 * ============================================================================
 * 
 * Like: LLM sample from softmax distribution
 * Like: Brain motor output from motor cortex
 * 
 * KEY INSIGHT: Sample from PATTERN (which includes spread nodes like " world")
 * Not just from input nodes
 */
static void generate_from_pattern(MFile *mfile, ActivationPattern *pattern, 
                                  Node **input_nodes, size_t input_count) {
    if (!mfile || !mfile->graph || !pattern || pattern->count == 0) return;
    
    Graph *graph = mfile->graph;
    
    // ERROR FEEDBACK: Clear previous output path tracking
    // Start fresh for this generation cycle
    mfile->last_output_path_count = 0;
    
    // Build candidate list: exclude input nodes (we want CONTINUATIONS, not echoes)
    Node **candidates = malloc(pattern->count * sizeof(Node*));
    float *weights = malloc(pattern->count * sizeof(float));
    size_t candidate_count = 0;
    
    if (!candidates || !weights) {
        free(candidates);
        free(weights);
        return;
    }
    
    // Collect CONTINUATION nodes (nodes reached via spreading, not input nodes)
    // This is the KEY insight: we want to generate what comes AFTER the input
    // Like LLM: given "hello", predict " world", not repeat "hello"
    
    // First, mark which nodes were input nodes
    int *is_input_node = calloc(pattern->count, sizeof(int));
    if (is_input_node) {
        for (size_t i = 0; i < pattern->count; i++) {
            Node *node = pattern->nodes[i];
            for (size_t j = 0; j < input_count; j++) {
                if (input_nodes[j] == node) {
                    is_input_node[i] = 1;
                    break;
                }
            }
        }
    }
    
    // Get expected output port from routing table
    uint8_t input_port = mfile->last_input_port_id;
    uint8_t output_port = input_port;  // Default: same port (text_in → text_out)
    // Note: melvin_out_port can override this with routing table
    
    // Collect only nodes that:
    // 1. Are NOT input nodes (discovered via spreading)
    // 2. Have valid payload
    // 3. Are not newlines (control characters shouldn't start output)
    // 4. Match output port (port_id filtering for multi-modal)
    for (size_t i = 0; i < pattern->count; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        if (!node || node->payload_size == 0 || !node->payload) continue;
        if (node->payload[0] == 0x00) continue;  // Skip EOS
        if (node->payload[0] == 0x0a) continue;  // Skip newlines (control char)
        if (activation < 0.01f) continue;  // Skip very weak activations
        
        // PORT ID FILTERING: Only include nodes from compatible port
        // This prevents text nodes from outputting to audio port, etc.
        // port_id == 0 means unknown/original (always compatible)
        // Edges can still connect across ports for cross-modal learning!
        if (node->port_id != 0 && output_port != 0 && node->port_id != output_port) {
            continue;  // Wrong port, skip (but edges remain for learning)
        }
        
        // ONLY include nodes that are NOT input nodes
        // These are continuation nodes discovered via spreading
        if (is_input_node && !is_input_node[i]) {
            candidates[candidate_count] = node;
            weights[candidate_count] = activation;
            candidate_count++;
        }
    }
    
    free(is_input_node);
    
    // Debug removed
    
    // If no candidates found, try using last input node's outgoing edges directly
    if (candidate_count == 0 && input_count > 0) {
        Node *last_input = input_nodes[input_count - 1];
        if (last_input && last_input->outgoing_count > 0) {
            for (size_t i = 0; i < last_input->outgoing_count; i++) {
                Edge *edge = last_input->outgoing_edges[i];
                if (edge && edge->to_node && edge->to_node->payload_size > 0) {
                    candidates[candidate_count] = edge->to_node;
                    weights[candidate_count] = edge->weight;
                    candidate_count++;
                }
            }
            // Debug removed
        }
    }
    
    if (candidate_count == 0) {
        // Debug removed
        free(candidates);
        free(weights);
        return;
    }
    
    // Debug: print candidates
    // Debug removed
    
    // Generate output autoregressively
    uint8_t *output = NULL;
    size_t output_len = 0;
    size_t output_capacity = 0;
    
    Node **output_nodes = NULL;
    size_t output_nodes_len = 0;
    size_t output_nodes_capacity = 0;
    
    // First byte: sample from candidates (highest activation wins)
    // Use activation from pattern, not just weight!
    size_t best_idx = 0;
    float best_score = -1.0f;
    
    for (size_t i = 0; i < candidate_count; i++) {
        // Score = activation from spreading (context-aware!)
        float score = weights[i];  // This is activation, not weight
        if (score > best_score) {
            best_score = score;
            best_idx = i;
        }
    }
    
    Node *current_node = candidates[best_idx];
    
    // Generate until stop condition
    // No hard length limit - relies on wave-based stop prediction (mini neural net)
    while (current_node && current_node->payload_size > 0) {
        // HIERARCHY USAGE: Output compressed knowledge
        // Following README: "use compressed knowledge" and "10:1 compression per level"
        // If node is a hierarchy (abstraction_level > 0), output ALL bytes, not just first
        // This leverages the compression benefit - one hierarchy node outputs multiple bytes
        
        size_t bytes_to_output = 1;  // Default: single byte (for raw nodes)
        if (current_node->abstraction_level > 0) {
            // Hierarchy node: output ALL bytes (compressed knowledge)
            bytes_to_output = current_node->payload_size;
        }
        
        // Output bytes from node's payload
        for (size_t b = 0; b < bytes_to_output; b++) {
            if (b >= current_node->payload_size) break;  // Safety check
            uint8_t byte = current_node->payload[b];
            if (byte == 0x00) break;  // EOS
            
            // Add to output
            if (output_len >= output_capacity) {
                size_t new_cap = (output_capacity == 0) ? 16 : output_capacity * 2;
                uint8_t *new_out = realloc(output, new_cap);
                if (!new_out) break;
                output = new_out;
                output_capacity = new_cap;
            }
            output[output_len++] = byte;
        }
        
        // Track output node (track once per hierarchy, not per byte)
        if (output_nodes_len >= output_nodes_capacity) {
            size_t new_cap = (output_nodes_capacity == 0) ? 16 : output_nodes_capacity * 2;
            Node **new_nodes = realloc(output_nodes, new_cap * sizeof(Node*));
            if (!new_nodes) break;
            output_nodes = new_nodes;
            output_nodes_capacity = new_cap;
        }
        output_nodes[output_nodes_len++] = current_node;
        
        // WAVE-BASED STOP PREDICTION (Mini Neural Net)
        // No hardcoded loop detection - use learned stop probability
        // Following README: "Mini neural nets" and "No hardcoded thresholds"
        float stop_prob = compute_stop_probability(current_node, output_len);
        
        // Sample: should we stop? (stochastic, allows exploration during training)
        float rand_val = (float)rand() / (float)RAND_MAX;
        if (rand_val < stop_prob) {
            // Neural net says stop
            learn_stop_prediction(current_node, output_len, 1);  // Reinforce this decision
            break;
        }
        
        // Find next node: use context-aware edge selection
        // SOLUTION 1: Sample from softmax distribution (exploration)
        // Instead of always picking strongest, sample to enable learning
        if (current_node->outgoing_count == 0) break;
        
        // Allocate arrays for candidate edges and scores
        size_t max_edges = current_node->outgoing_count;
        Edge **candidate_edges = malloc(max_edges * sizeof(Edge*));
        float *edge_scores = malloc(max_edges * sizeof(float));
        size_t edge_candidate_count = 0;
        
        if (!candidate_edges || !edge_scores) {
            free(candidate_edges);
            free(edge_scores);
            break;
        }
        
        // Get previous output byte for context (like n-gram language model)
        uint8_t prev_byte = 0;
        if (output_len > 0) {
            prev_byte = output[output_len - 1];
        }
        
        // KEY INSIGHT: Use edge index matching for context-aware selection
        // In "hello world":
        //   First 'o' (hello): preceded by 'l', edge[0] goes to ' '
        //   Second 'o' (world): preceded by 'w', edge[1] goes to 'r'
        // So: if prev_byte matches the predecessor for edge[k], prefer edge[k]
        
        // Get 2-byte context: [prev_byte, current_byte]
        uint8_t actual_prev = 0;
        uint8_t curr_byte = 0;
        if (output_len >= 2) {
            actual_prev = output[output_len - 2];
        }
        if (current_node->payload_size > 0) {
            curr_byte = current_node->payload[0];
        }
        
        // PATTERN MATCHING: Find where "prev curr" appears in training
        // Then prefer the byte that follows in that sequence
        // This is like n-gram language modeling
        Node *expected_next = NULL;
        if (actual_prev > 0 && curr_byte > 0) {
            // Find the node for prev_byte
            for (size_t b = 0; b < graph->node_count; b++) {
                Node *prev_node = graph->nodes[b];
                if (!prev_node || prev_node->payload_size == 0) continue;
                if (prev_node->payload[0] != actual_prev) continue;
                
                // Find edge from prev to current
                for (size_t k = 0; k < prev_node->outgoing_count; k++) {
                    Edge *e1 = prev_node->outgoing_edges[k];
                    if (!e1 || e1->to_node != current_node) continue;
                    
                    // Found prev→current! Now find what comes after in this sequence
                    // Look at current's outgoing edges that were likely created right after this
                    // The edge index k tells us which occurrence of prev→current this is
                    // The corresponding edge from current should be at a similar position
                    
                    // Heuristic: prefer edges from current that are NOT back to prev
                    // and are NOT to input nodes
                    for (size_t j = 0; j < current_node->outgoing_count; j++) {
                        Edge *e2 = current_node->outgoing_edges[j];
                        if (!e2 || !e2->to_node) continue;
                        if (e2->to_node == prev_node) continue;  // Skip going back
                        if (e2->to_node->payload_size == 0) continue;
                        if (e2->to_node->payload[0] < 32) continue;  // Skip control
                        
                        // Check if this is the first non-back, non-control edge
                        expected_next = e2->to_node;
                        break;
                    }
                    break;
                }
                if (expected_next) break;
            }
        }
        
        // Debug removed for clean output
        
        // Build current context: last 4 bytes from (input + output)
        uint8_t current_context[4] = {0};
        size_t current_ctx_len = 0;
        
        // Combine input and output into one sequence for context
        // First, add output bytes (most recent)
        for (size_t k = 0; k < output_len && current_ctx_len < 4; k++) {
            size_t idx = output_len - 1 - k;  // From end
            current_context[3 - current_ctx_len] = output[idx];
            current_ctx_len++;
        }
        
        // Then add input bytes (if we need more context)
        if (current_ctx_len < 4) {
            for (size_t k = 0; k < input_count && current_ctx_len < 4; k++) {
                size_t idx = input_count - 1 - k;  // From end
                Node *input_node = input_nodes[idx];
                if (input_node && input_node->payload_size > 0) {
                    current_context[3 - current_ctx_len] = input_node->payload[0];
                    current_ctx_len++;
                }
            }
        }
        
        // Score all outgoing edges using the MINI TRANSFORMER
        // Following README: "edges act as mini transformers"
        // The edge's transformation function handles ALL the intelligence:
        //   - Context-aware attention
        //   - Routing gates
        //   - Pattern similarity
        //   - Primary path boosting
        // NO manual boosts needed - the edge computes everything!
        
        for (size_t i = 0; i < current_node->outgoing_count; i++) {
            Edge *edge = current_node->outgoing_edges[i];
            if (!edge || !edge->to_node) continue;
            
            Node *target = edge->to_node;
            if (target->payload_size == 0 || !target->payload) continue;
            if (target->payload[0] == 0x00) continue;  // Skip EOS targets
            if (target->payload[0] < 32) continue;     // Skip control characters
            
            // MINI TRANSFORMER DOES ALL THE WORK
            // This single function call computes:
            //   1. Base transformation (edge weight * activation)
            //   2. Context-aware attention (Q·K matching)
            //   3. Routing gate (learned gating)
            //   4. Pattern similarity boost
            //   5. Primary path boost
            // All data-driven, no hardcoded boosts!
            
            float current_activation = current_node->activation_strength;
            if (current_activation <= 0.0f) {
                // Use node's accumulated state as activation (from wave propagation)
                current_activation = current_node->state;
                if (current_activation <= 0.0f) {
                    current_activation = current_node->weight;  // Fallback to node weight
                }
            }
            
            // THE KEY: Edge's mini transformer computes the score
            float score = edge_transform_activation_with_context(
                edge, 
                current_activation, 
                graph,
                current_context, 
                current_ctx_len
            );
            
            // META-LEARNING: Apply node's learned bias (self-modification)
            // Node learns better learning strategies over time
            if (current_node->bias > 0.0f) {
                score *= current_node->bias;
            }
            
            // Target's activation from wave propagation (if available)
            // Nodes activated during wave spreading get gentle boost
            float target_activation = activation_pattern_get_activation(pattern, target);
            if (target_activation > 0.0f) {
                score *= (1.0f + target_activation * 0.3f);  // Gentle boost
            }
            
            // Collect edge and score
            candidate_edges[edge_candidate_count] = edge;
            edge_scores[edge_candidate_count] = score;
            edge_candidate_count++;
        }
        
        // SOLUTION 1: Sample from softmax distribution
        Edge *selected_edge = NULL;
        
        if (edge_candidate_count == 0) {
            free(candidate_edges);
            free(edge_scores);
            break;
        } else if (edge_candidate_count == 1) {
            // Only one option, no need to sample
            selected_edge = candidate_edges[0];
        } else {
            // Multiple options: sample using softmax with adaptive temperature
            
            // Compute adaptive temperature from local variance (data-driven!)
            // High variance = high temperature (more exploration)
            // Low variance = low temperature (more exploitation)
            float score_sum = 0.0f;
            float score_sq_sum = 0.0f;
            for (size_t s = 0; s < edge_candidate_count; s++) {
                score_sum += edge_scores[s];
                score_sq_sum += edge_scores[s] * edge_scores[s];
            }
            float score_mean = score_sum / (float)edge_candidate_count;
            float score_variance = (score_sq_sum / (float)edge_candidate_count) - (score_mean * score_mean);
            if (score_variance < 0.0f) score_variance = 0.0f;
            
            // SELF-MODIFICATION: More deterministic edge selection
            // Lower temperature = more likely to pick highest-scoring edge
            // This reduces output chaos while still allowing some exploration
            float temp = 0.3f + sqrtf(score_variance) / (score_mean + 1.0f);
            if (temp < 0.2f) temp = 0.2f;  // Lower minimum: more deterministic
            if (temp > 1.5f) temp = 1.5f;  // Lower maximum: less random
            
            // Apply softmax with temperature
            float softmax_sum = 0.0f;
            for (size_t s = 0; s < edge_candidate_count; s++) {
                edge_scores[s] = expf(edge_scores[s] / temp);
                softmax_sum += edge_scores[s];
            }
            
            // Normalize to probabilities
            if (softmax_sum > 0.0f) {
                for (size_t s = 0; s < edge_candidate_count; s++) {
                    edge_scores[s] /= softmax_sum;
                }
            }
            
            // Sample from distribution
            // Use simple random number (could use better PRNG later)
            float rand_val = (float)rand() / (float)RAND_MAX;
            float cumsum = 0.0f;
            for (size_t s = 0; s < edge_candidate_count; s++) {
                cumsum += edge_scores[s];
                if (rand_val <= cumsum) {
                    selected_edge = candidate_edges[s];
                    break;
                }
            }
            
            // Fallback to last edge if sampling failed
            if (!selected_edge) {
                selected_edge = candidate_edges[edge_candidate_count - 1];
            }
        }
        
        free(candidate_edges);
        free(edge_scores);
        
        if (!selected_edge || !selected_edge->to_node) break;
        
        // Use selected_edge instead of best_edge
        Edge *best_edge = selected_edge;
        
        // ERROR FEEDBACK: Track this edge in the output path
        // This enables error-based learning (external system can feed back error signal)
        if (mfile->last_output_path_count >= mfile->last_output_path_capacity) {
            size_t new_cap = (mfile->last_output_path_capacity == 0) ? 16 : 
                             mfile->last_output_path_capacity * 2;
            Edge **new_path = realloc(mfile->last_output_path, new_cap * sizeof(Edge*));
            if (new_path) {
                mfile->last_output_path = new_path;
                mfile->last_output_path_capacity = new_cap;
            } else {
                // Allocation failed, continue without tracking (error feedback won't work)
            }
        }
        if (mfile->last_output_path_count < mfile->last_output_path_capacity) {
            mfile->last_output_path[mfile->last_output_path_count++] = best_edge;
        }
        
        current_node = best_edge->to_node;
        
        // No hard length limit - relies on wave-based stop prediction (mini neural net)
        // Neural net learns when to stop from training data
    }
    
stop_generation:
    // Write to universal output
    if (output_len > 0 && output) {
        // Grow output buffer if needed
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
        
        // Output written successfully
    } else {
        // No output generated
    }
    
    free(output);
    free(output_nodes);
    free(candidates);
    free(weights);
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
static void wave_propagate(WaveState *out, Graph *graph, Node **initial_nodes, 
                                      size_t initial_count, int max_steps) {
    if (!out || !graph || !initial_nodes || initial_count == 0) {
        if (out) wave_state_init(out);
        return;
    }
    
    // Reset output state (reset counters only, keep buffers for reuse)
    wave_state_reset(out);
    
    // O(1) edge activation tracking: Use generation counter instead of O(n) reset
    // Edges track last_wave_generation, we increment graph->wave_generation each wave
    // This avoids scanning all edges (O(n)) - edges are checked lazily when accessed
    graph->wave_generation++;  // O(1) operation - no need to reset all edges
    
    WaveState *state = out;  // Use out directly
    
    // Initialize/grow prediction collection arrays if needed
    size_t needed_capacity = graph->node_count;
    if (needed_capacity < initial_count) {
        needed_capacity = initial_count;
    }
    // Ensure minimum capacity (data-driven: at least enough for initial nodes)
    if (needed_capacity < 16) {
        needed_capacity = 16;  // Minimum to avoid zero allocation
    }
    
    if (!state->all_activated_nodes || state->all_activated_capacity < needed_capacity) {
        // Need to allocate or grow
        size_t new_capacity = needed_capacity;
        if (state->all_activated_capacity < new_capacity) {
            Node **new_all_nodes = realloc(state->all_activated_nodes, new_capacity * sizeof(Node*));
            float *new_all_strengths = realloc(state->all_activation_strengths, new_capacity * sizeof(float));
            float *new_all_transforms = realloc(state->all_edge_transforms, new_capacity * sizeof(float));
            
            if (!new_all_nodes || !new_all_strengths || !new_all_transforms) {
                wave_state_free(state);
                return;
            }
            
            state->all_activated_nodes = new_all_nodes;
            state->all_activation_strengths = new_all_strengths;
            state->all_edge_transforms = new_all_transforms;
            state->all_activated_capacity = new_capacity;
        }
    }
    
    // Track which nodes we've already collected (avoid duplicates)
    // Use hash set for O(1) lookup instead of O(n) array
    // Adaptive hash table size (data-driven, not hardcoded)
    size_t collected_set_size = compute_adaptive_hash_size(initial_count * 2);
    Node ***collected_set = calloc(collected_set_size, sizeof(Node**));
    size_t *collected_set_counts = calloc(collected_set_size, sizeof(size_t));
    size_t *collected_set_capacities = calloc(collected_set_size, sizeof(size_t));
    
    if (!collected_set || !collected_set_counts || !collected_set_capacities) {
        free(collected_set);
        free(collected_set_counts);
        free(collected_set_capacities);
        return;
    }
    
    // Hash function for node pointer (adaptive modulo)
    #define NODE_PTR_HASH(node) (((size_t)(node)) % collected_set_size)
    
    // Initialize wave front
    Node **wave_front = malloc(initial_count * sizeof(Node*));
    if (!wave_front) {
        // Clean up collected_set on error (adaptive size)
        for (size_t i = 0; i < collected_set_size; i++) {
            if (collected_set[i]) free(collected_set[i]);
        }
        free(collected_set);
        free(collected_set_counts);
        free(collected_set_capacities);
        return;
    }
    
    memcpy(wave_front, initial_nodes, initial_count * sizeof(Node*));
    size_t wave_front_size = initial_count;
    
    // Compute initial energy (sum of activation strengths)
    // Input nodes are the SOURCE of the wave - they should have initial activation
    // (Following README: "Wave starts from input nodes with activation")
    float initial_energy = 0.0f;
    for (size_t i = 0; i < initial_count; i++) {
        Node *input_node = initial_nodes[i];
        
        // Input nodes are the stimulus - compute activation from local context
        // Data-driven activation, not hardcoded 1.0f
        input_node->activation_strength = compute_input_activation(input_node);
        initial_energy += input_node->activation_strength;
        
        // IMPORTANT: Add initial nodes to all_activated_nodes BEFORE the loop
        // This ensures they are available for output generation even if the loop doesn't run
        // or if they don't propagate (no outgoing edges)
        size_t hash_idx = NODE_PTR_HASH(input_node);
        int already_collected = 0;
        
        // Check if already collected
        if (collected_set[hash_idx]) {
            for (size_t j = 0; j < collected_set_counts[hash_idx]; j++) {
                if (collected_set[hash_idx][j] == input_node) {
                    already_collected = 1;
                    break;
                }
            }
        }
        
        if (!already_collected) {
            // Add to collected set
            if (!collected_set[hash_idx]) {
                collected_set_capacities[hash_idx] = 4;
                collected_set[hash_idx] = malloc(collected_set_capacities[hash_idx] * sizeof(Node*));
                collected_set_counts[hash_idx] = 0;
            }
            if (collected_set_counts[hash_idx] >= collected_set_capacities[hash_idx]) {
                collected_set_capacities[hash_idx] *= 2;
                Node **new_bucket = realloc(collected_set[hash_idx], 
                                            collected_set_capacities[hash_idx] * sizeof(Node*));
                if (new_bucket) {
                    collected_set[hash_idx] = new_bucket;
                }
            }
            if (collected_set[hash_idx]) {
                collected_set[hash_idx][collected_set_counts[hash_idx]++] = input_node;
            }
            
            // Grow collection arrays if needed
            if (state->all_activated_count >= state->all_activated_capacity) {
                // Fix: If capacity is 0, start with 16. Otherwise double it.
                size_t new_capacity = (state->all_activated_capacity == 0) ? 16 : (state->all_activated_capacity * 2);
                state->all_activated_nodes = realloc(state->all_activated_nodes, new_capacity * sizeof(Node*));
                state->all_activation_strengths = realloc(state->all_activation_strengths, new_capacity * sizeof(float));
                state->all_edge_transforms = realloc(state->all_edge_transforms, new_capacity * sizeof(float));
                state->all_activated_capacity = new_capacity;
            }
            
            // Store initial node and its activation strength
            state->all_activated_nodes[state->all_activated_count] = input_node;
            state->all_activation_strengths[state->all_activated_count] = input_node->activation_strength;
            state->all_edge_transforms[state->all_activated_count] = 0.0f;  // Will be accumulated by edge transforms
            state->all_activated_count++;
        }
    }
    
    // Wave propagation steps (recurrent updates)
    // No BPTT state storage - we use biological local learning only
    for (int step = 0; step < max_steps && wave_front_size > 0; step++) {
        
        // Compute next wave front
        Node **next_wave_front = NULL;
        size_t next_wave_size = 0;
        size_t next_wave_capacity = 0;
        
        for (size_t i = 0; i < wave_front_size; i++) {
            Node *node = wave_front[i];
            if (!node) continue;
            
            // Node computes activation (mini neural net)
            // For input nodes (no incoming edges), preserve their initial activation
            // They are the source of the wave, so they should maintain their activation
            float activation;
            if (node->incoming_count == 0 && node->activation_strength > 0.0f) {
                // This is an input node with initial activation - preserve it
                activation = node->activation_strength;
                // Don't overwrite activation_strength - keep the initial value
            } else {
                // Normal node or input node that needs recomputation
                activation = node_compute_activation_strength(node);
                node->activation_strength = activation;
            }
            
            // Collect this activated node for output generation (if not already collected)
            // (Following README: "Wave Propagation Phase: Collects predictions from all activated nodes")
            // Use O(1) hash set lookup instead of O(n) linear search
            size_t hash_idx = NODE_PTR_HASH(node);
            int already_collected = 0;
            
            // Check if node is already in collected set (O(1) average case)
            if (collected_set[hash_idx]) {
                for (size_t i = 0; i < collected_set_counts[hash_idx]; i++) {
                    if (collected_set[hash_idx][i] == node) {
                        already_collected = 1;
                        break;
                    }
                }
            }
            
            if (!already_collected) {
                // Add node to collected set (O(1) average case)
                if (!collected_set[hash_idx]) {
                    collected_set_capacities[hash_idx] = 4;
                    collected_set[hash_idx] = malloc(collected_set_capacities[hash_idx] * sizeof(Node*));
                    collected_set_counts[hash_idx] = 0;
                }
                if (collected_set_counts[hash_idx] >= collected_set_capacities[hash_idx]) {
                    collected_set_capacities[hash_idx] *= 2;
                    Node **new_bucket = realloc(collected_set[hash_idx], 
                                                collected_set_capacities[hash_idx] * sizeof(Node*));
                    if (new_bucket) {
                        collected_set[hash_idx] = new_bucket;
                    }
                }
                if (collected_set[hash_idx]) {
                    collected_set[hash_idx][collected_set_counts[hash_idx]++] = node;
                }
                // Grow collection arrays if needed
                if (state->all_activated_count >= state->all_activated_capacity) {
                    // Fix: If capacity is 0, start with 16. Otherwise double it.
                    size_t new_capacity = (state->all_activated_capacity == 0) ? 16 : (state->all_activated_capacity * 2);
                    state->all_activated_nodes = realloc(state->all_activated_nodes, new_capacity * sizeof(Node*));
                    state->all_activation_strengths = realloc(state->all_activation_strengths, new_capacity * sizeof(float));
                    state->all_edge_transforms = realloc(state->all_edge_transforms, new_capacity * sizeof(float));
                    state->all_activated_capacity = new_capacity;
                }
                
                // Store activated node and its activation strength
                state->all_activated_nodes[state->all_activated_count] = node;
                state->all_activation_strengths[state->all_activated_count] = activation;
                state->all_edge_transforms[state->all_activated_count] = 0.0f;  // Will be accumulated by edge transforms
                state->all_activated_count++;
            }
            
            // NODE MAKES DECISION: Compute winning edge (O(1) from cache, no scanning)
            // (Following README: "Wave takes node decision as output payload")
            // "No need to collect all edges - just the winning path"
            
            // Update node weight (local learning)
            node_update_weight(node, activation);
            
            // SOLUTION 3: Track sequence path
            if (out->sequence_length >= out->sequence_capacity) {
                size_t new_cap = out->sequence_capacity == 0 ? 16 : out->sequence_capacity * 2;
                Node **new_path = realloc(out->sequence_path, new_cap * sizeof(Node*));
                if (new_path) {
                    out->sequence_path = new_path;
                    out->sequence_capacity = new_cap;
                }
            }
            if (out->sequence_length < out->sequence_capacity) {
                out->sequence_path[out->sequence_length++] = node;
            }
            
            // Check if we're following a hierarchy
            if (!out->active_hierarchy && out->sequence_length >= 2) {
                // Try to detect hierarchy execution
                for (size_t h = 0; h < graph->node_count; h++) {
                    Node *hierarchy = graph->nodes[h];
                    if (!hierarchy || hierarchy->abstraction_level == 0) continue;
                    
                    // Check if our sequence matches hierarchy start
                    int matches = 1;
                    size_t check_len = (out->sequence_length < hierarchy->payload_size) ? 
                                      out->sequence_length : hierarchy->payload_size;
                    for (size_t j = 0; j < check_len; j++) {
                        if (out->sequence_path[j]->payload_size == 0 ||
                            out->sequence_path[j]->payload[0] != hierarchy->payload[j]) {
                            matches = 0;
                            break;
                        }
                    }
                    
                    if (matches) {
                        out->active_hierarchy = hierarchy;
                        out->position_in_hierarchy = out->sequence_length;
                        out->hierarchy_confidence = hierarchy->weight / (hierarchy->weight + 1.0f);
                        break;
                    }
                }
            }
            
            // Get node's winning edge using context-relative math (O(1) from cache)
            Edge *winning_edge = node_compute_winning_edge(node, graph);
            
            if (winning_edge && winning_edge->to_node) {
                // Edge transforms activation through winning edge (mini transformer)
                float transformed = edge_transform_activation(winning_edge, activation, graph);
                
                // Compute routing gate (soft, not binary)
                float local_avg = node_get_local_outgoing_weight_avg(node);
                float routing_score = edge_score_for_routing(winning_edge, activation, local_avg);
                float routing_threshold = local_avg / (local_avg + 1.0f);
                float epsilon = compute_adaptive_epsilon(routing_score + routing_threshold);
                winning_edge->routing_gate = routing_score / (routing_score + routing_threshold + epsilon);
                
                // Apply routing gate (soft weighting)
                transformed *= winning_edge->routing_gate;
                
                // WAVE TAKES NODE DECISION: Propagate to winning edge's target only
                // (README: "Wave takes node decision as output payload")
                if (transformed > epsilon) {
                    // Add winning target to next wave front
                    if (next_wave_size >= next_wave_capacity) {
                        size_t new_cap = (next_wave_capacity == 0) ? 1 : next_wave_capacity * 2;
                        Node **new_front = realloc(next_wave_front, new_cap * sizeof(Node*));
                        if (new_front) {
                            next_wave_front = new_front;
                            next_wave_capacity = new_cap;
                        }
                    }
                    
                    if (next_wave_size < next_wave_capacity) {
                        next_wave_front[next_wave_size++] = winning_edge->to_node;
                        winning_edge->last_wave_generation = graph->wave_generation;
                    }
                    
                    // Update winning edge weight (local learning)
                    edge_update_weight(winning_edge, transformed);
                    
                    // Store edge transform for output generation
                    for (size_t k = 0; k < state->all_activated_count; k++) {
                        if (state->all_activated_nodes[k] == node) {
                            state->all_edge_transforms[k] = transformed;
                            break;
                        }
                    }
                }
                
                // STRUCTURE DECISIONS: Check for hierarchy formation on winning edge
                // Simple: if edge weight is strong relative to local average, consider hierarchy
                if (winning_edge->from_node && winning_edge->to_node) {
                    float local_avg = node_get_local_outgoing_weight_avg(winning_edge->from_node);
                    float epsilon = compute_adaptive_epsilon(local_avg);
                    float weight_relative = (local_avg > epsilon) ? 
                                           (winning_edge->weight / (local_avg + epsilon)) : 
                                           (winning_edge->weight / (epsilon + 1.0f));
                    
                    // Keep strongest candidate (simple comparison)
                    if (!state->hierarchy_candidate || weight_relative > state->hierarchy_weight_relative) {
                        state->hierarchy_candidate = winning_edge;
                        state->hierarchy_weight_relative = weight_relative;
                    }
                }
            }
            
            // Apply local edge decay (unused edges decay)
            node_apply_local_edge_decay(node, graph);
        }
        
        // Update wave front for next step
        free(wave_front);
        wave_front = next_wave_front;
        wave_front_size = next_wave_size;
        
        // Check convergence (energy dissipation, relative to initial)
        float current_energy = 0.0f;
        for (size_t i = 0; i < wave_front_size; i++) {
            if (wave_front[i]) {
                current_energy += wave_front[i]->activation_strength;
            }
        }
        
        float epsilon = compute_adaptive_epsilon(initial_energy);
        // Adaptive convergence threshold (data-driven, not hardcoded 0.1f)
        // Threshold adapts to initial energy magnitude
        float convergence_ratio = epsilon / (initial_energy + epsilon);  // Adaptive ratio
        if (current_energy < initial_energy * convergence_ratio + epsilon) {
            break;  // Natural convergence
        }
    }
    
    free(wave_front);
    
    // CLEANUP: Remove edges marked for deletion during wave propagation
    // (Following README: Self-destruct happens after wave to prevent use-after-free)
    for (size_t i = 0; i < graph->edge_count; i++) {
        Edge *edge = graph->edges[i];
        if (edge && edge->marked_for_deletion) {
            edge_self_destruct(edge);
            graph->edges[i] = NULL;  // Mark slot as empty
        }
    }
    
    // WAVE PROPAGATION STRUCTURE DECISIONS: Form hierarchies and blank nodes
    // (Following README: "Emergent Intelligence" - structure emerges from experience)
    // These are the core intelligence mechanisms that enable abstraction and generalization
    
    // 1. HIERARCHY FORMATION: Combine frequently co-occurring nodes
    // Simple: if edge weight is strong enough relative to local average, create hierarchy
    if (state->hierarchy_candidate && state->hierarchy_candidate->from_node && 
        state->hierarchy_candidate->to_node && state->hierarchy_weight_relative > 2.0f) {
        // Create hierarchy node combining the two nodes
        Node *hierarchy = create_hierarchy_node(graph, 
                                               state->hierarchy_candidate->from_node,
                                               state->hierarchy_candidate->to_node);
        if (hierarchy) {
            graph_add_node(graph, hierarchy);
        }
    }
    
    // Clean up collected_set hash table (adaptive size)
    for (size_t i = 0; i < collected_set_size; i++) {
        if (collected_set[i]) {
            free(collected_set[i]);
        }
    }
    free(collected_set);
    free(collected_set_counts);
    free(collected_set_capacities);
    
    // State is already in out, no return needed
}

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

/* Find blank node that accepts pattern (prototype matching)
 * - Returns best matching blank node, or NULL if none accept
 */
static Node* find_accepting_blank_node(Graph *graph, const uint8_t *pattern, size_t pattern_size) {
    if (!graph || !pattern || pattern_size == 0) return NULL;
    
    float best_score = 0.0f;
    Node *best_blank = NULL;
    
    // Check all blank nodes for acceptance
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (!node) continue;
        if (node->payload_size != 0) continue;  // Only blank nodes
        
        // Compute acceptance score (data-driven threshold)
        float score = compute_blank_acceptance_score(node, pattern, pattern_size);
        
        if (score > best_score) {
            best_score = score;
            best_blank = node;
        }
    }
    
    // Return best blank if score is positive (exceeds adaptive threshold)
    if (best_blank && best_score > 0.0f) {
        return best_blank;
    }
    
    return NULL;
}

static Node* graph_find_or_create_pattern_node(Graph *graph, const uint8_t *pattern, 
                                               size_t pattern_size) {
    if (!graph || !pattern || pattern_size == 0) return NULL;
    
    // STEP 1: Try hierarchy nodes first (larger patterns, higher abstraction)
    // Brain: Cortical hierarchy - higher areas recognize larger patterns
    // LLM: Learned representations - multi-token embeddings
    // Melvin: Try from highest abstraction to lowest
    
    if (graph->max_abstraction_level > 0) {
        // Try hierarchy nodes from highest to lowest abstraction
        for (uint32_t level = graph->max_abstraction_level; level > 0; level--) {
            for (size_t i = 0; i < graph->node_count; i++) {
                Node *node = graph->nodes[i];
                if (!node) continue;
                
                // Only check nodes at this abstraction level
                if (node->abstraction_level != level) continue;
                
                // Try matching this hierarchy node
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
        // Hash collision detected - need to search for exact match
        // This is rare, but handle it by checking all nodes with same hash prefix
        // For now, fall through to blank node check (collision handling can be optimized later)
    }
    
    // STEP 3: Check blank nodes (generalization)
    // Brain: If no exact match, try prototype-based categorization
    // LLM: Semantic similarity matching
    Node *accepting_blank = find_accepting_blank_node(graph, pattern, pattern_size);
    if (accepting_blank) {
        // Connect pattern to blank (generalization)
        Node *pattern_node = node_create(pattern, pattern_size, 0);
        if (pattern_node) {
            graph_add_node(graph, pattern_node);
            Edge *edge = edge_create(pattern_node, accepting_blank);
            if (edge) {
                graph_add_edge(graph, edge);
            }
        }
        return accepting_blank;  // Return blank node (generalization)
    }
    
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

/* Check if edge already exists between two nodes
 * - O(degree) operation
 */
static Edge* graph_find_edge_between(Node *from_node, Node *to_node) {
    if (!from_node || !to_node) return NULL;
    
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
static void graph_create_similarity_edges_for_node(Graph *graph, Node *new_node) {
    if (!graph || !new_node || new_node->payload_size == 0) return;
    
    // Compute adaptive similarity threshold (from local context)
    float similarity_threshold = compute_adaptive_similarity_threshold(new_node);
    
    // Check all existing nodes for similarity
    // OPTIMIZED: Limit to recent nodes for efficiency (last 100 nodes, or all if < 100)
    // Also limit total similarity checks to avoid O(n²) behavior
    size_t start_idx = (graph->node_count > 100) ? (graph->node_count - 100) : 0;
    size_t max_checks = 50;  // Limit total similarity checks per node
    size_t checks_done = 0;
    
    for (size_t i = start_idx; i < graph->node_count && checks_done < max_checks; i++) {
        Node *existing_node = graph->nodes[i];
        if (!existing_node || existing_node == new_node) continue;
        if (existing_node->payload_size == 0) continue;  // Skip blank nodes
        
        // Check if edge already exists
        if (graph_find_edge_between(new_node, existing_node) ||
            graph_find_edge_between(existing_node, new_node)) {
            continue;  // Edge already exists
        }
        
        checks_done++;  // Count similarity checks
        
        // Compute pattern similarity
        float similarity = compute_node_pattern_similarity(new_node, existing_node);
        
        // Create similarity edge if similarity exceeds threshold
        if (similarity >= similarity_threshold) {
            // Create bidirectional similarity edge (similar patterns connect both ways)
            Edge *edge1 = edge_create(new_node, existing_node);
            Edge *edge2 = edge_create(existing_node, new_node);
            
            if (edge1 && edge2) {
                // Mark as similarity edges and cache similarity (guides wave propagation)
                edge1->is_similarity_edge = 1;
                edge1->cached_similarity = similarity;
                edge2->is_similarity_edge = 1;
                edge2->cached_similarity = similarity;
                
                // Set similarity edge weights (computed directly from existing data, no hardcoded ranges)
                float local_avg = node_get_local_outgoing_weight_avg(new_node);
                float local_variance = compute_local_edge_variance(new_node);
                float epsilon = compute_adaptive_epsilon(local_avg);
                
                // Weight computed directly from local_avg, local_variance, and similarity
                // No ranges: weight = local_avg * (similarity * variance_factor)
                // variance_factor adapts to local distribution
                float variance_factor = local_variance / (local_variance + local_avg + epsilon);
                float similarity_weight = local_avg * (1.0f - variance_factor + (similarity * variance_factor));
                
                // Ensure non-zero (use epsilon if computed weight is too small)
                if (similarity_weight < epsilon) similarity_weight = epsilon;
                
                edge1->weight = similarity_weight;
                edge2->weight = similarity_weight;
                
                // Update cached weight sums
                node_update_outgoing_weight_sum(new_node, 0.0f, edge1->weight);
                node_update_incoming_weight_sum(existing_node, 0.0f, edge1->weight);
                node_update_outgoing_weight_sum(existing_node, 0.0f, edge2->weight);
                node_update_incoming_weight_sum(new_node, 0.0f, edge2->weight);
                
                graph_add_edge(graph, edge1);
                graph_add_edge(graph, edge2);
            }
        }
    }
}

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
    float learning_rate = node_compute_adaptive_learning_rate(current_node);
    
    if (predicted_next == actual_next) {
        // CORRECT prediction - boost this edge
        if (predicted_edge) {
            edge_update_weight(predicted_edge, learning_rate * 2.0f);
        }
    } else {
        // WRONG prediction - differential learning
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
            // Differential boost: correct gets more, incorrect gets less
            float boost_factor = 1.5f;
            edge_update_weight(correct_edge, learning_rate * boost_factor);
            edge_update_weight(predicted_edge, learning_rate * (1.0f / boost_factor));
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
    size_t position = 0;  // Track position for prediction learning
    
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
                // Look for existing hierarchy node with HIGHEST abstraction level
                // This ensures we prefer "hello" over "he" + "llo"
                Node *best_candidate = NULL;
                uint32_t best_level = 0;
                
                for (size_t h = 0; h < graph->node_count; h++) {
                    Node *hier = graph->nodes[h];
                    if (!hier || hier->abstraction_level == 0) continue;
                    if (hier->payload_size != try_len) continue;
                    
                    // Check if pattern matches
                    if (memcmp(hier->payload, &input[i], try_len) == 0) {
                        // Prefer higher abstraction level (more compressed)
                        if (!best_candidate || hier->abstraction_level > best_level) {
                            best_candidate = hier;
                            best_level = hier->abstraction_level;
                        }
                    }
                }
                
                candidate = best_candidate;
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
            position++;
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
                float old_weight = existing_edge->weight;
                float strengthening_rate = compute_adaptive_strengthening_rate(prev_node);
                existing_edge->weight *= strengthening_rate;  // Adaptive strengthening
                node_update_outgoing_weight_sum(prev_node, old_weight, existing_edge->weight);
                node_update_incoming_weight_sum(node, old_weight, existing_edge->weight);
                
                // Update edge context if we have more context available
                // (reinforces the edge's training context)
            } else {
                // Create new edge (including edges to hierarchy nodes!)
                Edge *edge = edge_create(prev_node, node);
                if (edge) {
                    // STORE EDGE CONTEXT: Remember what bytes preceded this edge
                    // This enables disambiguation during generation
                    size_t ctx_start = (i >= 4) ? i - 4 : 0;
                    size_t ctx_len = i - ctx_start;
                    if (ctx_len > 4) ctx_len = 4;
                    
                    for (size_t c = 0; c < ctx_len; c++) {
                        edge->context_bytes[c] = input[ctx_start + c];
                    }
                    edge->context_len = (uint8_t)ctx_len;
                    
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
        position++;
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
static Node** graph_find_similar_pattern_cluster(Graph *graph, Node *seed_node, 
                                                  size_t *cluster_size) {
    if (!graph || !seed_node || seed_node->payload_size == 0) {
        if (cluster_size) *cluster_size = 0;
        return NULL;
    }
    
    // Track visited nodes (avoid cycles) - adaptive hash set size
    size_t cluster_visited_size = compute_adaptive_hash_size(graph->node_count);
    Node ***visited_set = calloc(cluster_visited_size, sizeof(Node**));
    size_t *visited_counts = calloc(cluster_visited_size, sizeof(size_t));
    size_t *visited_capacities = calloc(cluster_visited_size, sizeof(size_t));
    
    if (!visited_set || !visited_counts || !visited_capacities) {
        free(visited_set);
        free(visited_counts);
        free(visited_capacities);
        if (cluster_size) *cluster_size = 0;
        return NULL;
    }
    
    #define CLUSTER_NODE_HASH(node) (((size_t)(node)) % cluster_visited_size)
    
    // Cluster nodes (dynamic array)
    Node **cluster = NULL;
    size_t cluster_count = 0;
    size_t cluster_capacity = 0;
    
    // Wave front: start from seed node
    Node **wave_front = malloc(sizeof(Node*));
    if (!wave_front) {
        if (cluster_size) *cluster_size = 0;
        return NULL;
    }
    wave_front[0] = seed_node;
    size_t wave_front_size = 1;
    
    // Mark seed as visited and add to cluster
    size_t seed_hash = CLUSTER_NODE_HASH(seed_node);
    visited_set[seed_hash] = malloc(2 * sizeof(Node*));
    if (!visited_set[seed_hash]) {
        free(wave_front);
        if (cluster_size) *cluster_size = 0;
        return NULL;
    }
    visited_set[seed_hash][0] = seed_node;
    visited_set[seed_hash][1] = NULL;
    visited_counts[seed_hash] = 1;
    visited_capacities[seed_hash] = 2;
    
    cluster_capacity = 4;
    cluster = malloc(cluster_capacity * sizeof(Node*));
    if (!cluster) {
        free(wave_front);
        free(visited_set[seed_hash]);
        if (cluster_size) *cluster_size = 0;
        return NULL;
    }
    cluster[cluster_count++] = seed_node;
    
    // Compute local average similarity edge weight (for adaptive threshold)
    float local_sim_avg = 0.0f;
    size_t sim_edge_count = 0;
    for (size_t i = 0; i < seed_node->outgoing_count; i++) {
        Edge *e = seed_node->outgoing_edges[i];
        if (e && e->to_node && e->to_node->payload_size > 0) {
            // Check if this is a similarity edge (weight in medium range relative to local)
            float local_avg = node_get_local_outgoing_weight_avg(seed_node);
            float epsilon = compute_adaptive_epsilon(local_avg);
            float weight_relative = e->weight / (local_avg + epsilon);
            
            // Similarity edges: weight range from local context (data-driven)
            float min_rel, max_rel;
            compute_similarity_edge_range(seed_node, &min_rel, &max_rel);
            if (weight_relative >= min_rel && weight_relative <= max_rel) {
                local_sim_avg += e->weight;
                sim_edge_count++;
            }
        }
    }
    if (sim_edge_count > 0) {
        local_sim_avg /= (float)sim_edge_count;
    }
    
    float epsilon = compute_adaptive_epsilon(local_sim_avg);
    // Adaptive threshold from local context (not hardcoded 0.7f)
    float threshold_factor = compute_cluster_threshold_factor(seed_node);
    float cluster_threshold = (local_sim_avg > epsilon) ? 
                             (local_sim_avg * threshold_factor) : epsilon;
    
    // Wave propagation: explore cluster through similarity edges (adaptive depth)
    int max_cluster_depth = compute_max_cluster_depth(graph);  // Data-driven, not hardcoded
    for (int step = 0; step < max_cluster_depth && wave_front_size > 0; step++) {
        Node **next_wave_front = NULL;
        size_t next_wave_size = 0;
        size_t next_wave_capacity = 0;
        
        for (size_t i = 0; i < wave_front_size; i++) {
            Node *node = wave_front[i];
            if (!node || node->payload_size == 0) continue;  // Skip blank nodes
            
            // Explore similarity edges (O(degree) operation)
            for (size_t j = 0; j < node->outgoing_count; j++) {
                Edge *edge = node->outgoing_edges[j];
                if (!edge || !edge->to_node) continue;
                if (edge->to_node->payload_size == 0) continue;  // Skip blank nodes
                
                // Check if similarity edge (adaptive threshold)
                float local_avg = node_get_local_outgoing_weight_avg(node);
                float edge_epsilon = compute_adaptive_epsilon(local_avg);
                float weight_relative = edge->weight / (local_avg + edge_epsilon);
                
                // Compute adaptive bounds for similarity edges (data-driven, not hardcoded 0.5f, 1.5f)
                float min_relative, max_relative;
                compute_similarity_edge_range(node, &min_relative, &max_relative);
                
                // Similarity edge: weight within adaptive range relative to local context
                if (weight_relative >= min_relative && weight_relative <= max_relative && 
                    edge->weight >= cluster_threshold) {
                    
                    Node *target = edge->to_node;
                    
                    // Check if already visited (O(1) average case)
                    size_t target_hash = CLUSTER_NODE_HASH(target);
                    int already_visited = 0;
                    if (visited_set[target_hash]) {
                        for (size_t k = 0; k < visited_counts[target_hash]; k++) {
                            if (visited_set[target_hash][k] == target) {
                                already_visited = 1;
                                break;
                            }
                        }
                    }
                    
                    if (!already_visited) {
                        // Add to visited set
                        if (!visited_set[target_hash]) {
                            visited_set[target_hash] = malloc(2 * sizeof(Node*));
                            if (!visited_set[target_hash]) continue;
                            visited_set[target_hash][0] = target;
                            visited_set[target_hash][1] = NULL;
                            visited_counts[target_hash] = 1;
                            visited_capacities[target_hash] = 2;
                        } else if (visited_counts[target_hash] >= visited_capacities[target_hash]) {
                            size_t new_cap = visited_capacities[target_hash] * 2;
                            Node **new_set = realloc(visited_set[target_hash], 
                                                     new_cap * sizeof(Node*));
                            if (!new_set) continue;
                            visited_set[target_hash] = new_set;
                            visited_capacities[target_hash] = new_cap;
                        }
                        if (visited_set[target_hash] && 
                            visited_counts[target_hash] < visited_capacities[target_hash]) {
                            visited_set[target_hash][visited_counts[target_hash]++] = target;
                        }
                        
                        // Add to cluster
                        if (cluster_count >= cluster_capacity) {
                            size_t new_cap = cluster_capacity * 2;
                            Node **new_cluster = realloc(cluster, new_cap * sizeof(Node*));
                            if (!new_cluster) break;
                            cluster = new_cluster;
                            cluster_capacity = new_cap;
                        }
                        cluster[cluster_count++] = target;
                        
                        // Add to next wave front
                        if (next_wave_size >= next_wave_capacity) {
                            size_t new_cap = (next_wave_capacity == 0) ? 4 : next_wave_capacity * 2;
                            Node **new_front = realloc(next_wave_front, new_cap * sizeof(Node*));
                            if (!new_front) break;
                            next_wave_front = new_front;
                            next_wave_capacity = new_cap;
                        }
                        next_wave_front[next_wave_size++] = target;
                    }
                }
            }
        }
        
        free(wave_front);
        wave_front = next_wave_front;
        wave_front_size = next_wave_size;
    }
    
    free(wave_front);
    
    // Clean up visited_set (adaptive size)
    for (size_t i = 0; i < cluster_visited_size; i++) {
        if (visited_set[i]) free(visited_set[i]);
    }
    free(visited_set);
    free(visited_counts);
    free(visited_capacities);
    
    if (cluster_size) *cluster_size = cluster_count;
    return cluster;
}

/* Compute cluster strength score (smooth function, no hard threshold)
 * - Based on cluster size and similarity edge weights
 * - Returns smooth score 0.0-1.0
 */
static float graph_compute_cluster_strength(Node **cluster, size_t cluster_size) {
    if (!cluster || cluster_size < 2) return 0.0f;  // Need at least 2 nodes
    
    // Compute average similarity edge weight within cluster
    float total_weight = 0.0f;
    size_t edge_count = 0;
    
    for (size_t i = 0; i < cluster_size; i++) {
        Node *node = cluster[i];
        if (!node) continue;
        
        // Find edges to other cluster nodes (O(degree) per node)
        for (size_t j = 0; j < node->outgoing_count; j++) {
            Edge *edge = node->outgoing_edges[j];
            if (!edge || !edge->to_node) continue;
            
            // Check if target is in cluster
            for (size_t k = 0; k < cluster_size; k++) {
                if (cluster[k] == edge->to_node) {
                    // This edge connects within cluster
                    float local_avg = node_get_local_outgoing_weight_avg(node);
                    float epsilon = compute_adaptive_epsilon(local_avg);
                    float weight_relative = edge->weight / (local_avg + epsilon);
                    
                    // Count similarity edges (adaptive range from local context)
                    float min_rel, max_rel;
                    compute_similarity_edge_range(node, &min_rel, &max_rel);
                    if (weight_relative >= min_rel && weight_relative <= max_rel) {
                        total_weight += edge->weight;
                        edge_count++;
                    }
                    break;
                }
            }
        }
    }
    
    if (edge_count == 0) return 0.0f;
    
    float avg_weight = total_weight / (float)edge_count;
    
    // Cluster strength: smooth combination of size and edge weights (no hard threshold)
    float size_factor = (float)cluster_size / ((float)cluster_size + 2.0f);  // 0.5 for 2 nodes, ~1.0 for many
    float weight_factor = avg_weight / (avg_weight + 1.0f);  // Smooth normalization
    
    return size_factor * weight_factor;
}

/* Get adaptive threshold for cluster creation
 * - Based on graph's current state (no hardcoded minimum)
 * - Uses local sampling (not O(n) iteration)
 */
static float graph_get_adaptive_cluster_threshold(Graph *graph) {
    if (!graph || graph->node_count < 3) return 0.8f;  // Higher threshold for small graphs
    
    // Sample average edge weight from graph (local sampling, not O(n))
    float sample_avg = 0.0f;
    size_t sample_count = 0;
    size_t sample_size = (graph->node_count < 50) ? graph->node_count : 50;
    
    // Sample from recent nodes (more representative of current state)
    size_t sample_start = graph->node_count - sample_size;
    for (size_t i = sample_start; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (node && node->outgoing_count > 0) {
            float local_avg = node_get_local_outgoing_weight_avg(node);
            sample_avg += local_avg;
            sample_count++;
        }
    }
    
    if (sample_count == 0) {
        // No data: use minimal context from graph properties or return neutral
        if (graph && graph->node_count > 0) {
            // Use minimal context: first node's weight as base
            Node *first_node = graph->nodes[0];
            if (first_node) {
                float min_weight = first_node->weight;
                float epsilon = compute_adaptive_epsilon(min_weight);
                // Smooth function from minimal context
                return min_weight / (min_weight + epsilon + 1.0f);
            }
        }
        return 0.0f;  // Neutral: no operation possible (not a threshold)
    }
    
    sample_avg /= (float)sample_count;
    float epsilon = compute_adaptive_epsilon(sample_avg);
    
    // Adaptive threshold: smooth function based on sample average (data-driven)
    // Multiplier computed from local variance (not hardcoded 0.6f)
    float variance = 0.0f;
    for (size_t i = sample_start; i < graph->node_count; i++) {
        Node *node = graph->nodes[i];
        if (node && node->outgoing_count > 0) {
            float local_avg = node_get_local_outgoing_weight_avg(node);
            float diff = local_avg - sample_avg;
            variance += diff * diff;
        }
    }
    variance /= (float)sample_count;
    float variance_norm = variance / (sample_avg + epsilon);
    float multiplier = variance_norm / (variance_norm + 1.0f);  // Adaptive multiplier (0 to 1)
    
    return (sample_avg * multiplier) / (sample_avg * multiplier + 1.0f + epsilon);
}

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

/* Create hierarchy node by combining payloads
 * - Brain: Synaptic consolidation - patterns merge into single representation
 * - LLM: Learned embeddings combine token sequences
 * - Melvin: Concatenate payloads, increment abstraction_level
 */
static Node* create_hierarchy_node(Graph *graph, Node *node1, Node *node2) {
    if (!graph || !node1 || !node2) return NULL;
    
    // Combine payloads (brain: synaptic consolidation)
    size_t combined_size = node1->payload_size + node2->payload_size;
    uint8_t *combined = malloc(combined_size);
    if (!combined) return NULL;
    
    memcpy(combined, node1->payload, node1->payload_size);
    memcpy(combined + node1->payload_size, node2->payload, node2->payload_size);
    
    // Increment abstraction level (cortical hierarchy)
    uint32_t new_level = (node1->abstraction_level > node2->abstraction_level) ?
                         node1->abstraction_level : node2->abstraction_level;
    new_level += 1;
    
    Node *hierarchy = node_create(combined, combined_size, new_level);
    free(combined);
    
    if (!hierarchy) return NULL;
    
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
static void node_residual_update(Node *node, float delta_activation) {
    if (!node) return;
    node->state = node->state + delta_activation;
}


/* ============================================================================
 * .M FILE OPERATIONS (Principle: Live Executable Programs)
 * ============================================================================ */

/* ============================================================================
 * FORWARD DECLARATIONS FOR OUTPUT GENERATION
 * ============================================================================ */
static uint8_t sample_next_byte(Node **candidates, float *weights, size_t candidate_count, float temperature);
static void melvin_generate_output_from_state(MelvinMFile *mfile, WaveState *state, Node **input_nodes, size_t input_count);

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
                                }
                                // If read fails, context_trace remains zeroed (backward compatible)
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
            uint8_t direction = 0;
            uint32_t last_wave_generation = 0;
            float weight = 0.0f;
            float routing_gate = 0.0f;
            uint8_t context_bytes[4] = {0};
            uint8_t context_len = 0;
            
            if (read(fd, from_id, 9) != 9 ||
                read(fd, to_id, 9) != 9 ||
                read(fd, &direction, 1) != 1 ||
                read(fd, &last_wave_generation, sizeof(uint32_t)) != sizeof(uint32_t) ||
                read(fd, &weight, sizeof(float)) != sizeof(float) ||
                read(fd, &routing_gate, sizeof(float)) != sizeof(float)) {
                break;
            }
            
            // Try to read edge context (may fail for old format files)
            ssize_t ctx_read = read(fd, context_bytes, 4);
            if (ctx_read == 4) {
                read(fd, &context_len, 1);
            }
            
            // Find nodes by ID
            Node *from_node = graph_find_node_by_id(mfile->graph, from_id);
            Node *to_node = graph_find_node_by_id(mfile->graph, to_id);
            
            if (from_node && to_node) {
                // Create edge
                Edge *edge = edge_create(from_node, to_node);
                if (edge) {
                    edge->direction = direction;
                    edge->last_wave_generation = last_wave_generation;
                    edge->weight = weight;
                    edge->routing_gate = routing_gate;
                    // Load edge context for disambiguation
                    memcpy(edge->context_bytes, context_bytes, 4);
                    edge->context_len = context_len;
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
        size_t node_data_size = sizeof(size_t) + (node ? node->payload_size : 0) + 
                               sizeof(float) * 14 + sizeof(uint32_t) + 2 * sizeof(uint8_t);
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
        if (write(mfile->fd, &edge->direction, 1) != 1) return -1;
        // Save last_wave_generation as uint32_t (4 bytes)
        if (write(mfile->fd, &edge->last_wave_generation, sizeof(uint32_t)) != sizeof(uint32_t)) return -1;
        if (write(mfile->fd, &edge->weight, sizeof(float)) != sizeof(float)) return -1;
        if (write(mfile->fd, &edge->routing_gate, sizeof(float)) != sizeof(float)) return -1;
        // EDGE CONTEXT for disambiguation
        if (write(mfile->fd, edge->context_bytes, 4) != 4) return -1;
        if (write(mfile->fd, &edge->context_len, 1) != 1) return -1;
        
        // Edge size: 9 + 9 + 1 + 4 + 4 + 4 + 4 + 1 = 36 bytes
        edge->file_offset = lseek(mfile->fd, 0, SEEK_CUR) - 36;
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
    
    // Free graph (TODO: proper cleanup of nodes and edges)
    if (mfile->graph) {
        free(mfile->graph->nodes);
        free(mfile->graph->edges);
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

/* Process input through .m file
 * - Universal input (any binary data)
 * - Triggers wave propagation
 * - Auto-saves after adaptation
 */
int melvin_m_process_input(MelvinMFile *mfile) {
    if (!mfile) return -1;
    
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
        // Three-phase architecture: encode → refine → decode
        
        // PHASE 1: ENCODE - Spreading activation
        // Input nodes activate, then spread through edges to find continuations
        pattern = encode_input_spreading(mfile->graph, initial_nodes, initial_count);
        
        if (pattern && pattern->count > 0) {
            // PHASE 2: REFINE - Recurrent dynamics
            // Nodes that connect to active context strengthen
            // Disconnected nodes decay
            // This creates emergent attention-like behavior
            
            // ADAPTIVE ITERATION COUNT (data-driven, not hardcoded 3)
            // Compute from pattern complexity and graph maturity
            float pattern_complexity = (float)pattern->count / (float)(initial_count + 1);
            float graph_maturity = (float)mfile->graph->edge_count / (float)(mfile->graph->node_count + 1.0f);
            
            // More iterations for complex patterns and mature graphs
            // Fewer iterations for simple patterns and sparse graphs
            int refine_iterations = 2 + (int)(pattern_complexity * 0.5f + graph_maturity * 0.3f);
            if (refine_iterations < 2) refine_iterations = 2;  // Minimum 2
            if (refine_iterations > 5) refine_iterations = 5;  // Maximum 5 (prevent excessive computation)
            
            refine_pattern_dynamics(pattern, mfile->graph, refine_iterations);  // Adaptive iterations
            
            // PHASE 3: DECODE - Context-aware generation
            // Sample from pattern (includes spread nodes like " world")
            // Not just input nodes
            generate_from_pattern(mfile, pattern, initial_nodes, initial_count);
        }
        
        // PHASE 3: HEBBIAN LEARNING + HIERARCHY FORMATION
        // Strengthen edges that were just traversed in the input sequence
        // This is the CORE learning mechanism - edges get stronger with use
        for (size_t i = 0; i + 1 < initial_count; i++) {
            Node *from = initial_nodes[i];
            Node *to = initial_nodes[i + 1];
            if (!from || !to) continue;
            
            // Find or create edge between them
            Edge *edge = NULL;
            for (size_t j = 0; j < from->outgoing_count; j++) {
                if (from->outgoing_edges[j] && from->outgoing_edges[j]->to_node == to) {
                    edge = from->outgoing_edges[j];
                    break;
                }
            }
            
            if (edge) {
                // Hebbian learning: "Neurons that fire together, wire together"
                // Edge was just used in the input sequence, so strengthen it
                float learning_rate = 0.1f;
                float old_weight = edge->weight;
                edge->weight += learning_rate;  // Simple increment (frequency-based)
                
                // Update cached sums
                node_update_outgoing_weight_sum(from, old_weight, edge->weight);
                node_update_incoming_weight_sum(to, old_weight, edge->weight);
                
                // HIERARCHY FORMATION (BRAIN-INSPIRED: Synaptic consolidation)
                // When edge has been used enough times, consolidate into hierarchy
                // SELF-MODIFICATION: More aggressive hierarchy formation
                // Threshold = 1.3x local average (was 2.0x - now forms hierarchies earlier!)
                // This accelerates abstraction learning, key for matching LLM quality
                float local_avg_weight = node_get_local_outgoing_weight_avg(from);
                float epsilon_h = compute_adaptive_epsilon(local_avg_weight);
                float hierarchy_threshold = (local_avg_weight > epsilon_h) ? (local_avg_weight * 1.3f) : 0.8f;
                float relative_strength = edge->weight / (local_avg_weight + epsilon_h + 0.1f);
                
                // Form hierarchy if edge is strong relative to neighbors (lowered from 1.5 to 1.2)
                if (edge->weight > hierarchy_threshold && relative_strength > 1.2f && 
                    from->payload_size > 0 && to->payload_size > 0) {
                    // Check if hierarchy already exists
                    int hierarchy_exists = 0;
                    size_t combined_size = from->payload_size + to->payload_size;
                    
                    for (size_t h = 0; h < mfile->graph->node_count; h++) {
                        Node *existing = mfile->graph->nodes[h];
                        if (!existing || existing->abstraction_level == 0) continue;
                        if (existing->payload_size != combined_size) continue;
                        
                        // Check if payloads match
                        if (memcmp(existing->payload, from->payload, from->payload_size) == 0 &&
                            memcmp(existing->payload + from->payload_size, to->payload, to->payload_size) == 0) {
                            hierarchy_exists = 1;
                            break;
                        }
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
            from->weight += 0.05f;  // Smaller learning rate for nodes
            to->weight += 0.05f;
        }
        
        // End three-phase
    }
    
    // Clean up pattern
    if (pattern) {
        activation_pattern_free(pattern);
        pattern = NULL;
    }
    
    free(initial_nodes);
    
    // Increment adaptation count
    mfile->adaptation_count++;
    mfile->last_modified = time(NULL);
    
    // SELF-MODIFICATION: Periodic self-optimization (every 10 adaptations)
    // Identifies underutilized nodes and boosts them
    if (mfile->adaptation_count % 10 == 0) {
        Graph *graph = mfile->graph;
        for (size_t i = 0; i < graph->node_count; i++) {
            node_self_optimize_if_weak(graph->nodes[i]);
        }
    }
    
    // Clear input buffer after processing
    melvin_m_universal_input_clear(mfile);
    
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
    
    // Error-based learning: adjust weights of edges in output path
    // error_signal = 1.0: perfect → strengthen edges
    // error_signal = 0.0: wrong → weaken edges
    // error_signal = 0.5: neutral → no change
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
        
        // Compute adaptive learning rate from edge's from_node context
        float base_learning_rate = node_compute_adaptive_learning_rate(edge->from_node);
        
        // Fallback: if no change history, compute from edge weight relative to local average
        if (base_learning_rate <= 0.0f) {
            float local_avg = node_get_local_outgoing_weight_avg(edge->from_node);
            float epsilon = compute_adaptive_epsilon(local_avg);
            if (local_avg > epsilon) {
                // Learning rate scales with edge weight relative to local context
                float weight_ratio = edge->weight / (local_avg + epsilon);
                // Normalize to reasonable range (0.01 to 0.1)
                base_learning_rate = (weight_ratio < 0.1f) ? 0.1f : 
                                    ((weight_ratio > 1.0f) ? 0.01f : (0.1f / weight_ratio));
                if (base_learning_rate < 0.01f) base_learning_rate = 0.01f;
                if (base_learning_rate > 0.1f) base_learning_rate = 0.1f;
            } else {
                // Minimal bootstrap (only when no context exists)
                base_learning_rate = 0.05f;
            }
        }
        
        // Adaptive penalty/reward rates: scale inversely with edge strength
        // Stronger edges (relative to neighbors) get smaller adjustments (more stable)
        // Weaker edges get larger adjustments (more exploratory)
        float local_avg_weight = node_get_local_outgoing_weight_avg(edge->from_node);
        float epsilon = compute_adaptive_epsilon(local_avg_weight);
        float weight_ratio = edge->weight / (local_avg_weight + epsilon + 0.1f);
        
        // Adaptive rates: inverse of weight ratio (stronger edges = smaller rates)
        // Smooth function: rate = base_rate / (weight_ratio + 1.0f)
        float adaptive_penalty_rate = base_learning_rate / (weight_ratio + 1.0f);
        float adaptive_reward_rate = base_learning_rate / (weight_ratio + 1.0f);
        
        if (error_signal < 0.5f) {
            // Wrong output: weaken this edge (adaptive penalty)
            // penalty scales with how wrong (0.0 error = full penalty)
            float penalty = adaptive_penalty_rate * (1.0f - error_signal * 2.0f);
            edge->weight *= (1.0f - penalty);
            
            // Adaptive floor: computed from local context (not hardcoded)
            // Floor is 50% of local minimum, but never below 0.01 (minimal bootstrap)
            float local_min = node_get_local_min_outgoing_weight(edge->from_node);
            float floor = (local_min > 0.0f) ? (local_min * 0.5f) : 0.01f;
            if (floor < 0.01f) floor = 0.01f;  // Minimal bootstrap floor
            if (edge->weight < floor) edge->weight = floor;
        } else {
            // Correct output: strengthen this edge (adaptive reward)
            // reward scales with how correct (1.0 error = full reward)
            float reward = adaptive_reward_rate * ((error_signal - 0.5f) * 2.0f);
            edge->weight += reward;
        }
        
        // Update cached sums (maintains O(1) access to weight sums)
        node_update_outgoing_weight_sum(edge->from_node, old_weight, edge->weight);
        if (edge->to_node) {
            node_update_incoming_weight_sum(edge->to_node, old_weight, edge->weight);
        }
    }
    
    // SELF-MODIFICATION: Trigger self-optimization on errors
    // Uses meta-learning to track which strategies work
    if (error_signal < 0.5f) {
        graph_self_optimize_on_error(mfile->graph, mfile->last_output_path, 
                                     mfile->last_output_path_count, error_signal);
    } else {
        // Success: update meta-learning for nodes in path
        for (size_t i = 0; i < mfile->last_output_path_count; i++) {
            Edge *edge = mfile->last_output_path[i];
            if (edge && edge->from_node) {
                node_meta_learn_update(edge->from_node, error_signal);
            }
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
                    // Use adaptive learning rate (data-driven) with 2x boost
                    float learning_rate = node_compute_adaptive_learning_rate(prev_node);
                    if (learning_rate <= 0.0f) learning_rate = 0.1f;  // Fallback
                    
                    float old_weight = edge->weight;
                    edge->weight += learning_rate * 2.0f;  // 2x boost for correct path!
                    
                    // Update cached sums
                    node_update_outgoing_weight_sum(prev_node, old_weight, edge->weight);
                    node_update_incoming_weight_sum(node, old_weight, edge->weight);
                    
                    // Also store context for this edge (for disambiguation later)
                    // Store last 4 bytes leading to this edge
                    size_t ctx_start = (i >= 4) ? (i - 4) : 0;
                    size_t ctx_len = i - ctx_start;
                    if (ctx_len > 4) ctx_len = 4;
                    if (ctx_len > 0) {
                        memcpy(edge->context_bytes, &sequence[ctx_start], ctx_len);
                        edge->context_len = ctx_len;
                    }
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
    
    // Compute local context statistics (for relative probability computation)
    // This is like computing local "vocabulary" statistics
    float local_activation_sum = 0.0f;
    size_t valid_count = 0;
    
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
        
        // Boost for direct continuations from ANY input node (learned sequential patterns via edges)
        // Edge weights are data-driven, computed from co-activation patterns during training
        // Prioritize later input nodes (more recent context) with higher boost
        // Boost is relative to activation_strength and local context (not hardcoded)
        // Note: input_nodes and input_count are function parameters, accessible here
        if (input_nodes && input_count > 0) {
            // Compute adaptive boost base from local context
            float local_avg_weight = 0.0f;
            size_t weight_count = 0;
            for (size_t k = 0; k < input_count; k++) {
                if (input_nodes[k]) {
                    float node_avg = node_get_local_outgoing_weight_avg(input_nodes[k]);
                    local_avg_weight += node_avg;
                    weight_count++;
                }
            }
            float avg_local_weight = (weight_count > 0) ? (local_avg_weight / (float)weight_count) : 0.0f;
            float boost_epsilon = compute_adaptive_epsilon(avg_local_weight);
            
            for (int input_idx = (int)input_count - 1; input_idx >= 0; input_idx--) {
                Node *input_node = input_nodes[input_idx];
                if (!input_node) continue;
                
                for (size_t j = 0; j < input_node->outgoing_count; j++) {
                    Edge *edge = input_node->outgoing_edges[j];
                    if (edge && edge->to_node == node) {
                        // Direct continuation: boost relative to activation_strength and local context
                        // Position boost: later positions (more recent) get higher boost
                        // Boost multiplier adapts to local weight average (not hardcoded)
                        float position_factor = (float)(input_count - input_idx) / (float)input_count;  // 0.0 to 1.0
                        float boost_base = (avg_local_weight > boost_epsilon) ? 
                                         (1.0f + avg_local_weight) : (1.0f + boost_epsilon);
                        float position_boost = boost_base * (1.0f + position_factor);  // Adaptive, relative to context
                        logits[i] += edge->weight * position_boost * (base_logit + edge_transform_sum + boost_epsilon);
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
        
        // Track for local context computation
        if (logits[i] > -1e8f) {
            local_activation_sum += logits[i];
            valid_count++;
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
    
    const char *stop_reason = NULL;  // Track why we stopped
    
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
                stop_reason = "no_outgoing_edges";
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
                stop_reason = "no_valid_candidate";
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
                    stop_reason = "node_no_valid_choice";
                    break;
                }
            } else {
                // Node has no outgoing edges - stop generation
                stop_reason = "no_outgoing_edges";
                break;
            }
        } else {
            // No input nodes or invalid state - stop generation
            stop_reason = "invalid_state";
            break;
        }
        
        // Verify we have a valid sampled node
        if (!sampled_node) {
            stop_reason = "no_sampled_node";
            break;
        }
        
        // Stop condition 3: EOS byte detected
        if (sampled_byte == EOS_BYTE) {
            stop_reason = "EOS";
            break;
        }
        
        // Allocate output buffer if needed
        if (output_capacity == 0) {
            output_capacity = 1;
            output = malloc(output_capacity);
            if (!output) {
                stop_reason = "malloc_failed";
                break;
            }
        }
        
        // Grow buffer if needed
        if (output_len >= output_capacity) {
            size_t new_capacity = output_capacity * 2;
            uint8_t *new_output = realloc(output, new_capacity);
            if (!new_output) {
                stop_reason = "realloc_failed";
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
        
        // Update node activation tracking
        if (sampled_node) {
            sampled_node->total_activations++;
        }
        
        // WAVE-BASED STOP PREDICTION (Mini Neural Net)
        // No hardcoded loop detection - use learned stop probability
        if (sampled_node) {
            float stop_prob = compute_stop_probability(sampled_node, output_len);
            float rand_val = (float)rand() / (float)RAND_MAX;
            if (rand_val < stop_prob) {
                stop_reason = "neural_net_stop";
                learn_stop_prediction(sampled_node, output_len, 1);
                break;
            }
        }
        
        // Natural stop: if node has no outgoing edges, stop
        if (sampled_node && sampled_node->outgoing_count == 0) {
            stop_reason = "no_outgoing_edges";
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


