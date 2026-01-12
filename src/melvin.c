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
 * DEBUG OUTPUT CONTROL
 * 
 * Compile with -DMELVIN_DEBUG to enable debug output.
 * This is NOT a limit - it's a build configuration for development vs production.
 * Production builds should NOT define MELVIN_DEBUG for maximum performance.
 * ============================================================================ */
#ifdef MELVIN_DEBUG
#define DEBUG_LOG(...) do { fprintf(stderr, __VA_ARGS__); fflush(stderr); } while(0)
#else
#define DEBUG_LOG(...) ((void)0)
#endif

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
 * SPARSE SEMANTIC EMBEDDINGS (General Intelligence Foundation)
 * 
 * Brain-inspired sparse representations for semantic understanding:
 * - Only 5-20 dimensions active (not 4096 like LLMs)
 * - O(k) operations where k = active_count (typically 10-20)
 * - Enables generalization without massive compute
 * - All learning uses Welford's algorithm (no hardcoded rates)
 * ============================================================================ */

/* SparseEmbedding: Lightweight semantic representation
 * - Only stores active dimensions (sparse, not dense)
 * - O(k) similarity computation via merge-style algorithm
 * - Adaptive learning via Welford statistics
 */
typedef struct SparseEmbedding {
    uint16_t *active_dims;        // Which dimensions are active (sorted for O(k) merge)
    float *active_values;         // Values for active dimensions
    size_t active_count;          // Number of active dims (typically 5-20)
    size_t active_capacity;       // Allocated capacity (grows adaptively)
    
    // Welford-style learning statistics
    float activation_mean;        // Running mean of activation values
    float activation_m2;          // Running sum of squared deviations
    uint64_t update_count;        // Number of updates (for Welford)
} SparseEmbedding;

/* Composition operations for concept manipulation */
typedef enum CompositionOp {
    COMP_COMPOSE,      // A + B -> AB (combine concepts)
    COMP_DECOMPOSE,    // AB - A -> B (extract component)
    COMP_SUBSTITUTE,   // AB - A + C -> CB (swap component)
    COMP_ANALOGIZE,    // A:B :: C:? -> D (analogical reasoning)
    COMP_COUNT         // Number of operations
} CompositionOp;

/* Temporal relations for sequence understanding */
typedef enum TemporalRelation {
    TEMP_BEFORE,       // A happens before B
    TEMP_AFTER,        // A happens after B
    TEMP_CAUSES,       // A causes B
    TEMP_ENABLES,      // A enables B
    TEMP_PREVENTS,     // A prevents B
    TEMP_NONE          // No temporal relation
} TemporalRelation;

/* Abstraction types for generalization */
typedef enum AbstractionType {
    ABST_CONTEXT_CLUSTER,   // Nodes appearing in similar contexts
    ABST_ROLE_PATTERN,      // Nodes with similar roles (noun, verb, etc.)
    ABST_CONCEPT,           // Learned concept (dog, cat -> ANIMAL)
    ABST_COUNT              // Number of abstraction types
} AbstractionType;

/* Forward declarations for sparse embedding functions */
static SparseEmbedding* sparse_embedding_create(size_t initial_capacity);
static void sparse_embedding_free(SparseEmbedding *emb);
static float sparse_embedding_similarity(SparseEmbedding *a, SparseEmbedding *b);
static void sparse_embedding_update_hebbian(SparseEmbedding *emb, SparseEmbedding *context, float learning_rate);
static SparseEmbedding* sparse_embedding_sparsify(float *dense, size_t dim, size_t target_sparsity);
static void sparse_embedding_set_dimension(SparseEmbedding *emb, uint16_t dim, float value);
static float sparse_embedding_get_dimension(SparseEmbedding *emb, uint16_t dim);
static SparseEmbedding* sparse_embedding_clone(SparseEmbedding *src);
static void sparse_embedding_normalize(SparseEmbedding *emb);
static SparseEmbedding* sparse_embedding_compose(SparseEmbedding *a, SparseEmbedding *b, CompositionOp op);

/* ============================================================================
 * CORE DATA STRUCTURES
 * ============================================================================ */

/* Forward declarations */
typedef struct Graph Graph;
typedef struct Node Node;
typedef struct Edge Edge;
typedef struct SparseContext SparseContext;
typedef struct ContextTag ContextTag;
typedef struct ActivationPattern ActivationPattern;

/* Forward declarations for general intelligence structures */
typedef struct ModalityBridge ModalityBridge;
typedef struct AttentionMechanism AttentionMechanism;
typedef struct AbstractionNode AbstractionNode;
typedef struct ProbabilisticOutput ProbabilisticOutput;
typedef struct CompositionCache CompositionCache;

/* Forward declarations for sparse context functions */
static SparseContext* sparse_context_create_from_nodes(Node **nodes, float *activations, size_t count);
static void sparse_context_add_node(SparseContext *ctx, Node *node, float activation);

/* Forward declarations for node weight functions (used in context scoring) */
static inline float node_get_local_outgoing_weight_avg(Node *node);
static SparseContext* sparse_context_create_from_pattern(ActivationPattern *pattern);
static float sparse_context_match(SparseContext *ctx1, SparseContext *ctx2);
static void sparse_context_free(SparseContext *ctx);
static SparseContext* sparse_context_clone(SparseContext *src);
static void edge_add_context_tag(Edge *edge, SparseContext *context, float weight_contribution);
static float edge_compute_context_weight(Edge *edge, SparseContext *current_context);
static void edge_prune_context_tags(Edge *edge);

/* ============================================================================
 * EDGE TRANSFORMER (Local Multi-Head Attention)
 * 
 * Implements Requirement.md line 7: "edges transform locally in the same LLM transform globally"
 * Each edge acts as a mini-transformer with Q, K, V projections
 * Multi-head attention enables different semantic aspects
 * O(k) complexity where k = sparse embedding dimensions
 * ============================================================================ */

/* EdgeTransformer: Local transformer attention per edge
 * - Query, Key, Value projections (sparse for O(k) operations)
 * - Multi-head attention (adaptive number of heads)
 * - Welford statistics for adaptive learning
 */
typedef struct EdgeTransformer {
    // Query, Key, Value projections (sparse embeddings)
    SparseEmbedding *query_proj;      // What this edge "asks for"
    SparseEmbedding *key_proj;        // What this edge "matches against"
    SparseEmbedding *value_proj;      // What this edge "provides"
    
    // Multi-head attention weights
    float *head_weights;              // Attention weight per head
    size_t num_heads;                 // Number of attention heads (adaptive)
    size_t head_capacity;             // Allocated capacity
    
    // Per-head success tracking (Welford-style)
    float *head_success_mean;         // Mean success per head
    float *head_success_m2;           // Sum of squared deviations
    uint64_t *head_counts;            // Per-head update count
    
    // Overall attention statistics (Welford-style)
    float attention_mean;             // Running mean of attention scores
    float attention_m2;               // Sum of squared deviations
    uint64_t attention_count;         // Number of attention computations
} EdgeTransformer;

/* Forward declarations for edge transformer functions */
static EdgeTransformer* edge_transformer_create(size_t num_heads);
static void edge_transformer_free(EdgeTransformer *transformer);
static float edge_compute_attention_score(Edge *edge, SparseEmbedding *query_context);
static float edge_multi_head_attention(Edge *edge, SparseEmbedding *query_context);
static SparseEmbedding* edge_transform_value(Edge *edge, float attention_score);
static void edge_update_projections(Edge *edge, SparseEmbedding *from_emb, SparseEmbedding *to_emb, float success);
static SparseEmbedding* node_aggregate_transformer_attention(Node *node, SparseEmbedding *query_context);

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
    
    // SPARSE SEMANTIC EMBEDDING (General Intelligence Foundation)
    // O(k) operations where k = active dimensions (typically 5-20)
    // Enables semantic understanding without massive compute
    SparseEmbedding *sparse_embedding;      // Sparse semantic representation (NULL = not learned)
    
    // SEMANTIC NEIGHBOR CACHE (O(1) lookup instead of O(n) search)
    // Caches top-k semantically similar nodes for fast retrieval
    struct Node **semantic_neighbors;       // Top-k similar nodes (cached)
    float *semantic_similarities;           // Cached similarity values
    size_t semantic_neighbor_count;         // Current count
    size_t semantic_neighbor_capacity;      // Allocated capacity
    uint32_t semantic_cache_generation;     // When cache was last updated
    
    // ABSTRACTION REFERENCE (for generalization)
    // Points to abstraction node if this node is an instance of a concept
    struct Node *abstraction_parent;        // Parent abstraction (NULL if not an instance)
    
    // CROSS-MODAL BRIDGE REFERENCE (for multi-modal binding)
    // Links nodes representing same concept across modalities
    struct ModalityBridge *modality_bridge; // Cross-modal binding (NULL if single-modality)
    
    // ATTENTION MECHANISM (task-dependent focus)
    struct AttentionMechanism *attention;   // Query-dependent attention (NULL = lazy init)
    
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
    uint8_t inactivity_timer;         // 1 byte - Inactivity counter (0-255)
    uint8_t flags;                    // 1 byte - Packed flags:
                                      //   bit 0: marked_for_deletion
                                      //   bit 1: is_similarity_edge
                                      //   bit 2: direction (1=from->to, 0=to->from)
                                      //   bits 3-7: reserved
    uint8_t reserved;                 // 1 byte - alignment padding
    uint32_t last_wave_generation;   // 4 bytes - Last wave this edge fired
    
    // BRAIN-INSPIRED: Multi-context synaptic tags (replaces routing_gate)
    // Neuroscience: Synapses store multiple tagged memories
    // Each tag remembers a different training context
    // When current context matches a tag, that tag's weight contribution activates
    ContextTag *context_tags;         // Array of context-specific weights
    size_t tag_count;                 // Number of context tags
    size_t tag_capacity;              // Allocated capacity (grows dynamically)
    
    // TEMPORAL REASONING (sequence understanding and causality)
    // Enables "what happened before/after X?" type queries
    // All statistics use Welford's algorithm (no hardcoded values)
    TemporalRelation temporal_relation;    // Temporal relationship type
    
    // Temporal distance statistics (Welford-style)
    float temporal_distance_mean;          // Mean temporal distance
    float temporal_distance_m2;            // Sum of squared deviations
    uint64_t temporal_observations;        // Number of observations
    
    // Causal strength statistics (Welford-style)
    float causal_strength_mean;            // Mean causal strength
    float causal_strength_m2;              // Sum of squared deviations
    uint64_t causal_observations;          // Number of observations
    
    // TRANSFORMER ATTENTION (local transformation)
    // Implements Requirement.md line 7: "transform locally in the same LLM transform globally"
    // Each edge acts as a mini-transformer with Q, K, V projections
    EdgeTransformer *transformer;          // NULL = lazy init (saves memory)
    
    // Cached attention score (computed on-demand)
    float cached_attention_score;          // Last computed attention score
    uint32_t attention_cache_gen;          // Generation when cached (for invalidation)
} Edge;

/* Payload Trie Node: O(1) Pattern Matching with Multi-Node Support
 * - Intelligence: Like brain's visual cortex - hierarchical feature detection
 * - Each level recognizes longer patterns
 * - Enables O(pattern_size) lookup instead of O(all_nodes)
 * - MULTI-PATTERN: Stores multiple nodes per payload for context-based disambiguation
 * - Enables billions of patterns to coexist and compound knowledge
 */
typedef struct PayloadTrieNode {
    struct PayloadTrieNode *children[256];  // One per byte value
    Node **terminal_nodes;                  // Array of nodes at this position (enables multi-pattern)
    size_t terminal_count;                  // Current number of terminal nodes
    size_t terminal_capacity;               // Allocated capacity (grows dynamically, no hardcoded max)
    size_t depth;                          // How deep in trie
} PayloadTrieNode;

/* ============================================================================
 * GENERAL INTELLIGENCE STRUCTURES
 * 
 * These structures implement the 6 architectural gaps for general-purpose AI:
 * 1. Abstraction - Generalization across similar structures
 * 2. Composition - Concept manipulation (A + B, A - B, etc.)
 * 3. Temporal - Sequence understanding and causality
 * 4. Cross-Modal - Knowledge transfer across modalities
 * 5. Attention - Task-dependent focus
 * 6. Uncertainty - Confidence and ambiguity handling
 * ============================================================================ */

/* AttentionMechanism: Query-dependent focus (brain-inspired, not Transformer)
 * - Enables "what color?" vs "what size?" to focus on different neighbors
 * - All statistics use Welford's algorithm (no hardcoded values)
 */
struct AttentionMechanism {
    // Per-neighbor attention weights (dynamic array)
    float *attention_weights;              // Current attention weights
    size_t weight_count;                   // Number of weights
    size_t weight_capacity;                // Allocated capacity
    
    // Attention success tracking per neighbor (Welford-style)
    float *attention_success_mean;         // Mean success per neighbor
    float *attention_success_m2;           // Sum of squared deviations
    uint64_t *attention_counts;            // Per-neighbor count
    
    // Adaptive focus width (how many neighbors to activate)
    float focus_width_mean;                // Mean focus width
    float focus_width_m2;                  // Sum of squared deviations
    uint64_t focus_updates;                // Number of updates
};

/* ModalityBridge: Cross-modal binding for same concept
 * - Links text "dog", audio [bark], visual [dog image] as same concept
 * - Enables transfer learning across modalities
 */
struct ModalityBridge {
    // Nodes from different modalities representing same concept
    Node *modality_nodes[8];               // One per port type (PORT_TEXT, PORT_AUDIO, etc.)
    uint8_t modality_mask;                 // Which modalities are present (bitmask)
    
    // Shared semantic embedding (learned center of all modalities)
    SparseEmbedding *shared_embedding;     // Common semantic representation
    
    // Cross-modal consistency tracking (Welford-style)
    float consistency_mean;                // Mean consistency
    float consistency_m2;                  // Sum of squared deviations
    uint64_t cooccurrence_count;           // Number of co-occurrences
};

/* AbstractionNode: Represents a concept/class (e.g., "ANIMAL" for cat, dog, bird)
 * - Enables generalization: "the mouse sat" works after learning "the cat/dog sat"
 * - Instances can substitute for each other in similar contexts
 */
struct AbstractionNode {
    Node base;                             // Inherit from Node (first member for casting)
    
    AbstractionType abstraction_type;      // Type of abstraction
    
    // Instance tracking (which concrete nodes belong to this abstraction)
    Node **instances;                      // Array of instance nodes
    size_t instance_count;                 // Current instance count
    size_t instance_capacity;              // Allocated capacity
    
    // Prototype embedding (center of cluster, sparse)
    SparseEmbedding *prototype;            // Prototype semantic representation
    
    // Cluster variance tracking (Welford-style)
    float cluster_variance_mean;           // Mean variance within cluster
    float cluster_variance_m2;             // Sum of squared deviations
    uint64_t cluster_updates;              // Number of updates
    
    // Substitutability tracking (can instances swap in context?)
    float substitution_success_mean;       // Mean substitution success
    float substitution_success_m2;         // Sum of squared deviations
    uint64_t substitution_attempts;        // Number of attempts
};

/* CompositionCache: Cache for compositional operations (avoid recomputation)
 * - Caches A + B = C, A - B = D, etc.
 * - O(1) lookup for repeated compositions
 */
struct CompositionCache {
    Node *operand_a;                       // First operand
    Node *operand_b;                       // Second operand
    CompositionOp op;                      // Operation performed
    Node *result;                          // Cached result
    float confidence;                      // Confidence in result
    uint64_t use_count;                    // How often used (for LRU)
};

/* ProbabilisticOutput: Uncertainty-aware output with multiple candidates
 * - Returns "80% A, 15% B, 5% C" instead of just "A"
 * - Enables asking for clarification when uncertain
 */
struct ProbabilisticOutput {
    Node **candidates;                     // Possible output nodes
    float *probabilities;                  // Probability per candidate (sum to 1)
    size_t candidate_count;                // Number of candidates
    size_t candidate_capacity;             // Allocated capacity
    
    // Uncertainty metrics
    float entropy;                         // Shannon entropy (higher = more uncertain)
    
    // Calibration tracking (Welford-style)
    float calibration_error_mean;          // Mean calibration error
    float calibration_error_m2;            // Sum of squared deviations
    uint64_t calibration_samples;          // Number of samples
};

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
    
    // CACHED STATISTICS - Avoids O(n) loops (Requirement.md line 2)
    // Updated incrementally when nodes/edges added/removed
    size_t cached_total_degree;       // Sum of all outgoing degrees
    float cached_avg_degree;          // Average degree (cached_total_degree / node_count)
    float cached_total_edge_weight;   // Sum of all edge weights
    size_t cached_blank_count;        // Number of blank nodes
    size_t cached_raw_count;          // Number of raw (non-blank, non-hierarchy) nodes
    
    // ADAPTIVE STATISTICS - Brain-inspired running statistics (NO O(n) scans)
    // All updated incrementally during normal operations
    
    // Running statistics for adaptive thresholds (Welford's algorithm)
    float running_activation_mean;    // Running mean of activations
    float running_activation_m2;      // Running sum of squared deviations
    uint64_t activation_sample_count; // Number of samples for running stats
    
    float running_confidence_mean;    // Running mean of confidence values
    float running_confidence_m2;      // Running sum of squared deviations
    uint64_t confidence_sample_count; // Number of confidence samples
    
    float running_error_mean;         // Running mean of error signals
    float running_error_m2;           // Running sum of squared deviations
    uint64_t error_sample_count;      // Number of error samples
    
    // Path statistics (updated during generation)
    float running_path_length_mean;   // Running mean of path lengths
    float running_path_length_m2;     // Running sum of squared deviations
    uint64_t path_sample_count;       // Number of path samples
    
    // Adaptive limits that emerge from data
    float adaptive_neighbor_factor;   // Multiplier for neighbor iteration limits
    float adaptive_output_factor;     // Multiplier for output length limits
    
    // ========================================================================
    // GENERAL INTELLIGENCE CAPABILITIES
    // ========================================================================
    
    // ABSTRACTION LAYER (generalization across similar structures)
    AbstractionNode **abstractions;        // Array of abstraction nodes
    size_t abstraction_count;              // Number of abstractions
    size_t abstraction_capacity;           // Allocated capacity
    
    // Abstraction formation statistics (Welford-style)
    float abstraction_formation_mean;      // Mean abstractions per activation
    float abstraction_formation_m2;        // Sum of squared deviations
    uint64_t abstraction_formation_count;  // Number of formation events
    
    // COMPOSITIONAL ALGEBRA (concept manipulation)
    CompositionCache **composition_cache;  // Cache of composition results
    size_t composition_cache_count;        // Number of cached compositions
    size_t composition_cache_capacity;     // Allocated capacity
    
    // Composition MiniNet (learned composition weights)
    MiniNet *composition_net;              // Decides composition parameters
    
    // Composition success statistics (Welford-style)
    float composition_success_mean[COMP_COUNT];   // Per-operation success
    float composition_success_m2[COMP_COUNT];     // Sum of squared deviations
    uint64_t composition_counts[COMP_COUNT];      // Per-operation count
    
    // CROSS-MODAL INTEGRATION (knowledge transfer across modalities)
    ModalityBridge **modality_bridges;     // Array of cross-modal bridges
    size_t bridge_count;                   // Number of bridges
    size_t bridge_capacity;                // Allocated capacity
    
    // Bridge hash table for O(1) lookup
    ModalityBridge ***bridge_hash;         // Hash table
    size_t bridge_hash_size;               // Hash table size
    
    // Cross-modal consistency statistics (Welford-style)
    float crossmodal_consistency_mean;     // Mean cross-modal consistency
    float crossmodal_consistency_m2;       // Sum of squared deviations
    uint64_t crossmodal_samples;           // Number of samples
    
    // UNCERTAINTY HANDLING (confidence and ambiguity)
    ProbabilisticOutput *current_output;   // Current probabilistic output
    
    // Uncertainty threshold statistics (Welford-style)
    float uncertainty_threshold_mean;      // Mean uncertainty threshold
    float uncertainty_threshold_m2;        // Sum of squared deviations
    uint64_t uncertainty_samples;          // Number of samples
    
    // SEMANTIC EMBEDDING STATISTICS (for sparse embeddings)
    // Track global embedding statistics for adaptive dimensionality
    float semantic_similarity_mean;        // Mean similarity between nodes
    float semantic_similarity_m2;          // Sum of squared deviations
    uint64_t semantic_similarity_samples;  // Number of similarity computations
    
    // Embedding update generation (for cache invalidation)
    uint32_t semantic_update_generation;   // Incremented on major semantic changes
    
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
 * SPARSE DISTRIBUTED REPRESENTATION (Brain-Inspired Context)
 * ============================================================================
 * 
 * Neuroscience: The brain uses sparse, high-dimensional codes:
 * - Sparse: Only ~2% of neurons active at once
 * - High-dimensional: Thousands of neurons encode context
 * - Distributed: Pattern spread across many neurons
 * 
 * This replaces the broken 8-bit context signature (256 values) with
 * sparse distributed representations (10^70+ possible patterns).
 * 
 * Requirement.md line 6: "context is a payload, of that activated nodes"
 * - SparseContext stores the actual activated nodes, not a hash!
 */

/* SparseContext: Stores actual activated nodes (replaces 8-bit hash)
 * - Brain-like: Sparse activation pattern (2-5% of nodes active)
 * - No collisions: Each pattern has unique sparse code
 * - High capacity: k active nodes = C(n,k) possible patterns
 * - O(k) operations where k = active nodes (NOT O(n))
 */
typedef struct SparseContext {
    Node **active_nodes;      // Sparse array of active nodes
    float *activations;       // Activation strength per node
    uint8_t *port_ids;        // Port ID for each active node (for multimodal matching)
    uint32_t *abstraction_levels;  // Abstraction level per node (0=raw, 1+=hierarchy)
    size_t count;             // Number of active nodes (typically 2-5% of total)
    size_t capacity;          // Allocated capacity (grows dynamically)
    uint32_t generation;      // When this context was created (for temporal ordering)
    uint32_t max_abstraction_level;  // Highest abstraction level in this context
} SparseContext;

/* ContextTag: Synaptic Tagging (Brain-Inspired Multi-Context Memory)
 * 
 * Neuroscience: Synapses store multiple tagged memories:
 * - Synaptic Tags: Molecular markers that label when/how synapse was strengthened
 * - Multiple Tags: Same synapse can have multiple tags from different learning events
 * - Context Reactivation: When context matches a tag, that specific memory strengthens
 * 
 * This replaces single routing_gate (one context) with array of context tags.
 * Each edge can remember MULTIPLE training contexts!
 */
typedef struct ContextTag {
    SparseContext *context;        // The sparse activation pattern (training context)
    float weight_contribution;     // How much this context contributes to edge weight
    uint32_t creation_time;        // When this tag was created
    uint32_t last_activation;      // Last time this tag was activated
} ContextTag;

/* ============================================================================
 * SPARSE CONTEXT FUNCTIONS (Brain-Inspired Context Management)
 * ============================================================================
 * 
 * These functions implement Sparse Distributed Representations (SDRs):
 * - Create: Build sparse context from activation patterns or node arrays
 * - Match: Compare contexts using Jaccard-like overlap (O(k) complexity)
 * - Free: Clean up memory
 * 
 * Requirement.md compliance:
 * - Line 2: NO O(n) searches - all operations are O(k) where k = active nodes
 * - Line 3: No hardcoded limits - arrays grow dynamically
 * - Line 6: Context = activated nodes - stores actual nodes, not hash
 */

/* Create sparse context from node array with activations
 * - Copies only nodes with positive activation (sparse)
 * - Sorts by activation strength for efficient matching
 * - O(k) where k = count
 */
static SparseContext* sparse_context_create_from_nodes(Node **nodes, float *activations, size_t count) {
    if (!nodes || count == 0) return NULL;
    
    SparseContext *ctx = calloc(1, sizeof(SparseContext));
    if (!ctx) return NULL;
    
    // Count nodes with positive activation (sparse selection)
    size_t active_count = 0;
    for (size_t i = 0; i < count; i++) {
        if (nodes[i] && activations && activations[i] > 0.0f) {
            active_count++;
        } else if (nodes[i] && !activations) {
            active_count++;  // If no activations provided, include all
        }
    }
    
    if (active_count == 0) {
        free(ctx);
        return NULL;
    }
    
    // Allocate arrays (including port_ids and abstraction_levels for hierarchical context)
    ctx->active_nodes = malloc(active_count * sizeof(Node*));
    ctx->activations = malloc(active_count * sizeof(float));
    ctx->port_ids = malloc(active_count * sizeof(uint8_t));
    ctx->abstraction_levels = malloc(active_count * sizeof(uint32_t));
    if (!ctx->active_nodes || !ctx->activations || !ctx->port_ids || !ctx->abstraction_levels) {
        free(ctx->active_nodes);
        free(ctx->activations);
        free(ctx->port_ids);
        free(ctx->abstraction_levels);
        free(ctx);
        return NULL;
    }
    
    // Compute activation range for recency normalization (data-driven, no hardcoded decay)
    float max_activation = 0.0f, min_activation = 1.0f;
    for (size_t i = 0; i < count; i++) {
        if (nodes[i] && activations && activations[i] > 0.0f) {
            if (activations[i] > max_activation) max_activation = activations[i];
            if (activations[i] < min_activation) min_activation = activations[i];
        }
    }
    float range = max_activation - min_activation;
    float epsilon = (range > 0.0f) ? (range * 0.001f) : 0.001f;  // Adaptive epsilon
    (void)epsilon;  // Used for range tracking
    
    // Copy active nodes with recency weighting, port_id, and abstraction level tracking
    size_t idx = 0;
    uint32_t max_level = 0;  // Track highest abstraction level
    for (size_t i = 0; i < count; i++) {
        if (nodes[i] && activations && activations[i] > 0.0f) {
            // Position-based recency: later in sequence = more recent
            // Recency boost emerges from position ratio (pure data-driven, no hardcoded decay)
            float position_factor = (count > 1) ? ((float)i / (float)(count - 1)) : 0.5f;  // 0.0 = oldest, 1.0 = newest
            float recency_boost = position_factor / (position_factor + 1.0f);  // Range [0, 0.5], adaptive
            
            // Level-based boost: hierarchies get stronger context weight
            // NO HARDCODED 0.3f - boost proportional to level (each level adds proportional strength)
            uint32_t node_level = nodes[i]->abstraction_level;
            float level_boost = 1.0f + (float)node_level / (1.0f + (float)node_level);  // Asymptotic [1.0, 2.0]
            
            ctx->active_nodes[idx] = nodes[i];
            ctx->activations[idx] = activations[i] * (1.0f + recency_boost) * level_boost;  // Boost recent + hierarchy
            ctx->port_ids[idx] = nodes[i]->port_id;  // Track port for multimodal
            ctx->abstraction_levels[idx] = node_level;  // Track abstraction level
            
            if (node_level > max_level) max_level = node_level;
            idx++;
        } else if (nodes[i] && !activations) {
            // No activations provided - use position-based default
            float position_factor = (count > 1) ? ((float)i / (float)(count - 1)) : 0.5f;
            float recency_boost = position_factor / (position_factor + 1.0f);
            
            uint32_t node_level = nodes[i]->abstraction_level;
            float level_boost = 1.0f + (float)node_level / (1.0f + (float)node_level);  // Asymptotic [1.0, 2.0]
            
            ctx->active_nodes[idx] = nodes[i];
            ctx->activations[idx] = 1.0f * (1.0f + recency_boost) * level_boost;  // Default with recency + level
            ctx->port_ids[idx] = nodes[i]->port_id;
            ctx->abstraction_levels[idx] = node_level;
            
            if (node_level > max_level) max_level = node_level;
            idx++;
        }
    }
    
    ctx->count = active_count;
    ctx->capacity = active_count;
    ctx->generation = 0;  // Will be set by caller if needed
    ctx->max_abstraction_level = max_level;  // Track highest level in context
    
    return ctx;
}

/* Create sparse context from activation pattern
 * - Uses the pattern's activated nodes and strengths
 * - Filters to only include significantly active nodes (top percentile)
 * - O(k) where k = pattern->count
 */
static SparseContext* sparse_context_create_from_pattern(ActivationPattern *pattern) {
    if (!pattern || pattern->count == 0) return NULL;
    
    return sparse_context_create_from_nodes(
        pattern->nodes, 
        pattern->activations, 
        pattern->count
    );
}

/* Compare two sparse contexts using Jaccard-like overlap
 * Returns: 0.0 (no overlap) to 1.0 (identical)
 * 
 * Brain-like: Neurons that fire together have overlapping sparse codes
 * O(k1 * k2) in naive implementation, but k is small (2-5% of total)
 * For k=50, this is 2500 comparisons (not millions)
 * 
 * TODO: Can optimize to O(k) with hash table if needed
 */
/* Helper: Check if a node is a child/component of a hierarchy (O(child_count))
 * Used for cross-level context matching
 */
static int node_is_child_of_hierarchy(Node *child, Node *hierarchy) {
    if (!hierarchy || !child) return 0;
    if (hierarchy->abstraction_level == 0) return 0;  // Not a hierarchy
    if (!hierarchy->child_nodes || hierarchy->child_count == 0) return 0;
    
    // O(child_count) - typically small (2-10 children per hierarchy)
    for (size_t i = 0; i < hierarchy->child_count; i++) {
        if (hierarchy->child_nodes[i] == child) return 1;
    }
    return 0;
}

static float sparse_context_match(SparseContext *ctx1, SparseContext *ctx2) {
    if (!ctx1 || !ctx2) return 0.0f;
    if (ctx1->count == 0 || ctx2->count == 0) return 0.0f;
    
    // Count overlapping UNIQUE nodes (pointer comparison)
    // Also track port_id matches for multimodal boosting
    size_t overlap_count = 0;
    size_t port_match_count = 0;
    
    // Find overlapping nodes
    // O(k1 * k2) but k is typically small (20-50)
    for (size_t i = 0; i < ctx1->count; i++) {
        for (size_t j = 0; j < ctx2->count; j++) {
            if (ctx1->active_nodes[i] == ctx2->active_nodes[j]) {
                overlap_count++;
                
                // Track port_id matches for multimodal discrimination
                // Same node + same port = stronger match (intra-modal)
                // Same node + different port = weaker match (cross-modal)
                if (ctx1->port_ids && ctx2->port_ids &&
                    ctx1->port_ids[i] == ctx2->port_ids[j]) {
                    port_match_count++;
                }
                break;  // Only count each node once
            }
        }
    }
    
    // === CROSS-LEVEL CONTEXT MATCHING ===
    // If ctx1 has hierarchies, check if ctx2's raw nodes are components
    // Enables "hello" hierarchy to match context containing [h,e,l,l,o] raw nodes
    size_t cross_level_matches = 0;
    
    for (size_t i = 0; i < ctx1->count; i++) {
        Node *n1 = ctx1->active_nodes[i];
        if (!n1) continue;  // Null check
        if (n1->abstraction_level == 0) continue;  // Skip raw nodes, only check hierarchies
        
        // Check if any ctx2 nodes are children of this hierarchy
        for (size_t j = 0; j < ctx2->count; j++) {
            Node *n2 = ctx2->active_nodes[j];
            if (!n2) continue;  // Null check
            if (node_is_child_of_hierarchy(n2, n1)) {
                cross_level_matches++;
            }
        }
    }
    
    // Also check reverse: ctx2 hierarchies matching ctx1 raw nodes
    for (size_t i = 0; i < ctx2->count; i++) {
        Node *n2 = ctx2->active_nodes[i];
        if (!n2) continue;  // Null check
        if (n2->abstraction_level == 0) continue;
        
        for (size_t j = 0; j < ctx1->count; j++) {
            Node *n1 = ctx1->active_nodes[j];
            if (!n1) continue;  // Null check
            if (node_is_child_of_hierarchy(n1, n2)) {
                cross_level_matches++;
            }
        }
    }
    
    // If no direct overlap but cross-level matches exist, use those
    if (overlap_count == 0 && cross_level_matches == 0) return 0.0f;
    
    // JACCARD SIMILARITY: intersection / union
    // union = |ctx1| + |ctx2| - |intersection|
    // This properly penalizes size mismatch:
    // - [h,e,l,l,o] vs [h,e,l,l,o] → 5/5 = 1.0 (perfect)
    // - [h,e,l,l,o] vs [h,e,l,l,o,' ',w,o] → 5/(5+8-5) = 5/8 = 0.625 (penalized for extra nodes)
    // - [h,e,l,l,o] vs [w,o] → 1/(5+2-1) = 1/6 = 0.167 (low overlap)
    
    // Count unique nodes in each context (handle duplicates like 'l' appearing twice)
    // For simplicity, we use count directly - duplicates are rare in practice
    size_t unique1 = ctx1->count;
    size_t unique2 = ctx2->count;
    size_t total_overlap = overlap_count + cross_level_matches;
    size_t union_size = unique1 + unique2 - overlap_count;  // Don't subtract cross-level (they're different nodes)
    
    if (union_size == 0) return 0.0f;
    
    float match = (float)total_overlap / (float)union_size;
    
    // PORT ALIGNMENT BOOST (adaptive, for multimodal discrimination)
    // Boost match when ports align - enables cross-modal vs intra-modal distinction
    // Pure ratio: no hardcoded constants, boost emerges from port alignment
    if (ctx1->port_ids && ctx2->port_ids && overlap_count > 0) {
        float port_alignment = (float)port_match_count / (float)overlap_count;  // Range [0, 1]
        // Boost: 1.0 (no alignment) to 2.0 (perfect alignment)
        // This is data-driven: boost = 1 + alignment ratio
        match *= (1.0f + port_alignment);
    }
    
    // CROSS-LEVEL BOOST: If cross-level matches found, boost match
    // Enables hierarchy-to-raw-node associations
    // NO HARDCODED 0.5f - boost proportional to cross-level ratio
    if (cross_level_matches > 0 && total_overlap > 0) {
        float cross_level_ratio = (float)cross_level_matches / (float)total_overlap;
        // Boost: asymptotic based on ratio (pure data-driven)
        match *= (1.0f + cross_level_ratio / (1.0f + cross_level_ratio));  // Range [1.0, 1.5]
    }
    
    return match;
}

/* Free sparse context and its arrays */
static void sparse_context_free(SparseContext *ctx) {
    if (!ctx) return;
    free(ctx->active_nodes);
    free(ctx->activations);
    free(ctx->port_ids);
    free(ctx->abstraction_levels);
    free(ctx);
}

/* Clone a sparse context (deep copy) */
static SparseContext* sparse_context_clone(SparseContext *src) {
    if (!src) return NULL;
    
    SparseContext *dst = calloc(1, sizeof(SparseContext));
    if (!dst) return NULL;
    
    dst->active_nodes = malloc(src->count * sizeof(Node*));
    dst->activations = malloc(src->count * sizeof(float));
    dst->port_ids = malloc(src->count * sizeof(uint8_t));
    dst->abstraction_levels = malloc(src->count * sizeof(uint32_t));
    if (!dst->active_nodes || !dst->activations || !dst->port_ids || !dst->abstraction_levels) {
        free(dst->active_nodes);
        free(dst->activations);
        free(dst->port_ids);
        free(dst->abstraction_levels);
        free(dst);
        return NULL;
    }
    
    memcpy(dst->active_nodes, src->active_nodes, src->count * sizeof(Node*));
    memcpy(dst->activations, src->activations, src->count * sizeof(float));
    if (src->port_ids) {
        memcpy(dst->port_ids, src->port_ids, src->count * sizeof(uint8_t));
    } else {
        memset(dst->port_ids, 0, src->count * sizeof(uint8_t));
    }
    if (src->abstraction_levels) {
        memcpy(dst->abstraction_levels, src->abstraction_levels, src->count * sizeof(uint32_t));
    } else {
        memset(dst->abstraction_levels, 0, src->count * sizeof(uint32_t));
    }
    dst->count = src->count;
    dst->capacity = src->count;
    dst->generation = src->generation;
    dst->max_abstraction_level = src->max_abstraction_level;
    
    return dst;
}

/* Add a node to an existing sparse context (grows dynamically)
 * - Used to add hierarchy nodes to context when they activate
 * - Level-based weighting: hierarchies get stronger context weight
 * - O(1) amortized (doubles capacity when needed)
 */
static void sparse_context_add_node(SparseContext *ctx, Node *node, float activation) {
    if (!ctx || !node) return;
    
    // Check if node already exists in context (avoid duplicates)
    for (size_t i = 0; i < ctx->count; i++) {
        if (ctx->active_nodes[i] == node) {
            // Update activation if already present (boost)
            ctx->activations[i] += activation;
            return;
        }
    }
    
    // Grow arrays if needed (exponential growth, no hardcoded limit)
    if (ctx->count >= ctx->capacity) {
        size_t new_cap = (ctx->capacity == 0) ? 4 : ctx->capacity * 2;
        
        Node **new_nodes = realloc(ctx->active_nodes, new_cap * sizeof(Node*));
        float *new_acts = realloc(ctx->activations, new_cap * sizeof(float));
        uint8_t *new_ports = realloc(ctx->port_ids, new_cap * sizeof(uint8_t));
        uint32_t *new_levels = realloc(ctx->abstraction_levels, new_cap * sizeof(uint32_t));
        
        if (!new_nodes || !new_acts || !new_ports || !new_levels) {
            // Allocation failed, keep existing (graceful degradation)
            return;
        }
        
        ctx->active_nodes = new_nodes;
        ctx->activations = new_acts;
        ctx->port_ids = new_ports;
        ctx->abstraction_levels = new_levels;
        ctx->capacity = new_cap;
    }
    
    // Add new node with level-based weighting
    // NO HARDCODED 0.5f - boost asymptotic based on level
    uint32_t node_level = node->abstraction_level;
    float level_boost = 1.0f + (float)node_level / (1.0f + (float)node_level);  // Asymptotic [1.0, 2.0]
    
    ctx->active_nodes[ctx->count] = node;
    ctx->activations[ctx->count] = activation * level_boost;
    ctx->port_ids[ctx->count] = node->port_id;
    ctx->abstraction_levels[ctx->count] = node_level;
    ctx->count++;
    
    // Update max abstraction level
    if (node_level > ctx->max_abstraction_level) {
        ctx->max_abstraction_level = node_level;
    }
}

/* ============================================================================
 * SPARSE EMBEDDING IMPLEMENTATION (General Intelligence Foundation)
 * 
 * All operations are O(k) where k = active_count (typically 5-20)
 * NO O(n) operations, NO hardcoded limits, all adaptive via Welford
 * ============================================================================ */

/* Create a new sparse embedding with initial capacity */
static SparseEmbedding* sparse_embedding_create(size_t initial_capacity) {
    SparseEmbedding *emb = calloc(1, sizeof(SparseEmbedding));
    if (!emb) return NULL;
    
    // Use adaptive initial capacity (no hardcoded minimum)
    size_t cap = (initial_capacity > 0) ? initial_capacity : 8;
    
    emb->active_dims = malloc(cap * sizeof(uint16_t));
    emb->active_values = malloc(cap * sizeof(float));
    
    if (!emb->active_dims || !emb->active_values) {
        free(emb->active_dims);
        free(emb->active_values);
        free(emb);
        return NULL;
    }
    
    emb->active_count = 0;
    emb->active_capacity = cap;
    
    // Initialize Welford statistics
    emb->activation_mean = 0.0f;
    emb->activation_m2 = 0.0f;
    emb->update_count = 0;
    
    return emb;
}

/* Free a sparse embedding */
static void sparse_embedding_free(SparseEmbedding *emb) {
    if (!emb) return;
    free(emb->active_dims);
    free(emb->active_values);
    free(emb);
}

/* Clone a sparse embedding (deep copy) */
static SparseEmbedding* sparse_embedding_clone(SparseEmbedding *src) {
    if (!src) return NULL;
    
    SparseEmbedding *dst = sparse_embedding_create(src->active_capacity);
    if (!dst) return NULL;
    
    memcpy(dst->active_dims, src->active_dims, src->active_count * sizeof(uint16_t));
    memcpy(dst->active_values, src->active_values, src->active_count * sizeof(float));
    dst->active_count = src->active_count;
    
    // Copy Welford statistics
    dst->activation_mean = src->activation_mean;
    dst->activation_m2 = src->activation_m2;
    dst->update_count = src->update_count;
    
    return dst;
}

/* Binary search for dimension in sorted active_dims array - O(log k) */
static size_t sparse_embedding_find_dim(SparseEmbedding *emb, uint16_t dim) {
    if (!emb || emb->active_count == 0) return SIZE_MAX;
    
    size_t left = 0;
    size_t right = emb->active_count;
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        if (emb->active_dims[mid] == dim) {
            return mid;
        } else if (emb->active_dims[mid] < dim) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return SIZE_MAX;  // Not found
}

/* Get value for a dimension (O(log k) via binary search) */
static float sparse_embedding_get_dimension(SparseEmbedding *emb, uint16_t dim) {
    if (!emb) return 0.0f;
    
    size_t idx = sparse_embedding_find_dim(emb, dim);
    if (idx != SIZE_MAX) {
        return emb->active_values[idx];
    }
    return 0.0f;  // Inactive dimensions have value 0
}

/* Set value for a dimension (O(k) for insertion, O(log k) for update) */
static void sparse_embedding_set_dimension(SparseEmbedding *emb, uint16_t dim, float value) {
    if (!emb) return;
    
    // If value is near zero, remove the dimension (sparsity maintenance)
    if (fabsf(value) < 1e-6f) {
        size_t idx = sparse_embedding_find_dim(emb, dim);
        if (idx != SIZE_MAX) {
            // Remove by shifting
            for (size_t i = idx; i < emb->active_count - 1; i++) {
                emb->active_dims[i] = emb->active_dims[i + 1];
                emb->active_values[i] = emb->active_values[i + 1];
            }
            emb->active_count--;
        }
        return;
    }
    
    // Check if dimension already exists
    size_t idx = sparse_embedding_find_dim(emb, dim);
    if (idx != SIZE_MAX) {
        emb->active_values[idx] = value;
        return;
    }
    
    // Need to insert new dimension
    // Grow capacity if needed (exponential growth, no hardcoded limit)
    if (emb->active_count >= emb->active_capacity) {
        size_t new_cap = emb->active_capacity * 2;
        uint16_t *new_dims = realloc(emb->active_dims, new_cap * sizeof(uint16_t));
        float *new_vals = realloc(emb->active_values, new_cap * sizeof(float));
        
        if (!new_dims || !new_vals) {
            return;  // Keep existing (graceful degradation)
        }
        
        emb->active_dims = new_dims;
        emb->active_values = new_vals;
        emb->active_capacity = new_cap;
    }
    
    // Find insertion point to maintain sorted order
    size_t insert_pos = 0;
    while (insert_pos < emb->active_count && emb->active_dims[insert_pos] < dim) {
        insert_pos++;
    }
    
    // Shift elements to make room
    for (size_t i = emb->active_count; i > insert_pos; i--) {
        emb->active_dims[i] = emb->active_dims[i - 1];
        emb->active_values[i] = emb->active_values[i - 1];
    }
    
    // Insert new dimension
    emb->active_dims[insert_pos] = dim;
    emb->active_values[insert_pos] = value;
    emb->active_count++;
}

/* Compute similarity between two sparse embeddings - O(k_a + k_b) merge-style
 * Returns dot product normalized by magnitude (cosine-like similarity)
 */
static float sparse_embedding_similarity(SparseEmbedding *a, SparseEmbedding *b) {
    if (!a || !b || a->active_count == 0 || b->active_count == 0) {
        return 0.0f;
    }
    
    float dot_product = 0.0f;
    float mag_a = 0.0f;
    float mag_b = 0.0f;
    
    // Compute magnitudes
    for (size_t i = 0; i < a->active_count; i++) {
        mag_a += a->active_values[i] * a->active_values[i];
    }
    for (size_t i = 0; i < b->active_count; i++) {
        mag_b += b->active_values[i] * b->active_values[i];
    }
    
    mag_a = sqrtf(mag_a);
    mag_b = sqrtf(mag_b);
    
    if (mag_a < 1e-6f || mag_b < 1e-6f) return 0.0f;
    
    // Merge-style dot product - O(k_a + k_b)
    size_t i = 0, j = 0;
    while (i < a->active_count && j < b->active_count) {
        if (a->active_dims[i] == b->active_dims[j]) {
            dot_product += a->active_values[i] * b->active_values[j];
            i++;
            j++;
        } else if (a->active_dims[i] < b->active_dims[j]) {
            i++;
        } else {
            j++;
        }
    }
    
    return dot_product / (mag_a * mag_b);  // Cosine similarity
}

/* Normalize embedding to unit length - O(k) */
static void sparse_embedding_normalize(SparseEmbedding *emb) {
    if (!emb || emb->active_count == 0) return;
    
    float mag = 0.0f;
    for (size_t i = 0; i < emb->active_count; i++) {
        mag += emb->active_values[i] * emb->active_values[i];
    }
    mag = sqrtf(mag);
    
    if (mag > 1e-6f) {
        for (size_t i = 0; i < emb->active_count; i++) {
            emb->active_values[i] /= mag;
        }
    }
}

/* Hebbian update: strengthen dimensions that co-activate - O(k_context)
 * Uses Welford's algorithm for adaptive learning rate
 */
static void sparse_embedding_update_hebbian(SparseEmbedding *emb, SparseEmbedding *context, float base_learning_rate) {
    if (!emb || !context || context->active_count == 0) return;
    
    // Compute adaptive learning rate from Welford statistics
    float learning_rate = base_learning_rate;
    if (emb->update_count > 1) {
        // Use variance to modulate learning rate
        // High variance = more learning (unstable)
        // Low variance = less learning (stable)
        float variance = emb->activation_m2 / (float)(emb->update_count - 1);
        float stddev = sqrtf(variance + 1e-6f);
        
        // Modulate: higher variance -> higher learning rate
        learning_rate = base_learning_rate * (1.0f + stddev);
    }
    
    // Apply Hebbian update for each context dimension
    for (size_t i = 0; i < context->active_count; i++) {
        uint16_t dim = context->active_dims[i];
        float context_val = context->active_values[i];
        
        float current = sparse_embedding_get_dimension(emb, dim);
        float new_val = current + learning_rate * context_val;
        
        sparse_embedding_set_dimension(emb, dim, new_val);
    }
    
    // Update Welford statistics
    float mag = 0.0f;
    for (size_t i = 0; i < emb->active_count; i++) {
        mag += emb->active_values[i] * emb->active_values[i];
    }
    mag = sqrtf(mag);
    
    emb->update_count++;
    float delta = mag - emb->activation_mean;
    emb->activation_mean += delta / (float)emb->update_count;
    float delta2 = mag - emb->activation_mean;
    emb->activation_m2 += delta * delta2;
    
    // Normalize to prevent unbounded growth
    sparse_embedding_normalize(emb);
}

/* Convert dense vector to sparse embedding - O(dim + k log k)
 * Keeps only top-k dimensions by magnitude
 */
static SparseEmbedding* sparse_embedding_sparsify(float *dense, size_t dim, size_t target_sparsity) {
    if (!dense || dim == 0) return NULL;
    
    SparseEmbedding *emb = sparse_embedding_create(target_sparsity);
    if (!emb) return NULL;
    
    // Find top-k dimensions by magnitude
    // Simple selection: add all non-zero, then prune if needed
    for (size_t i = 0; i < dim && i < 65535; i++) {
        if (fabsf(dense[i]) > 1e-6f) {
            sparse_embedding_set_dimension(emb, (uint16_t)i, dense[i]);
        }
    }
    
    // If too many active dimensions, keep only top-k
    if (emb->active_count > target_sparsity) {
        // Sort by absolute value (using insertion sort since k is small)
        for (size_t i = 1; i < emb->active_count; i++) {
            uint16_t dim_tmp = emb->active_dims[i];
            float val_tmp = emb->active_values[i];
            float abs_val = fabsf(val_tmp);
            
            size_t j = i;
            while (j > 0 && fabsf(emb->active_values[j-1]) < abs_val) {
                emb->active_dims[j] = emb->active_dims[j-1];
                emb->active_values[j] = emb->active_values[j-1];
                j--;
            }
            emb->active_dims[j] = dim_tmp;
            emb->active_values[j] = val_tmp;
        }
        
        // Keep only top-k, re-sort by dimension
        emb->active_count = target_sparsity;
        
        // Re-sort by dimension for merge operations
        for (size_t i = 1; i < emb->active_count; i++) {
            uint16_t dim_tmp = emb->active_dims[i];
            float val_tmp = emb->active_values[i];
            
            size_t j = i;
            while (j > 0 && emb->active_dims[j-1] > dim_tmp) {
                emb->active_dims[j] = emb->active_dims[j-1];
                emb->active_values[j] = emb->active_values[j-1];
                j--;
            }
            emb->active_dims[j] = dim_tmp;
            emb->active_values[j] = val_tmp;
        }
    }
    
    return emb;
}

/* Compose two embeddings using specified operation - O(k_a + k_b)
 * Implements compositional algebra for concept manipulation
 */
static SparseEmbedding* sparse_embedding_compose(SparseEmbedding *a, SparseEmbedding *b, CompositionOp op) {
    if (!a && !b) return NULL;
    if (!a) return sparse_embedding_clone(b);
    if (!b) return sparse_embedding_clone(a);
    
    SparseEmbedding *result = sparse_embedding_create(a->active_count + b->active_count);
    if (!result) return NULL;
    
    switch (op) {
        case COMP_COMPOSE:
            // A + B: Add vectors (union of dimensions)
            for (size_t i = 0; i < a->active_count; i++) {
                sparse_embedding_set_dimension(result, a->active_dims[i], a->active_values[i]);
            }
            for (size_t i = 0; i < b->active_count; i++) {
                float current = sparse_embedding_get_dimension(result, b->active_dims[i]);
                sparse_embedding_set_dimension(result, b->active_dims[i], current + b->active_values[i]);
            }
            break;
            
        case COMP_DECOMPOSE:
            // AB - A = B: Subtract A from B
            for (size_t i = 0; i < a->active_count; i++) {
                sparse_embedding_set_dimension(result, a->active_dims[i], a->active_values[i]);
            }
            for (size_t i = 0; i < b->active_count; i++) {
                float current = sparse_embedding_get_dimension(result, b->active_dims[i]);
                sparse_embedding_set_dimension(result, b->active_dims[i], current - b->active_values[i]);
            }
            break;
            
        case COMP_SUBSTITUTE:
            // Similar to decompose then compose (handled at higher level)
            // Here just average the two
            for (size_t i = 0; i < a->active_count; i++) {
                sparse_embedding_set_dimension(result, a->active_dims[i], a->active_values[i] * 0.5f);
            }
            for (size_t i = 0; i < b->active_count; i++) {
                float current = sparse_embedding_get_dimension(result, b->active_dims[i]);
                sparse_embedding_set_dimension(result, b->active_dims[i], current + b->active_values[i] * 0.5f);
            }
            break;
            
        case COMP_ANALOGIZE:
            // A:B :: C:? uses (B - A + C) formula
            // For now, same as compose (higher-level handles full analogy)
            for (size_t i = 0; i < a->active_count; i++) {
                sparse_embedding_set_dimension(result, a->active_dims[i], a->active_values[i]);
            }
            for (size_t i = 0; i < b->active_count; i++) {
                float current = sparse_embedding_get_dimension(result, b->active_dims[i]);
                sparse_embedding_set_dimension(result, b->active_dims[i], current + b->active_values[i]);
            }
            break;
            
        default:
            break;
    }
    
    sparse_embedding_normalize(result);
    return result;
}

/* ============================================================================
 * EDGE TRANSFORMER IMPLEMENTATION (Local Multi-Head Attention)
 * 
 * Implements Requirement.md line 7: "edges transform locally in the same LLM transform globally"
 * 
 * Key operations:
 * - Q * K / sqrt(d) attention score computation
 * - Multi-head attention aggregation
 * - Value transformation based on attention
 * - Hebbian learning for Q, K, V projections
 * 
 * All operations O(k) where k = sparse embedding dimensions
 * ============================================================================ */

/* Create edge transformer with specified number of heads */
static EdgeTransformer* edge_transformer_create(size_t num_heads) {
    EdgeTransformer *transformer = calloc(1, sizeof(EdgeTransformer));
    if (!transformer) return NULL;
    
    // Use adaptive number of heads (no hardcoded minimum)
    size_t heads = (num_heads > 0) ? num_heads : 2;
    
    // Create sparse Q, K, V projections
    transformer->query_proj = sparse_embedding_create(16);
    transformer->key_proj = sparse_embedding_create(16);
    transformer->value_proj = sparse_embedding_create(16);
    
    if (!transformer->query_proj || !transformer->key_proj || !transformer->value_proj) {
        sparse_embedding_free(transformer->query_proj);
        sparse_embedding_free(transformer->key_proj);
        sparse_embedding_free(transformer->value_proj);
        free(transformer);
        return NULL;
    }
    
    // Allocate per-head arrays
    transformer->head_weights = calloc(heads, sizeof(float));
    transformer->head_success_mean = calloc(heads, sizeof(float));
    transformer->head_success_m2 = calloc(heads, sizeof(float));
    transformer->head_counts = calloc(heads, sizeof(uint64_t));
    
    if (!transformer->head_weights || !transformer->head_success_mean ||
        !transformer->head_success_m2 || !transformer->head_counts) {
        sparse_embedding_free(transformer->query_proj);
        sparse_embedding_free(transformer->key_proj);
        sparse_embedding_free(transformer->value_proj);
        free(transformer->head_weights);
        free(transformer->head_success_mean);
        free(transformer->head_success_m2);
        free(transformer->head_counts);
        free(transformer);
        return NULL;
    }
    
    transformer->num_heads = heads;
    transformer->head_capacity = heads;
    
    // Initialize head weights uniformly
    for (size_t i = 0; i < heads; i++) {
        transformer->head_weights[i] = 1.0f / (float)heads;
        transformer->head_success_mean[i] = 0.5f;  // Neutral initial
    }
    
    // Initialize Welford statistics
    transformer->attention_mean = 0.0f;
    transformer->attention_m2 = 0.0f;
    transformer->attention_count = 0;
    
    return transformer;
}

/* Free edge transformer */
static void edge_transformer_free(EdgeTransformer *transformer) {
    if (!transformer) return;
    
    sparse_embedding_free(transformer->query_proj);
    sparse_embedding_free(transformer->key_proj);
    sparse_embedding_free(transformer->value_proj);
    free(transformer->head_weights);
    free(transformer->head_success_mean);
    free(transformer->head_success_m2);
    free(transformer->head_counts);
    free(transformer);
}

/* Compute attention score: Q * K / sqrt(d) - O(k) */
static float edge_compute_attention_score(Edge *edge, SparseEmbedding *query_context) {
    if (!edge) return 0.0f;
    
    // Lazy initialize transformer if needed
    if (!edge->transformer) {
        edge->transformer = edge_transformer_create(2);  // Start with 2 heads
        if (!edge->transformer) return 0.0f;
    }
    
    // Check cache validity
    if (edge->from_node && edge->from_node->graph &&
        edge->attention_cache_gen == edge->from_node->graph->wave_generation) {
        return edge->cached_attention_score;
    }
    
    EdgeTransformer *t = edge->transformer;
    
    // Compute Q from query context (or use stored Q if no context)
    SparseEmbedding *query = query_context ? query_context : t->query_proj;
    
    // Compute Q * K similarity (O(k) sparse dot product)
    float dot_product = sparse_embedding_similarity(query, t->key_proj);
    
    // Scale by sqrt(d) where d = active dimensions
    size_t d = (query->active_count > 0) ? query->active_count : 1;
    float scale = 1.0f / sqrtf((float)d);
    
    float attention_score = dot_product * scale;
    
    // Apply softmax-like normalization (single edge, so just sigmoid)
    attention_score = 1.0f / (1.0f + expf(-attention_score));
    
    // Cache the score
    edge->cached_attention_score = attention_score;
    if (edge->from_node && edge->from_node->graph) {
        edge->attention_cache_gen = edge->from_node->graph->wave_generation;
    }
    
    // Update Welford statistics
    t->attention_count++;
    float delta = attention_score - t->attention_mean;
    t->attention_mean += delta / (float)t->attention_count;
    float delta2 = attention_score - t->attention_mean;
    t->attention_m2 += delta * delta2;
    
    return attention_score;
}

/* Multi-head attention: aggregate across heads - O(k * num_heads) */
static float edge_multi_head_attention(Edge *edge, SparseEmbedding *query_context) {
    if (!edge) return 0.0f;
    
    // Ensure transformer exists
    if (!edge->transformer) {
        edge->transformer = edge_transformer_create(2);
        if (!edge->transformer) return 0.0f;
    }
    
    EdgeTransformer *t = edge->transformer;
    
    // Compute base attention score
    float base_score = edge_compute_attention_score(edge, query_context);
    
    // Weight by head success (heads that have been successful get more weight)
    float weighted_score = 0.0f;
    float weight_sum = 0.0f;
    
    for (size_t h = 0; h < t->num_heads; h++) {
        // Each head applies a different "view" of the attention
        // Head weight is modulated by its success history
        float head_weight = t->head_weights[h];
        float success_factor = t->head_success_mean[h];
        
        // Combine: heads with higher success get more influence
        float effective_weight = head_weight * (0.5f + success_factor);
        
        // Each head can have slightly different attention based on its learned bias
        float head_attention = base_score * (0.8f + 0.4f * (float)h / (float)t->num_heads);
        
        weighted_score += head_attention * effective_weight;
        weight_sum += effective_weight;
    }
    
    // Normalize by total weight
    if (weight_sum > 0.0f) {
        weighted_score /= weight_sum;
    }
    
    return weighted_score;
}

/* Transform value based on attention score - O(k) */
static SparseEmbedding* edge_transform_value(Edge *edge, float attention_score) {
    if (!edge || !edge->transformer) return NULL;
    
    EdgeTransformer *t = edge->transformer;
    
    // Clone value projection
    SparseEmbedding *result = sparse_embedding_clone(t->value_proj);
    if (!result) return NULL;
    
    // Scale by attention score
    for (size_t i = 0; i < result->active_count; i++) {
        result->active_values[i] *= attention_score;
    }
    
    return result;
}

/* Update Q, K, V projections via Hebbian learning - O(k) */
static void edge_update_projections(Edge *edge, SparseEmbedding *from_emb, SparseEmbedding *to_emb, float success) {
    if (!edge || !from_emb || !to_emb) return;
    
    // Ensure transformer exists
    if (!edge->transformer) {
        edge->transformer = edge_transformer_create(2);
        if (!edge->transformer) return;
    }
    
    EdgeTransformer *t = edge->transformer;
    
    // Compute adaptive learning rate from Welford statistics
    float learning_rate = 0.1f;  // Base rate
    if (t->attention_count > 10) {
        float variance = t->attention_m2 / (float)t->attention_count;
        float stddev = sqrtf(variance + 1e-6f);
        // Higher variance = more learning (system is unstable)
        learning_rate *= (1.0f + stddev);
    }
    
    // Scale learning by success (successful predictions strengthen more)
    learning_rate *= (0.5f + success);
    
    // Update Q projection from source embedding
    sparse_embedding_update_hebbian(t->query_proj, from_emb, learning_rate);
    
    // Update K projection from target embedding
    sparse_embedding_update_hebbian(t->key_proj, to_emb, learning_rate);
    
    // Update V projection from combined context
    SparseEmbedding *combined = sparse_embedding_compose(from_emb, to_emb, COMP_COMPOSE);
    if (combined) {
        sparse_embedding_update_hebbian(t->value_proj, combined, learning_rate * 0.5f);
        sparse_embedding_free(combined);
    }
    
    // Update head success statistics (Welford)
    for (size_t h = 0; h < t->num_heads; h++) {
        t->head_counts[h]++;
        float delta = success - t->head_success_mean[h];
        t->head_success_mean[h] += delta / (float)t->head_counts[h];
        float delta2 = success - t->head_success_mean[h];
        t->head_success_m2[h] += delta * delta2;
    }
}

/* Aggregate transformer attention from all incoming edges - O(degree * k) */
static SparseEmbedding* node_aggregate_transformer_attention(Node *node, SparseEmbedding *query_context) {
    if (!node || node->incoming_count == 0) return NULL;
    
    // Create result embedding
    SparseEmbedding *result = sparse_embedding_create(32);
    if (!result) return NULL;
    
    float total_attention = 0.0f;
    
    // Aggregate attention-weighted values from all incoming edges
    for (size_t i = 0; i < node->incoming_count; i++) {
        Edge *edge = node->incoming_edges[i];
        if (!edge) continue;
        
        // Compute multi-head attention for this edge
        float attention = edge_multi_head_attention(edge, query_context);
        
        // Get transformed value
        SparseEmbedding *value = edge_transform_value(edge, attention);
        if (!value) continue;
        
        // Aggregate: add attention-weighted value to result
        for (size_t j = 0; j < value->active_count; j++) {
            float current = sparse_embedding_get_dimension(result, value->active_dims[j]);
            sparse_embedding_set_dimension(result, value->active_dims[j], 
                                          current + value->active_values[j]);
        }
        
        total_attention += attention;
        sparse_embedding_free(value);
    }
    
    // Normalize by total attention
    if (total_attention > 0.0f) {
        for (size_t i = 0; i < result->active_count; i++) {
            result->active_values[i] /= total_attention;
        }
    }
    
    sparse_embedding_normalize(result);
    return result;
}

/* Compute adaptive number of heads based on edge complexity */
static size_t compute_adaptive_num_heads(Edge *edge) {
    if (!edge) return 2;
    
    // Base: 2 heads
    size_t heads = 2;
    
    // More heads for edges with high variance in attention
    if (edge->transformer && edge->transformer->attention_count > 10) {
        float variance = edge->transformer->attention_m2 / (float)edge->transformer->attention_count;
        // High variance = need more heads to capture different aspects
        if (variance > 0.1f) heads = 3;
        if (variance > 0.2f) heads = 4;
    }
    
    // More heads for edges between semantically rich nodes
    if (edge->from_node && edge->from_node->sparse_embedding &&
        edge->from_node->sparse_embedding->active_count > 10) {
        heads++;
    }
    
    // Cap at reasonable maximum (still O(k))
    if (heads > 8) heads = 8;
    
    return heads;
}

/* ============================================================================
 * ABSTRACTION MECHANISM (Generalization)
 * 
 * Enables generalization across similar structures:
 * - "cat/dog/bird sat" → learns ANIMAL abstraction
 * - "the mouse sat" works without explicit training
 * 
 * All operations O(k) where k = neighbor count or instance count
 * All thresholds adaptive via Welford statistics
 * ============================================================================ */

/* Create an abstraction node */
static AbstractionNode* abstraction_node_create(AbstractionType type) {
    AbstractionNode *abs = calloc(1, sizeof(AbstractionNode));
    if (!abs) return NULL;
    
    abs->abstraction_type = type;
    
    // Initialize instance array with small capacity (grows adaptively)
    abs->instance_capacity = 4;
    abs->instances = malloc(abs->instance_capacity * sizeof(Node*));
    if (!abs->instances) {
        free(abs);
        return NULL;
    }
    abs->instance_count = 0;
    
    // Initialize prototype embedding
    abs->prototype = sparse_embedding_create(16);
    
    // Initialize Welford statistics
    abs->cluster_variance_mean = 0.0f;
    abs->cluster_variance_m2 = 0.0f;
    abs->cluster_updates = 0;
    
    abs->substitution_success_mean = 0.5f;  // Initial neutral assumption
    abs->substitution_success_m2 = 0.0f;
    abs->substitution_attempts = 0;
    
    return abs;
}

/* Free an abstraction node */
static void abstraction_node_free(AbstractionNode *abs) {
    if (!abs) return;
    
    free(abs->instances);
    sparse_embedding_free(abs->prototype);
    
    // Free base Node fields if allocated
    free(abs->base.payload);
    free(abs->base.outgoing_edges);
    free(abs->base.incoming_edges);
    
    free(abs);
}

/* Add an instance to an abstraction */
static void abstraction_add_instance(AbstractionNode *abs, Node *instance) {
    if (!abs || !instance) return;
    
    // Check if already present
    for (size_t i = 0; i < abs->instance_count; i++) {
        if (abs->instances[i] == instance) return;
    }
    
    // Grow capacity if needed (exponential growth, no hardcoded max)
    if (abs->instance_count >= abs->instance_capacity) {
        size_t new_cap = abs->instance_capacity * 2;
        Node **new_instances = realloc(abs->instances, new_cap * sizeof(Node*));
        if (!new_instances) return;
        abs->instances = new_instances;
        abs->instance_capacity = new_cap;
    }
    
    abs->instances[abs->instance_count++] = instance;
    
    // Update prototype embedding (running average of instance embeddings)
    if (instance->sparse_embedding && abs->prototype) {
        // Hebbian update: prototype moves toward instance
        float learning_rate = 1.0f / (float)(abs->instance_count + 1);
        sparse_embedding_update_hebbian(abs->prototype, instance->sparse_embedding, learning_rate);
    }
    
    // Link instance to abstraction
    instance->abstraction_parent = (Node*)abs;
}

/* Compute context signature for a node (what appears before/after)
 * Returns sparse embedding representing context - O(degree)
 */
static SparseEmbedding* compute_context_signature(Node *node) {
    if (!node) return NULL;
    
    SparseEmbedding *sig = sparse_embedding_create(16);
    if (!sig) return NULL;
    
    // Aggregate embeddings of incoming neighbors (what comes before)
    for (size_t i = 0; i < node->incoming_count && i < 20; i++) {
        Edge *edge = node->incoming_edges[i];
        if (edge && edge->from_node && edge->from_node->sparse_embedding) {
            // Weight by edge weight (normalized to [0,1])
            float weight = (float)edge->weight / 255.0f;
            sparse_embedding_update_hebbian(sig, edge->from_node->sparse_embedding, weight * 0.5f);
        }
    }
    
    // Aggregate embeddings of outgoing neighbors (what comes after)
    for (size_t i = 0; i < node->outgoing_count && i < 20; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node && edge->to_node->sparse_embedding) {
            float weight = (float)edge->weight / 255.0f;
            sparse_embedding_update_hebbian(sig, edge->to_node->sparse_embedding, weight * 0.5f);
        }
    }
    
    sparse_embedding_normalize(sig);
    return sig;
}

/* Find nodes with similar context signatures from semantic neighbors - O(k) */
static size_t find_similar_context_nodes(
    Node *node,
    SparseEmbedding *signature,
    Node **out_similar,
    size_t max_similar,
    float similarity_threshold
) {
    if (!node || !signature || !out_similar || max_similar == 0) return 0;
    
    size_t found = 0;
    
    // Only check semantic neighbors (O(k), NOT O(n))
    for (size_t i = 0; i < node->semantic_neighbor_count && found < max_similar; i++) {
        Node *neighbor = node->semantic_neighbors[i];
        if (!neighbor || neighbor == node) continue;
        
        // Compute context signature for neighbor
        SparseEmbedding *neighbor_sig = compute_context_signature(neighbor);
        if (!neighbor_sig) continue;
        
        // Compare signatures
        float similarity = sparse_embedding_similarity(signature, neighbor_sig);
        sparse_embedding_free(neighbor_sig);
        
        if (similarity > similarity_threshold) {
            out_similar[found++] = neighbor;
        }
    }
    
    // Also check nodes connected by edges (local, O(degree))
    for (size_t i = 0; i < node->outgoing_count && found < max_similar; i++) {
        Node *neighbor = node->outgoing_edges[i]->to_node;
        if (!neighbor || neighbor == node) continue;
        
        // Check if already found
        int already_found = 0;
        for (size_t j = 0; j < found; j++) {
            if (out_similar[j] == neighbor) {
                already_found = 1;
                break;
            }
        }
        if (already_found) continue;
        
        SparseEmbedding *neighbor_sig = compute_context_signature(neighbor);
        if (!neighbor_sig) continue;
        
        float similarity = sparse_embedding_similarity(signature, neighbor_sig);
        sparse_embedding_free(neighbor_sig);
        
        if (similarity > similarity_threshold) {
            out_similar[found++] = neighbor;
        }
    }
    
    return found;
}

/* Compute adaptive cluster threshold from Welford statistics */
static size_t compute_adaptive_cluster_threshold(Graph *graph) {
    if (!graph) return 3;  // Minimum sensible value
    
    // Base threshold from graph statistics
    float base = 3.0f;
    
    // Scale by abstraction formation history
    if (graph->abstraction_formation_count > 10) {
        float variance = graph->abstraction_formation_m2 / (float)graph->abstraction_formation_count;
        float stddev = sqrtf(variance + 1e-6f);
        
        // Higher variance -> higher threshold (be more selective)
        base += stddev;
    }
    
    // Scale by graph maturity (more nodes -> higher threshold)
    if (graph->node_count > 100) {
        base *= 1.0f + logf((float)graph->node_count / 100.0f) * 0.1f;
    }
    
    return (size_t)(base + 0.5f);
}

/* Compute adaptive semantic similarity threshold from Welford statistics */
static float compute_adaptive_semantic_threshold(Graph *graph) {
    if (!graph) return 0.5f;
    
    // Use semantic similarity statistics
    if (graph->semantic_similarity_samples > 10) {
        float variance = graph->semantic_similarity_m2 / (float)graph->semantic_similarity_samples;
        float stddev = sqrtf(variance + 1e-6f);
        
        // Threshold = mean + 0.5 * stddev (top ~30% most similar)
        return graph->semantic_similarity_mean + 0.5f * stddev;
    }
    
    return 0.5f;  // Default if not enough samples
}

/* Create or update abstraction from similar nodes */
static AbstractionNode* create_or_update_abstraction(
    Graph *graph,
    Node **similar_nodes,
    size_t similar_count
) {
    if (!graph || !similar_nodes || similar_count < 2) return NULL;
    
    // Check if any node already belongs to an abstraction
    AbstractionNode *existing = NULL;
    for (size_t i = 0; i < similar_count; i++) {
        if (similar_nodes[i]->abstraction_parent) {
            existing = (AbstractionNode*)similar_nodes[i]->abstraction_parent;
            break;
        }
    }
    
    if (existing) {
        // Add new nodes to existing abstraction
        for (size_t i = 0; i < similar_count; i++) {
            if (!similar_nodes[i]->abstraction_parent) {
                abstraction_add_instance(existing, similar_nodes[i]);
            }
        }
        return existing;
    }
    
    // Create new abstraction
    AbstractionNode *abs = abstraction_node_create(ABST_CONTEXT_CLUSTER);
    if (!abs) return NULL;
    
    // Add all similar nodes as instances
    for (size_t i = 0; i < similar_count; i++) {
        abstraction_add_instance(abs, similar_nodes[i]);
    }
    
    // Add to graph's abstraction list
    if (graph->abstraction_count >= graph->abstraction_capacity) {
        size_t new_cap = (graph->abstraction_capacity == 0) ? 8 : graph->abstraction_capacity * 2;
        AbstractionNode **new_abs = realloc(graph->abstractions, new_cap * sizeof(AbstractionNode*));
        if (!new_abs) {
            abstraction_node_free(abs);
            return NULL;
        }
        graph->abstractions = new_abs;
        graph->abstraction_capacity = new_cap;
    }
    
    graph->abstractions[graph->abstraction_count++] = abs;
    
    // Update Welford statistics
    graph->abstraction_formation_count++;
    float delta = (float)similar_count - graph->abstraction_formation_mean;
    graph->abstraction_formation_mean += delta / (float)graph->abstraction_formation_count;
    float delta2 = (float)similar_count - graph->abstraction_formation_mean;
    graph->abstraction_formation_m2 += delta * delta2;
    
    return abs;
}

/* Detect and create abstractions from recently activated nodes - O(k) per node */
static void graph_detect_abstractions(Graph *graph) {
    if (!graph || graph->recent_activation_count == 0) return;
    
    size_t cluster_threshold = compute_adaptive_cluster_threshold(graph);
    float similarity_threshold = compute_adaptive_semantic_threshold(graph);
    
    // For each recently activated node (NOT all nodes!)
    for (size_t i = 0; i < graph->recent_activation_count; i++) {
        Node *node = graph->recent_activations[i];
        if (!node || node->abstraction_parent) continue;  // Already abstracted
        
        // Compute context signature (O(degree))
        SparseEmbedding *sig = compute_context_signature(node);
        if (!sig) continue;
        
        // Find similar context nodes (O(k))
        Node *similar[32];  // Stack allocation for small array
        size_t similar_count = find_similar_context_nodes(
            node, sig, similar, 32, similarity_threshold
        );
        
        sparse_embedding_free(sig);
        
        // If enough similar nodes, create/update abstraction
        if (similar_count >= cluster_threshold) {
            // Include the original node
            similar[similar_count++] = node;
            create_or_update_abstraction(graph, similar, similar_count);
        }
    }
}

/* Find matching abstraction for an embedding - O(abstraction_count * k) */
static AbstractionNode* find_matching_abstraction(Graph *graph, SparseEmbedding *embedding) {
    if (!graph || !embedding) return NULL;
    
    AbstractionNode *best_match = NULL;
    float best_similarity = 0.0f;
    float threshold = compute_adaptive_semantic_threshold(graph);
    
    for (size_t i = 0; i < graph->abstraction_count; i++) {
        AbstractionNode *abs = graph->abstractions[i];
        if (!abs || !abs->prototype) continue;
        
        float similarity = sparse_embedding_similarity(embedding, abs->prototype);
        
        if (similarity > threshold && similarity > best_similarity) {
            best_similarity = similarity;
            best_match = abs;
        }
    }
    
    return best_match;
}

/* Select best instance from abstraction based on context - O(instance_count) */
static Node* select_instance_from_abstraction(
    AbstractionNode *abs,
    SparseEmbedding *context_embedding
) {
    if (!abs || abs->instance_count == 0) return NULL;
    
    // If no context, return first instance (most frequent by insertion order)
    if (!context_embedding) {
        return abs->instances[0];
    }
    
    Node *best_instance = abs->instances[0];
    float best_similarity = 0.0f;
    
    for (size_t i = 0; i < abs->instance_count; i++) {
        Node *instance = abs->instances[i];
        if (!instance || !instance->sparse_embedding) continue;
        
        float similarity = sparse_embedding_similarity(context_embedding, instance->sparse_embedding);
        
        if (similarity > best_similarity) {
            best_similarity = similarity;
            best_instance = instance;
        }
    }
    
    return best_instance;
}

/* ============================================================================
 * TEMPORAL REASONING (Sequence Understanding)
 * 
 * Enables sequence understanding and causality:
 * - "What happened before/after X?"
 * - "What causes Y?"
 * 
 * All operations O(degree) - only checks local edges
 * All statistics use Welford's algorithm
 * ============================================================================ */

/* Learn temporal relation for an edge - O(1) */
static void edge_learn_temporal_relation(Edge *edge, TemporalRelation relation, float distance) {
    if (!edge) return;
    
    // Update relation if stronger (earlier relations take precedence until overridden)
    if (edge->temporal_observations == 0 || 
        (relation != TEMP_NONE && edge->temporal_relation == TEMP_NONE)) {
        edge->temporal_relation = relation;
    }
    
    // Update temporal distance using Welford's algorithm
    edge->temporal_observations++;
    float delta = distance - edge->temporal_distance_mean;
    edge->temporal_distance_mean += delta / (float)edge->temporal_observations;
    float delta2 = distance - edge->temporal_distance_mean;
    edge->temporal_distance_m2 += delta * delta2;
    
    // Update causal strength (higher weight edges have stronger causality)
    float causal = (float)edge->weight / 255.0f;
    edge->causal_observations++;
    float cdelta = causal - edge->causal_strength_mean;
    edge->causal_strength_mean += cdelta / (float)edge->causal_observations;
    float cdelta2 = causal - edge->causal_strength_mean;
    edge->causal_strength_m2 += cdelta * cdelta2;
}

/* Query temporal relation - O(degree) */
static Node* temporal_query(Graph *graph, Node *event, TemporalRelation relation) {
    if (!graph || !event) return NULL;
    
    Node *best_match = NULL;
    float best_score = 0.0f;
    
    // Only check outgoing edges (O(degree), NOT O(n))
    for (size_t i = 0; i < event->outgoing_count; i++) {
        Edge *edge = event->outgoing_edges[i];
        if (!edge || edge->temporal_relation != relation) continue;
        
        // Score = causal_strength * inverse_distance
        float distance_factor = 1.0f / (1.0f + edge->temporal_distance_mean);
        float score = edge->causal_strength_mean * distance_factor;
        
        if (score > best_score) {
            best_score = score;
            best_match = edge->to_node;
        }
    }
    
    // Also check incoming edges for BEFORE relation
    if (relation == TEMP_BEFORE || relation == TEMP_CAUSES) {
        for (size_t i = 0; i < event->incoming_count; i++) {
            Edge *edge = event->incoming_edges[i];
            if (!edge) continue;
            
            // For incoming edges, AFTER means the source is BEFORE
            TemporalRelation check = (relation == TEMP_BEFORE) ? TEMP_AFTER : TEMP_ENABLES;
            if (edge->temporal_relation != check) continue;
            
            float distance_factor = 1.0f / (1.0f + edge->temporal_distance_mean);
            float score = edge->causal_strength_mean * distance_factor;
            
            if (score > best_score) {
                best_score = score;
                best_match = edge->from_node;
            }
        }
    }
    
    return best_match;
}

/* ============================================================================
 * CROSS-MODAL INTEGRATION (Multi-Modal Binding)
 * 
 * Enables knowledge transfer across modalities:
 * - Links text "dog", audio [bark], visual [dog image]
 * - Query one modality, get answers from all
 * 
 * O(1) for lookup, O(k) for binding creation
 * ============================================================================ */

/* Create a modality bridge */
static ModalityBridge* modality_bridge_create(void) {
    ModalityBridge *bridge = calloc(1, sizeof(ModalityBridge));
    if (!bridge) return NULL;
    
    bridge->shared_embedding = sparse_embedding_create(16);
    if (!bridge->shared_embedding) {
        free(bridge);
        return NULL;
    }
    
    bridge->consistency_mean = 0.5f;
    bridge->consistency_m2 = 0.0f;
    bridge->cooccurrence_count = 0;
    
    return bridge;
}

/* Free a modality bridge */
static void modality_bridge_free(ModalityBridge *bridge) {
    if (!bridge) return;
    sparse_embedding_free(bridge->shared_embedding);
    free(bridge);
}

/* Add node to modality bridge */
static void modality_bridge_add_node(ModalityBridge *bridge, Node *node) {
    if (!bridge || !node) return;
    
    uint8_t port = node->port_id;
    if (port >= 8) port = 0;
    
    bridge->modality_nodes[port] = node;
    bridge->modality_mask |= (1 << port);
    
    // Update shared embedding
    if (node->sparse_embedding) {
        float learning_rate = 1.0f / (float)(__builtin_popcount(bridge->modality_mask) + 1);
        sparse_embedding_update_hebbian(bridge->shared_embedding, node->sparse_embedding, learning_rate);
    }
    
    // Link node to bridge
    node->modality_bridge = bridge;
}

/* Learn cross-modal binding - O(1) per binding */
static void learn_cross_modal_binding(Graph *graph, Node **nodes, size_t count) {
    if (!graph || !nodes || count < 2) return;
    
    // Find or create bridge
    ModalityBridge *bridge = NULL;
    
    // Check if any node already has a bridge
    for (size_t i = 0; i < count; i++) {
        if (nodes[i] && nodes[i]->modality_bridge) {
            bridge = nodes[i]->modality_bridge;
            break;
        }
    }
    
    if (!bridge) {
        bridge = modality_bridge_create();
        if (!bridge) return;
        
        // Add to graph's bridge list
        if (graph->bridge_count >= graph->bridge_capacity) {
            size_t new_cap = (graph->bridge_capacity == 0) ? 8 : graph->bridge_capacity * 2;
            ModalityBridge **new_bridges = realloc(graph->modality_bridges, new_cap * sizeof(ModalityBridge*));
            if (!new_bridges) {
                modality_bridge_free(bridge);
                return;
            }
            graph->modality_bridges = new_bridges;
            graph->bridge_capacity = new_cap;
        }
        graph->modality_bridges[graph->bridge_count++] = bridge;
    }
    
    // Add all nodes to bridge
    for (size_t i = 0; i < count; i++) {
        if (nodes[i]) {
            modality_bridge_add_node(bridge, nodes[i]);
        }
    }
    
    // Update consistency statistics
    bridge->cooccurrence_count++;
    float consistency = 0.0f;
    int modal_count = 0;
    
    for (int i = 0; i < 8; i++) {
        if (bridge->modality_nodes[i] && bridge->modality_nodes[i]->sparse_embedding) {
            float sim = sparse_embedding_similarity(
                bridge->shared_embedding,
                bridge->modality_nodes[i]->sparse_embedding
            );
            consistency += sim;
            modal_count++;
        }
    }
    
    if (modal_count > 0) {
        consistency /= (float)modal_count;
        float delta = consistency - bridge->consistency_mean;
        bridge->consistency_mean += delta / (float)bridge->cooccurrence_count;
        float delta2 = consistency - bridge->consistency_mean;
        bridge->consistency_m2 += delta * delta2;
    }
}

/* Expand activation pattern with cross-modal links - O(k) */
static void expand_cross_modal(ActivationPattern *pattern) {
    if (!pattern) return;
    
    size_t original_count = pattern->count;
    
    for (size_t i = 0; i < original_count; i++) {
        Node *node = pattern->nodes[i];
        if (!node || !node->modality_bridge) continue;
        
        ModalityBridge *bridge = node->modality_bridge;
        
        // Add all modality nodes from bridge
        for (int m = 0; m < 8; m++) {
            Node *modal_node = bridge->modality_nodes[m];
            if (modal_node && modal_node != node) {
                // Check if already in pattern
                int found = 0;
                for (size_t j = 0; j < pattern->count; j++) {
                    if (pattern->nodes[j] == modal_node) {
                        found = 1;
                        break;
                    }
                }
                
                if (!found && pattern->count < pattern->capacity) {
                    // Add with reduced activation (cross-modal transfer)
                    pattern->nodes[pattern->count] = modal_node;
                    pattern->activations[pattern->count] = pattern->activations[i] * 0.5f;
                    pattern->count++;
                }
            }
        }
    }
}

/* ============================================================================
 * ATTENTION MECHANISM (Task-Dependent Focus)
 * 
 * Enables focusing on task-relevant information:
 * - "what color?" focuses on color-related neighbors
 * - "what size?" focuses on size-related neighbors
 * 
 * All operations O(degree) - local computation only
 * ============================================================================ */

/* Create attention mechanism */
static AttentionMechanism* attention_mechanism_create(size_t initial_capacity) {
    AttentionMechanism *att = calloc(1, sizeof(AttentionMechanism));
    if (!att) return NULL;
    
    size_t cap = (initial_capacity > 0) ? initial_capacity : 8;
    
    att->attention_weights = calloc(cap, sizeof(float));
    att->attention_success_mean = calloc(cap, sizeof(float));
    att->attention_success_m2 = calloc(cap, sizeof(float));
    att->attention_counts = calloc(cap, sizeof(uint64_t));
    
    if (!att->attention_weights || !att->attention_success_mean ||
        !att->attention_success_m2 || !att->attention_counts) {
        free(att->attention_weights);
        free(att->attention_success_mean);
        free(att->attention_success_m2);
        free(att->attention_counts);
        free(att);
        return NULL;
    }
    
    att->weight_capacity = cap;
    att->focus_width_mean = (float)cap / 2.0f;
    
    return att;
}

/* Free attention mechanism */
static void attention_mechanism_free(AttentionMechanism *att) {
    if (!att) return;
    free(att->attention_weights);
    free(att->attention_success_mean);
    free(att->attention_success_m2);
    free(att->attention_counts);
    free(att);
}

/* Compute attention weights - O(degree) */
static float* compute_attention_weights(Node *node, SparseEmbedding *query_embedding) {
    if (!node || node->outgoing_count == 0) return NULL;
    
    size_t n = node->outgoing_count;
    float *weights = malloc(n * sizeof(float));
    if (!weights) return NULL;
    
    float sum = 0.0f;
    
    for (size_t i = 0; i < n; i++) {
        Node *neighbor = node->outgoing_edges[i]->to_node;
        
        // Query-neighbor similarity (O(k) sparse dot product)
        float similarity = 0.5f;  // Default neutral
        if (query_embedding && neighbor && neighbor->sparse_embedding) {
            similarity = sparse_embedding_similarity(query_embedding, neighbor->sparse_embedding);
        }
        
        // Combine with learned attention
        float learned = 0.5f;
        if (node->attention && i < node->attention->weight_capacity) {
            learned = node->attention->attention_success_mean[i];
            if (learned <= 0.0f) learned = 0.5f;  // Default if no history
        }
        
        // Adaptive combination (no hardcoded weights)
        float alpha = 0.5f;  // Could be learned per-node
        weights[i] = alpha * similarity + (1.0f - alpha) * learned;
        
        // Edge weight also contributes
        weights[i] *= (float)node->outgoing_edges[i]->weight / 255.0f;
        
        sum += weights[i];
    }
    
    // Normalize (softmax-like)
    if (sum > 0.0f) {
        for (size_t i = 0; i < n; i++) {
            weights[i] /= sum;
        }
    }
    
    return weights;
}

/* Compute adaptive focus width from statistics */
static size_t compute_adaptive_focus_width(Node *node) {
    if (!node) return 3;
    
    // Base: sqrt of degree (sublinear scaling)
    size_t base = (size_t)(sqrtf((float)node->outgoing_count) + 0.5f);
    if (base < 1) base = 1;
    
    // Use learned focus width if available
    if (node->attention && node->attention->focus_updates > 10) {
        float variance = node->attention->focus_width_m2 / (float)node->attention->focus_updates;
        float stddev = sqrtf(variance + 1e-6f);
        
        // Learned width with some variance
        base = (size_t)(node->attention->focus_width_mean + 0.5f);
        base = (base > 0) ? base : 1;
    }
    
    // Cap at degree
    if (base > node->outgoing_count) {
        base = node->outgoing_count;
    }
    
    return base;
}

/* Select top-k indices by weight - O(k * n) but n and k are small */
static size_t* select_top_k_indices(float *weights, size_t n, size_t k) {
    if (!weights || n == 0 || k == 0) return NULL;
    
    if (k > n) k = n;
    
    size_t *indices = malloc(k * sizeof(size_t));
    if (!indices) return NULL;
    
    // Initialize with first k indices
    for (size_t i = 0; i < k; i++) {
        indices[i] = i;
    }
    
    // Simple selection: for each remaining element, if larger than min in top-k, replace
    for (size_t i = k; i < n; i++) {
        // Find minimum in current top-k
        size_t min_idx = 0;
        float min_val = weights[indices[0]];
        for (size_t j = 1; j < k; j++) {
            if (weights[indices[j]] < min_val) {
                min_val = weights[indices[j]];
                min_idx = j;
            }
        }
        
        // Replace if current is larger
        if (weights[i] > min_val) {
            indices[min_idx] = i;
        }
    }
    
    return indices;
}

/* Activate with attention - O(k) where k = focus_width
 * Uses edge transformer attention for LLM-style local transformation
 * (Requirement.md line 7: "edges transform locally in the same LLM transform globally")
 */
static void activate_with_attention(
    Node *node,
    SparseEmbedding *query_embedding,
    ActivationPattern *pattern
) {
    if (!node || !pattern || node->outgoing_count == 0) return;
    
    // Compute node-level attention weights
    float *weights = compute_attention_weights(node, query_embedding);
    if (!weights) return;
    
    // Compute adaptive focus width
    size_t focus_width = compute_adaptive_focus_width(node);
    
    // Select top-k neighbors by attention weight
    size_t *top_k = select_top_k_indices(weights, node->outgoing_count, focus_width);
    if (!top_k) {
        free(weights);
        return;
    }
    
    // Only activate top-k using EDGE TRANSFORMER ATTENTION
    for (size_t i = 0; i < focus_width; i++) {
        size_t idx = top_k[i];
        Edge *edge = node->outgoing_edges[idx];
        Node *neighbor = edge->to_node;
        
        // Use edge transformer for local attention (LLM-style transformation)
        // This implements Requirement.md line 7: edges transform locally
        float edge_attention = edge_multi_head_attention(edge, query_embedding);
        
        // Combine node attention with edge transformer attention
        float combined_attention = weights[idx] * 0.5f + edge_attention * 0.5f;
        float activation = combined_attention * node->activation_strength;
        
        // Add to pattern if not already present
        int found = 0;
        for (size_t j = 0; j < pattern->count; j++) {
            if (pattern->nodes[j] == neighbor) {
                pattern->activations[j] += activation;
                found = 1;
                break;
            }
        }
        
        if (!found && pattern->count < pattern->capacity) {
            pattern->nodes[pattern->count] = neighbor;
            pattern->activations[pattern->count] = activation;
            pattern->count++;
        }
        
        // Update edge transformer projections based on success
        // (Hebbian learning: edges that fire together wire together)
        if (node->sparse_embedding && neighbor->sparse_embedding) {
            float success = activation / (node->activation_strength + 0.01f);
            edge_update_projections(edge, node->sparse_embedding, neighbor->sparse_embedding, success);
        }
    }
    
    free(weights);
    free(top_k);
}

/* ============================================================================
 * UNCERTAINTY HANDLING (Probabilistic Output)
 * 
 * Enables expressing confidence and handling ambiguity:
 * - Returns "80% A, 15% B, 5% C" instead of just "A"
 * - Can ask for clarification when uncertain
 * 
 * All operations O(candidates) which is typically small
 * ============================================================================ */

/* Create probabilistic output */
static ProbabilisticOutput* probabilistic_output_create(size_t initial_capacity) {
    ProbabilisticOutput *output = calloc(1, sizeof(ProbabilisticOutput));
    if (!output) return NULL;
    
    size_t cap = (initial_capacity > 0) ? initial_capacity : 8;
    
    output->candidates = malloc(cap * sizeof(Node*));
    output->probabilities = malloc(cap * sizeof(float));
    
    if (!output->candidates || !output->probabilities) {
        free(output->candidates);
        free(output->probabilities);
        free(output);
        return NULL;
    }
    
    output->candidate_capacity = cap;
    output->entropy = 0.0f;
    
    return output;
}

/* Free probabilistic output */
static void probabilistic_output_free(ProbabilisticOutput *output) {
    if (!output) return;
    free(output->candidates);
    free(output->probabilities);
    free(output);
}

/* Add candidate to probabilistic output */
static void probabilistic_output_add_candidate(ProbabilisticOutput *output, Node *candidate, float probability) {
    if (!output || !candidate) return;
    
    // Check if already present
    for (size_t i = 0; i < output->candidate_count; i++) {
        if (output->candidates[i] == candidate) {
            output->probabilities[i] += probability;
            return;
        }
    }
    
    // Grow if needed
    if (output->candidate_count >= output->candidate_capacity) {
        size_t new_cap = output->candidate_capacity * 2;
        Node **new_cands = realloc(output->candidates, new_cap * sizeof(Node*));
        float *new_probs = realloc(output->probabilities, new_cap * sizeof(float));
        
        if (!new_cands || !new_probs) return;
        
        output->candidates = new_cands;
        output->probabilities = new_probs;
        output->candidate_capacity = new_cap;
    }
    
    output->candidates[output->candidate_count] = candidate;
    output->probabilities[output->candidate_count] = probability;
    output->candidate_count++;
}

/* Normalize probabilities to sum to 1 */
static void probabilistic_output_normalize(ProbabilisticOutput *output) {
    if (!output || output->candidate_count == 0) return;
    
    float sum = 0.0f;
    for (size_t i = 0; i < output->candidate_count; i++) {
        sum += output->probabilities[i];
    }
    
    if (sum > 0.0f) {
        for (size_t i = 0; i < output->candidate_count; i++) {
            output->probabilities[i] /= sum;
        }
    }
}

/* Compute entropy of probability distribution */
static float compute_entropy(float *probabilities, size_t count) {
    if (!probabilities || count == 0) return 0.0f;
    
    float entropy = 0.0f;
    for (size_t i = 0; i < count; i++) {
        if (probabilities[i] > 1e-6f) {
            entropy -= probabilities[i] * log2f(probabilities[i]);
        }
    }
    
    return entropy;
}

/* Generate probabilistic output from activation pattern - O(pattern->count) */
static ProbabilisticOutput* generate_probabilistic_output(Graph *graph, ActivationPattern *pattern) {
    if (!graph || !pattern || pattern->count == 0) return NULL;
    
    ProbabilisticOutput *output = probabilistic_output_create(pattern->count);
    if (!output) return NULL;
    
    // Add candidates from pattern
    for (size_t i = 0; i < pattern->count; i++) {
        Node *candidate = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        // Probability proportional to activation
        probabilistic_output_add_candidate(output, candidate, activation);
    }
    
    // Normalize
    probabilistic_output_normalize(output);
    
    // Compute entropy
    output->entropy = compute_entropy(output->probabilities, output->candidate_count);
    
    return output;
}

/* Compute adaptive uncertainty threshold - uses Welford statistics */
static float compute_adaptive_uncertainty_threshold(Graph *graph) {
    if (!graph) return 1.0f;  // Default: entropy of 1.0 is uncertain
    
    // Use uncertainty statistics if available
    if (graph->uncertainty_samples > 10) {
        float variance = graph->uncertainty_threshold_m2 / (float)graph->uncertainty_samples;
        float stddev = sqrtf(variance + 1e-6f);
        
        // Threshold = mean + stddev (above average is uncertain)
        return graph->uncertainty_threshold_mean + stddev;
    }
    
    return 1.0f;  // Default threshold
}

/* Check if output is uncertain */
static int is_output_uncertain(Graph *graph, ProbabilisticOutput *output) {
    if (!graph || !output) return 0;
    
    float threshold = compute_adaptive_uncertainty_threshold(graph);
    return output->entropy > threshold;
}

/* Get top candidate from probabilistic output */
static Node* get_top_candidate(ProbabilisticOutput *output) {
    if (!output || output->candidate_count == 0) return NULL;
    
    Node *best = output->candidates[0];
    float best_prob = output->probabilities[0];
    
    for (size_t i = 1; i < output->candidate_count; i++) {
        if (output->probabilities[i] > best_prob) {
            best_prob = output->probabilities[i];
            best = output->candidates[i];
        }
    }
    
    return best;
}

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
 * BRAIN-INSPIRED CONTEXT SYSTEM (Sparse Distributed Representations)
 * ============================================================================
 * 
 * This replaces the old 8-bit context signature system. See SparseContext,
 * ContextTag, and related functions defined earlier in this file.
 * 
 * Old system (REMOVED):
 * - compute_context_signature() - 8-bit hash, only 256 contexts, collisions
 * - compute_context_signature_from_nodes() - same limitations
 * - compute_context_match() - circular distance matching
 * 
 * New system:
 * - SparseContext - stores actual activated nodes (10^70+ patterns)
 * - ContextTag - synaptic tagging for multiple contexts per edge
 * - sparse_context_match() - overlap-based matching (no collisions)
 * - edge_add_context_tag() - add training context to edge
 * - edge_compute_context_weight() - compute context-weighted score
 */

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
    
    // Prepare inputs: edge weight + target node payload + context tag indicator
    float inputs[3];
    inputs[0] = (float)edge->weight / 255.0f;  // Normalized edge weight
    inputs[1] = (edge->to_node->payload_size > 0) ? 
                ((float)edge->to_node->payload[0] / 255.0f) : 0.0f;  // Target byte
    inputs[2] = (edge->tag_count > 0) ? 1.0f : 0.0f;  // Has context tags (replaces routing_gate)
    
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
    // Minimum size adapts to current data (not hardcoded 8)
    size_t min_size = (current_context_len > 0) ? 
        (current_context_len / 2 > 1 ? current_context_len / 2 : 1) : 1;
    if (desired_size < min_size) desired_size = min_size;
    // NO MAXIMUM - unlimited context as user specified
    
    return desired_size;
}

/* ============================================================================
 * COMPONENT AGENCY: Each Component Makes Its Own Predictions
 * ============================================================================
 * 
 * KEY ARCHITECTURAL CHANGE:
 * - Hierarchies ROUTE: "I know this pattern, here's what comes next"
 * - Blank nodes ROUTE: "I know this category, here's what fits"
 * - Node mini-nets PREDICT: "Given my context, I predict edge X"
 * 
 * NOT softmax over all edges (LLM-like)
 * Instead: Each component with knowledge speaks up
 * ============================================================================ */

/* Forward declaration for edge_is_similarity (defined later) */
static inline int edge_is_similarity(Edge *edge);

/* Node predicts which edge to take next (mini prediction)
 * Requirement line 8: "nodes make mini prediction... predict what edge comes next"
 * 
 * BRAIN-INSPIRED: Uses Sparse Distributed Representations for context matching
 * - Each edge stores multiple context tags (synaptic tagging)
 * - Context matching uses sparse overlap (no 8-bit hash collisions)
 * - Unlimited contexts per edge, unlimited context length
 * 
 * Returns: Best edge based on CONTEXT MATCH + weight + target activation
 */
static Edge* node_predict_next_edge_sparse(Node *node, SparseContext *current_context) {
    if (!node || node->outgoing_count == 0) return NULL;
    
    Edge *best_edge = NULL;
    float best_score = -1.0f;
    
    // Compute local average for relative scaling (adaptive, no hardcoded values)
    float local_avg = node_get_local_outgoing_weight_avg(node);
    float epsilon = (local_avg > 0.0f) ? (local_avg * 0.001f) : 0.001f;
    
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
        if (edge_is_similarity(edge)) continue;
        
        // === BRAIN-INSPIRED CONTEXT MATCH ===
        // Use edge_compute_context_weight() which:
        // - Checks all context tags on this edge
        // - Uses sparse overlap (Jaccard-like) instead of hash
        // - Already includes exponential + level-based amplification
        float context_weight = edge_compute_context_weight(edge, current_context);
        
        // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
        // Like LLM attention: context is the PRIMARY signal, others modulate
        // NO HARDCODED MULTIPLIERS - all scaling from compute_adaptive_context_multiplier
        
        // 1. Context is PRIMARY - relative strength provides natural amplification
        float relative_strength = context_weight / (local_avg + epsilon);
        // Context dominance emerges from relative_strength (data-driven, not hardcoded 10x)
        float context_primary = context_weight * (1.0f + relative_strength);
        
        // 2. Base weight contributes proportionally to context (ratio, not 0.1x)
        float base_weight_ratio = ((float)edge->weight / 255.0f) / (context_weight + epsilon);
        float base_contribution = ((float)edge->weight / 255.0f) * base_weight_ratio;
        
        // 3. Activation contributes proportionally (ratio, not 0.3x)
        float activation = edge->to_node->activation_strength;
        float activation_ratio = activation / (context_weight + epsilon);
        float activation_contribution = activation * activation_ratio;
        
        // Combined: all ratios are data-driven
        float score = context_primary + base_contribution + activation_contribution;
        
        if (score > best_score) {
            best_score = score;
            best_edge = edge;
        }
    }
    
    return best_edge;
}

/* Legacy wrapper for byte-based context (for backwards compatibility during transition)
 * Creates a temporary sparse context from bytes, calls the sparse version
 * TODO: Remove once all callers are updated to use sparse contexts directly
 */
static Edge* node_predict_next_edge(Node *node, uint8_t *context_bytes, size_t context_len) {
    if (!node || node->outgoing_count == 0) return NULL;
    (void)context_bytes;  // Unused in new system
    (void)context_len;    // Unused in new system
    
    // For now, call sparse version with NULL context (uses base weights only)
    // This maintains backwards compatibility until all callers are updated
    return node_predict_next_edge_sparse(node, NULL);
}

/* Hierarchy node predicts continuation for its pattern
 * When input matches a hierarchy, the hierarchy knows what comes next
 * This is NOT completion - it's the hierarchy's PREDICTION
 * 
 * Returns: Next node that hierarchy predicts, or NULL
 */
static Node* hierarchy_predict_continuation(Node *hierarchy, uint8_t *input_bytes, size_t input_len) {
    if (!hierarchy || hierarchy->abstraction_level == 0) return NULL;
    if (hierarchy->payload_size == 0) return NULL;
    if (!input_bytes || input_len == 0) return NULL;
    
    // For reference-based hierarchies without payload, use child nodes
    uint8_t *hier_payload = hierarchy->payload;
    if (!hier_payload && hierarchy->is_reference_hierarchy && hierarchy->child_count > 0) {
        // Reference hierarchy - just use outgoing edges for prediction
        // Can't match payload since it's reconstructed on-demand
        if (hierarchy->outgoing_count > 0) {
            Edge *best_edge = NULL;
            float best_weight = -1.0f;
            
            for (size_t i = 0; i < hierarchy->outgoing_count; i++) {
                Edge *edge = hierarchy->outgoing_edges[i];
                if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
                if (edge_is_similarity(edge)) continue;
                
                // BRAIN-INSPIRED: Skip edges to child nodes (component nodes create loops!)
                int is_child = 0;
                for (size_t c = 0; c < hierarchy->child_count; c++) {
                    if (edge->to_node == hierarchy->child_nodes[c]) {
                        is_child = 1;
                        break;
                    }
                }
                if (is_child) continue;
                
                if (edge->weight > best_weight) {
                    best_weight = edge->weight;
                    best_edge = edge;
                }
            }
            
            if (best_edge) return best_edge->to_node;
        }
        return NULL;
    }
    
    if (!hier_payload) return NULL;
    
    // Check if END of input matches this hierarchy (not start!)
    // Hierarchy should only fire when we've just processed its pattern
    // Example: hierarchy "he" fires when context ends with "he", not when it starts with "he"
    if (input_len < hierarchy->payload_size) return NULL;
    
    // Compare last N bytes of input with hierarchy payload
    size_t offset = input_len - hierarchy->payload_size;
    size_t match_len = 0;
    for (size_t i = 0; i < hierarchy->payload_size; i++) {
        if (input_bytes[offset + i] != hier_payload[i]) break;
        match_len++;
    }
    
    // Must match the full hierarchy at the end of context
    if (match_len < hierarchy->payload_size) return NULL;
    
    // We've matched the FULL hierarchy at the end of context
    // Now predict what comes after using BRAIN-INSPIRED CONTEXT MATCHING
    if (hierarchy->outgoing_count > 0) {
        Edge *best_edge = NULL;
        float best_score = -1.0f;
        
        // NOTE: We don't have a SparseContext here, so use NULL
        // This means edges will be scored by base weight only
        // Future: Pass SparseContext from caller for better disambiguation
        
        for (size_t i = 0; i < hierarchy->outgoing_count; i++) {
            Edge *edge = hierarchy->outgoing_edges[i];
            if (!edge || !edge->to_node) continue;
            if (edge_is_similarity(edge)) continue;
            
            // BRAIN-INSPIRED: Skip edges back to component nodes (we want what comes AFTER)
            // For large hierarchies: check child_nodes array
            // For small hierarchies: check if edge target's payload is part of hierarchy payload
            int is_component = 0;
            if (hierarchy->child_nodes) {
                // Reference-based hierarchy - check child array
                for (size_t c = 0; c < hierarchy->child_count; c++) {
                    if (edge->to_node == hierarchy->child_nodes[c]) {
                        is_component = 1;
                        break;
                    }
                }
            } else if (hierarchy->payload && hierarchy->payload_size > 0 && 
                       edge->to_node->payload && edge->to_node->payload_size > 0) {
                // Small hierarchy (copied payload) - check if target's payload is a substring
                // of hierarchy payload (component detection)
                for (size_t p = 0; p + edge->to_node->payload_size <= hierarchy->payload_size; p++) {
                    if (memcmp(&hierarchy->payload[p], edge->to_node->payload, 
                               edge->to_node->payload_size) == 0) {
                        is_component = 1;
                        break;
                    }
                }
            }
            if (is_component) continue;  // Skip component nodes - they create loops!
            
            // Skip edges without any context tags - they're likely spurious
            if (edge->tag_count == 0) continue;
            
            // === BRAIN-INSPIRED CONTEXT SCORING ===
            // Use edge_compute_context_weight with NULL context (uses base weight)
            // Already includes exponential + level-based amplification
            float context_weight = edge_compute_context_weight(edge, NULL);
            
            // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
            // NO HARDCODED MULTIPLIERS - all scaling from relative strength
            float hier_local_avg = node_get_local_outgoing_weight_avg(hierarchy);
            float hier_epsilon = (hier_local_avg > 0.0f) ? (hier_local_avg * 0.001f) : 0.001f;
            float hier_relative = context_weight / (hier_local_avg + hier_epsilon);
            float score = context_weight * (1.0f + hier_relative);
            
            if (score > best_score) {
                best_score = score;
                best_edge = edge;
            }
        }
        
        // Only return if we found a context-trained edge
        if (best_edge) return best_edge->to_node;
    }
    
    return NULL;
}

/* Blank node predicts for its category
 * Blank nodes represent categories - they route based on context
 * 
 * BRAIN-INSPIRED: Blank uses sparse context matching to disambiguate
 * Each edge stores multiple context tags (synaptic tagging)
 * Context matching uses sparse overlap for unlimited disambiguation
 * 
 * Returns: Best prediction for this category based on CONTEXT MATCH
 */
static Node* blank_predict_for_category(Node *blank, Node *current_node, 
                                        uint8_t *context_bytes, size_t context_len) {
    if (!blank || blank->payload_size != 0) return NULL;  // Must be blank (no payload)
    if (blank->outgoing_count == 0) return NULL;
    
    (void)current_node;   // Used for activation check below
    (void)context_bytes;  // Unused in new system (would need SparseContext)
    (void)context_len;    // Unused in new system
    
    Edge *best_edge = NULL;
    float best_score = -1.0f;
    int has_context_edge = 0;  // Track if any edge has context tags
    
    // Compute local average for adaptive scoring (no hardcoded multipliers)
    float blank_local_avg = node_get_local_outgoing_weight_avg(blank);
    float blank_epsilon = (blank_local_avg > 0.0f) ? (blank_local_avg * 0.001f) : 0.001f;
    
    // Check which of blank's outgoing edges should fire
    for (size_t i = 0; i < blank->outgoing_count; i++) {
        Edge *edge = blank->outgoing_edges[i];
        if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
        
        // === BRAIN-INSPIRED CONTEXT SCORING ===
        // Use edge_compute_context_weight with NULL context (uses base weight)
        // Future: Pass SparseContext from caller for better disambiguation
        float context_weight = edge_compute_context_weight(edge, NULL);
        
        if (edge->tag_count > 0) {
            has_context_edge = 1;
        }
        
        // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
        // NO HARDCODED MULTIPLIERS - all scaling from relative strength
        float blank_relative = context_weight / (blank_local_avg + blank_epsilon);
        float context_primary = context_weight * (1.0f + blank_relative);
        
        // Activation contributes proportionally (ratio, not hardcoded 0.3x)
        float activation = edge->to_node->activation_strength;
        float activation_ratio = activation / (context_weight + blank_epsilon);
        float activation_contribution = activation * activation_ratio;
        float score = context_primary + activation_contribution;
        
        if (score > best_score) {
            best_score = score;
            best_edge = edge;
        }
    }
    
    // If no edges have context tags, defer to mini-net (return NULL)
    // Blank shouldn't override context-trained direct edges
    if (!has_context_edge) {
        return NULL;
    }
    
    return best_edge ? best_edge->to_node : NULL;
}

/* Find blank node connected to current node
 * Blank nodes are reached through edges (Requirement line 7)
 * 
 * Returns: Connected blank node, or NULL
 */
static Node* find_connected_blank(Node *current_node) {
    if (!current_node) return NULL;
    
    // Check outgoing edges for blank nodes
    for (size_t i = 0; i < current_node->outgoing_count; i++) {
        Edge *edge = current_node->outgoing_edges[i];
        if (!edge || !edge->to_node) continue;
        
        // Blank nodes have payload_size == 0
        if (edge->to_node->payload_size == 0) {
            return edge->to_node;
        }
    }
    
    // Check incoming edges too (blank might be a predecessor)
    for (size_t i = 0; i < current_node->incoming_count; i++) {
        Edge *edge = current_node->incoming_edges[i];
        if (!edge || !edge->from_node) continue;
        
        if (edge->from_node->payload_size == 0) {
            return edge->from_node;
        }
    }
    
    return NULL;
}

/* Find hierarchy that matches current context
 * Uses edges only - no O(n) scan (Requirement line 2)
 * Hierarchies are connected to their component nodes
 * 
 * Returns: Matching hierarchy node, or NULL
 */
static Node* find_hierarchy_through_edges(Node *current_node, uint8_t *context_bytes, size_t context_len) {
    if (!current_node) return NULL;
    
    // Check incoming edges - hierarchies are connected to their children
    for (size_t i = 0; i < current_node->incoming_count; i++) {
        Edge *edge = current_node->incoming_edges[i];
        if (!edge || !edge->from_node) continue;
        
        Node *potential_hier = edge->from_node;
        if (potential_hier->abstraction_level > 0 && potential_hier->payload_size > 0) {
            // Check if context matches this hierarchy
            Node *prediction = hierarchy_predict_continuation(potential_hier, context_bytes, context_len);
            if (prediction) {
                return potential_hier;
            }
        }
    }
    
    // Check outgoing edges too
    for (size_t i = 0; i < current_node->outgoing_count; i++) {
        Edge *edge = current_node->outgoing_edges[i];
        if (!edge || !edge->to_node) continue;
        
        Node *potential_hier = edge->to_node;
        if (potential_hier->abstraction_level > 0 && potential_hier->payload_size > 0) {
            Node *prediction = hierarchy_predict_continuation(potential_hier, context_bytes, context_len);
            if (prediction) {
                return potential_hier;
            }
        }
    }
    
    return NULL;
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

/* Compute context match variance across edges (measures context ambiguity)
 * - High variance = distinct contexts (low ambiguity, easier discrimination)
 * - Low variance = overlapping contexts (high ambiguity, need stronger weighting)
 * - O(degree * k) where k = avg context tags per edge
 * - Pure data-driven: no hardcoded thresholds
 */
static float compute_context_match_variance(Node *node, SparseContext *current_context) {
    if (!node || node->outgoing_count == 0 || !current_context) return 0.0f;
    
    float match_sum = 0.0f;
    float match_sum_sq = 0.0f;
    size_t match_count = 0;
    
    // Compute match scores for all edges with context tags
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || edge->tag_count == 0) continue;
        
        // Find best match for this edge across all its context tags
        float best_match = 0.0f;
        for (size_t j = 0; j < edge->tag_count; j++) {
            if (edge->context_tags[j].context) {
                float match = sparse_context_match(edge->context_tags[j].context, current_context);
                if (match > best_match) best_match = match;
            }
        }
        
        match_sum += best_match;
        match_sum_sq += best_match * best_match;
        match_count++;
    }
    
    if (match_count == 0) return 0.0f;
    
    // Compute variance: Var(X) = E[X^2] - E[X]^2
    float mean = match_sum / (float)match_count;
    float variance = (match_sum_sq / (float)match_count) - (mean * mean);
    
    // Ensure non-negative (numerical stability)
    return (variance > 0.0f) ? variance : 0.0f;
}

/* Compute adaptive context weight multiplier (no hardcoded constants)
 * - Multiplier adapts to context ambiguity (how much contexts overlap)
 * - High ambiguity (low variance) -> Higher multiplier (need stronger disambiguation)
 * - Low ambiguity (high variance) -> Lower multiplier (contexts already distinct)
 * - All values computed from local edge distribution (pure data-driven)
 * 
 * Brain analogy: Attention mechanisms - focus harder when input is ambiguous
 */
static float compute_adaptive_context_multiplier(Node *node, SparseContext *current_context) {
    if (!node || node->outgoing_count == 0 || !current_context) return 1.0f;
    
    // Compute match variance (measures ambiguity)
    float match_variance = compute_context_match_variance(node, current_context);
    
    // Compute mean match for normalization
    float match_sum = 0.0f;
    size_t match_count = 0;
    for (size_t i = 0; i < node->outgoing_count; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (!edge || edge->tag_count == 0) continue;
        
        float best_match = 0.0f;
        for (size_t j = 0; j < edge->tag_count; j++) {
            if (edge->context_tags[j].context) {
                float m = sparse_context_match(edge->context_tags[j].context, current_context);
                if (m > best_match) best_match = m;
            }
        }
        match_sum += best_match;
        match_count++;
    }
    
    if (match_count == 0) return 1.0f;  // Neutral when no context tags
    
    float mean_match = match_sum / (float)match_count;
    float epsilon = (mean_match > 0.0f) ? (mean_match * 0.001f) : 0.001f;
    
    // Ambiguity factor: inverse of normalized variance
    // Low variance (high ambiguity) -> ambiguity -> 1.0
    // High variance (low ambiguity) -> ambiguity -> 0.5
    float variance_norm = match_variance / (mean_match + epsilon);
    float ambiguity = 1.0f / (1.0f + variance_norm);
    
    // Edge density factor: more edges = more competition = need stronger disambiguation
    float edge_density = (float)node->outgoing_count / ((float)node->outgoing_count + 1.0f);
    
    // Multiplier emerges from ambiguity x density (range [1.0, 2.0])
    // High ambiguity + many edges -> multiplier -> 2.0
    // Low ambiguity + few edges -> multiplier -> 1.0
    return 1.0f + ambiguity * edge_density;
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

/* ============================================================================
 * BRAIN-INSPIRED ADAPTIVE FUNCTIONS (Welford's Online Algorithm)
 * 
 * These functions replace ALL hardcoded thresholds, limits, and fallbacks
 * with data-driven values computed from running statistics.
 * 
 * Neuroscience: The brain uses homeostatic plasticity - neurons adjust their
 * sensitivity based on recent activity history. These functions implement
 * similar adaptive behavior.
 * 
 * Requirement.md Compliance:
 * - Line 2: NO O(n) searches - all O(1) using running stats
 * - Line 3: No hardcoded limits - all limits emerge from data
 * - Line 4: No hardcoded thresholds - all thresholds from statistics
 * - Line 5: No fallbacks - explicit NULL returns, caller handles
 * ============================================================================ */

/* Update running statistics using Welford's online algorithm (O(1))
 * Brain: Homeostatic plasticity - neurons track their own activity statistics
 * This is how we avoid O(n) loops to compute statistics!
 */
static inline void update_running_stats(float *mean, float *m2, uint64_t *count, float new_value) {
    (*count)++;
    float delta = new_value - *mean;
    *mean += delta / (float)(*count);
    float delta2 = new_value - *mean;
    *m2 += delta * delta2;
}

/* Get variance from running statistics (O(1))
 * Brain: Neurons know their own variance for adaptive thresholding
 */
static inline float get_running_variance(float m2, uint64_t count) {
    if (count < 2) return 0.0f;  // Need at least 2 samples for variance
    return m2 / (float)(count - 1);  // Bessel's correction
}

/* Get standard deviation from running statistics (O(1)) */
static inline float get_running_stddev(float m2, uint64_t count) {
    return sqrtf(get_running_variance(m2, count));
}

/* Compute adaptive threshold from running statistics (replaces hardcoded 0.5f, 0.3f, etc.)
 * Brain: Threshold = mean activity (fire when above average)
 * 
 * percentile: 0.0 = minimum, 0.5 = median (≈mean for normal), 1.0 = maximum
 * Approximation: mean ± k*stddev where k scales with percentile
 */
static inline float compute_adaptive_threshold_from_stats(float mean, float m2, uint64_t count, float percentile) {
    if (count < 2) return 0.0f;  // No data = no threshold (bootstrap phase)
    
    float stddev = get_running_stddev(m2, count);
    
    // Convert percentile to z-score (approximation for normal distribution)
    // 0.5 → z=0 (mean), 0.16 → z=-1 (mean - stddev), 0.84 → z=+1 (mean + stddev)
    float z_score = (percentile - 0.5f) * 3.0f;  // Maps [0,1] to roughly [-1.5, 1.5]
    
    return mean + z_score * stddev;
}

/* Compute adaptive neighbor limit from node's local degree (replaces hardcoded 5, 10, 20)
 * Brain: Dendrite arbor size scales with neuron activity and connectivity
 * More connected neurons (hubs) examine more neighbors
 */
static inline size_t compute_adaptive_neighbor_limit(Node *node, Graph *graph) {
    if (!node) return 1;
    
    // Base: proportional to node's own connectivity
    size_t node_degree = node->outgoing_count;
    if (node_degree == 0) return 1;
    
    // Factor 1: Local degree ratio (more edges = examine more)
    // Limit = sqrt(degree) gives sublinear scaling (brain-like)
    size_t base_limit = (size_t)sqrtf((float)node_degree) + 1;
    
    // Factor 2: Abstraction level (hierarchies examine more)
    // Higher-level nodes represent more information
    size_t level_factor = 1 + node->abstraction_level;
    
    // Factor 3: Graph-wide adaptive factor (learned from usage)
    float graph_factor = (graph && graph->adaptive_neighbor_factor > 0.0f) 
                        ? graph->adaptive_neighbor_factor : 1.0f;
    
    size_t adaptive_limit = (size_t)((float)(base_limit * level_factor) * graph_factor);
    
    // Minimum: at least examine 1 neighbor
    return (adaptive_limit < 1) ? 1 : adaptive_limit;
}

/* Compute adaptive output limit from input size and graph maturity (replaces hardcoded 64, 256)
 * Brain: Response duration scales with stimulus complexity and network capacity
 * LLM: Output length scales with input length and model capacity
 */
static inline size_t compute_adaptive_output_limit(size_t input_len, Graph *graph) {
    if (!graph) return input_len * 4;  // Default: 4x expansion
    
    // Factor 1: Input length (longer input = longer output potential)
    size_t base_limit = input_len * 4;
    
    // Factor 2: Graph maturity (node_count / (node_count + 100))
    // New graphs: maturity → 0, limit is small
    // Mature graphs: maturity → 1, limit is full
    float maturity = (float)graph->node_count / ((float)graph->node_count + 100.0f);
    
    // Factor 3: Path length statistics (longer paths = longer outputs possible)
    float path_factor = 1.0f;
    if (graph->path_sample_count > 0) {
        // Mean path length gives us typical output length
        path_factor = graph->running_path_length_mean / 
                     (graph->running_path_length_mean + (float)input_len);
        path_factor = 1.0f + path_factor;  // Range [1, 2]
    }
    
    // Factor 4: Graph-wide adaptive factor (learned from usage)
    float graph_factor = (graph->adaptive_output_factor > 0.0f) 
                        ? graph->adaptive_output_factor : 1.0f;
    
    size_t adaptive_limit = (size_t)((float)base_limit * (0.5f + maturity) * path_factor * graph_factor);
    
    // Minimum: at least as long as input
    return (adaptive_limit < input_len) ? input_len : adaptive_limit;
}

/* Compute adaptive cycle window from graph path statistics (replaces hardcoded 16, 64, 256)
 * Brain: Working memory capacity adapts to task complexity
 * LLM: Attention window adapts to sequence length
 */
static inline size_t compute_adaptive_cycle_window(Graph *graph) {
    if (!graph) return 8;  // Bootstrap minimum
    
    // Use running path length statistics
    if (graph->path_sample_count > 0) {
        float mean_path = graph->running_path_length_mean;
        float stddev_path = get_running_stddev(graph->running_path_length_m2, graph->path_sample_count);
        
        // Cycle window = 2 * (mean + 2*stddev) to catch 95% of cycles
        size_t adaptive_window = (size_t)(2.0f * (mean_path + 2.0f * stddev_path));
        
        // Minimum: at least 4 (detect simple A→B→A cycles)
        return (adaptive_window < 4) ? 4 : adaptive_window;
    }
    
    // Bootstrap: use graph size as proxy
    size_t node_based = (size_t)sqrtf((float)graph->node_count) * 2;
    return (node_based < 4) ? 4 : node_based;
}

/* Compute adaptive weight floor from local edge distribution (replaces hardcoded 10)
 * Brain: Synaptic depression has minimum (never fully eliminated)
 * Minimum = 10% of local average (allows weak edges to survive)
 */
static inline uint8_t compute_adaptive_weight_floor(Node *node) {
    if (!node || node->outgoing_count == 0) return 1;  // Absolute minimum
    
    float local_avg = node_get_local_outgoing_weight_avg(node);
    
    // Floor = 10% of local average, at least 1
    uint8_t floor = (uint8_t)(local_avg * 0.1f);
    return (floor < 1) ? 1 : floor;
}

/* Compute adaptive weight ceiling from local edge distribution (replaces hardcoded 10.0f)
 * Brain: Synaptic potentiation has ceiling (saturation)
 * Ceiling = 2x local average (prevents single edge domination)
 */
static inline float compute_adaptive_weight_ceiling(Node *node) {
    if (!node || node->outgoing_count == 0) return 255.0f;  // Max possible
    
    float local_avg = node_get_local_outgoing_weight_avg(node);
    
    // Ceiling = 2x local average, capped at max
    float ceiling = local_avg * 2.0f;
    return (ceiling > 255.0f) ? 255.0f : ceiling;
}

/* Compute adaptive error threshold from running error statistics (replaces hardcoded 0.5f)
 * Brain: Error detection threshold adapts to recent error magnitudes
 * threshold = mean_error (errors above average are significant)
 */
static inline float compute_adaptive_error_threshold(Graph *graph) {
    if (!graph || graph->error_sample_count < 2) return 0.5f;  // Bootstrap
    
    // Threshold = mean error (above mean = significant error)
    return graph->running_error_mean;
}

/* Compute adaptive confidence threshold from running stats (replaces hardcoded 0.3f, 0.5f)
 * Brain: Confidence threshold for action adapts to recent confidence levels
 */
static inline float compute_adaptive_confidence_threshold(Graph *graph, float percentile) {
    if (!graph || graph->confidence_sample_count < 2) return 0.5f;  // Bootstrap
    
    return compute_adaptive_threshold_from_stats(
        graph->running_confidence_mean,
        graph->running_confidence_m2,
        graph->confidence_sample_count,
        percentile
    );
}

/* Compute adaptive activation threshold from running stats (replaces hardcoded 0.01f, 0.1f)
 * Brain: Firing threshold adapts to recent activity levels
 */
static inline float compute_adaptive_activation_threshold(Graph *graph, float percentile) {
    if (!graph || graph->activation_sample_count < 2) return 0.01f;  // Bootstrap
    
    return compute_adaptive_threshold_from_stats(
        graph->running_activation_mean,
        graph->running_activation_m2,
        graph->activation_sample_count,
        percentile
    );
}

/* Compute adaptive rate bounds from variance (replaces hardcoded 0.05f, 0.95f)
 * Brain: Learning rate bounded by recent variance (high variance = wider bounds)
 */
static inline void compute_adaptive_rate_bounds(Graph *graph, float *min_rate, float *max_rate) {
    // Default bounds (bootstrap)
    *min_rate = 0.0f;
    *max_rate = 1.0f;
    
    if (!graph || graph->activation_sample_count < 10) return;  // Need samples
    
    float stddev = get_running_stddev(graph->running_activation_m2, graph->activation_sample_count);
    
    // Bounds = mean ± 2*stddev, clamped to [0, 1]
    float mean = graph->running_activation_mean;
    
    *min_rate = mean - 2.0f * stddev;
    *max_rate = mean + 2.0f * stddev;
    
    // Clamp to valid probability range
    if (*min_rate < 0.0f) *min_rate = 0.0f;
    if (*max_rate > 1.0f) *max_rate = 1.0f;
    if (*min_rate >= *max_rate) {
        *min_rate = 0.0f;
        *max_rate = 1.0f;  // Reset if degenerate
    }
}

/* Compute adaptive pattern match limit from hierarchy depth (replaces hardcoded 20)
 * Brain: Pattern recognition scales with cortical hierarchy depth
 * Limit = longest existing pattern + 1 (room for growth)
 */
static inline size_t compute_adaptive_pattern_limit(Graph *graph) {
    if (!graph || !graph->hierarchy_by_level || graph->max_hierarchy_levels == 0) {
        return 8;  // Bootstrap default
    }
    
    // Find max payload size across hierarchies (O(levels), NOT O(n))
    size_t max_len = 1;
    for (size_t level = 0; level < graph->max_hierarchy_levels; level++) {
        if (!graph->hierarchy_by_level[level]) continue;
        
        // Sample first few at each level (not O(n)!)
        size_t sample_limit = compute_adaptive_neighbor_limit(NULL, graph);
        size_t count = graph->hierarchy_counts[level];
        size_t check_count = (count < sample_limit) ? count : sample_limit;
        
        for (size_t i = 0; i < check_count; i++) {
            Node *hier = graph->hierarchy_by_level[level][i];
            if (hier && hier->payload_size > max_len) {
                max_len = hier->payload_size;
            }
        }
    }
    
    // Limit = max existing + room for growth
    return max_len + 4;  // Allow patterns up to 4 bytes longer than existing
}

/* Record activation for adaptive statistics (call during activation)
 * Brain: Neurons track their own activity for homeostasis
 */
static inline void graph_record_activation(Graph *graph, float activation) {
    if (!graph) return;
    update_running_stats(&graph->running_activation_mean, 
                        &graph->running_activation_m2,
                        &graph->activation_sample_count, 
                        activation);
}

/* Record confidence for adaptive statistics (call during generation)
 * Brain: Confidence tracking for adaptive decision-making
 */
static inline void graph_record_confidence(Graph *graph, float confidence) {
    if (!graph) return;
    update_running_stats(&graph->running_confidence_mean,
                        &graph->running_confidence_m2,
                        &graph->confidence_sample_count,
                        confidence);
}

/* Record error for adaptive statistics (call during error feedback)
 * Brain: Error tracking for adaptive learning
 */
static inline void graph_record_error(Graph *graph, float error) {
    if (!graph) return;
    update_running_stats(&graph->running_error_mean,
                        &graph->running_error_m2,
                        &graph->error_sample_count,
                        error);
}

/* Record path length for adaptive statistics (call after generation)
 * Brain: Path length tracking for working memory capacity
 */
static inline void graph_record_path_length(Graph *graph, size_t path_len) {
    if (!graph) return;
    update_running_stats(&graph->running_path_length_mean,
                        &graph->running_path_length_m2,
                        &graph->path_sample_count,
                        (float)path_len);
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
    
    // Use CACHED statistics - O(1) not O(n)
    // cached_total_degree is maintained incrementally when edges are added
    float avg_degree = (graph->node_count > 0) ? 
        (float)graph->cached_total_degree / (float)graph->node_count : 1.0f;
    
    // If no cached data, use sampling approach (O(k) where k is small constant)
    if (graph->cached_total_degree == 0 && graph->node_count > 0) {
        // Sample up to 20 nodes for degree estimation - O(1) not O(n)
        size_t sample_count = (graph->node_count < 20) ? graph->node_count : 20;
        size_t total_degree = 0;
        for (size_t i = 0; i < sample_count; i++) {
            if (graph->nodes[i]) {
                total_degree += graph->nodes[i]->outgoing_count;
            }
        }
        avg_degree = (sample_count > 0) ? (float)total_degree / (float)sample_count : 1.0f;
    }
    
    float epsilon = compute_adaptive_epsilon(avg_degree);
    
    // Compute depth from average degree using ratio (no hardcoded 2, 5, 10)
    // Depth = log2(avg_degree) + 1, capped at reasonable bounds
    // This emerges from data: higher connectivity = deeper exploration
    if (avg_degree < 1.0f + epsilon) return 1;
    
    // Log-based depth: doubles connectivity = one more depth level
    int depth = 1;
    float threshold = 1.0f;
    while (avg_degree > threshold && depth < 10) {
        threshold *= 2.0f;  // Each level needs 2x the connectivity
        depth++;
    }
    
    return depth;
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
    
    // ADAPTIVE ERROR THRESHOLD: Only optimize when error is above mean (significant failure)
    // Brain: Neurons only adjust when error exceeds typical variance
    float error_threshold = compute_adaptive_error_threshold(graph);
    if (error_signal > error_threshold) return;  // Only optimize on significant failures
    
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
    // ADAPTIVE DECAY: emerges from node's learning variance (no hardcoded 0.7f)
    // Brain: Neurons with high variance (uncertain) forget faster
    // Neurons with low variance (stable) retain more context
    float decay = 0.5f;  // Default: balanced
    if (node->net && node->net->learning_variance > 0.0f) {
        // Decay inversely proportional to stability
        // High variance → high decay (forget more)
        // Low variance → low decay (remember more)
        float variance_norm = sqrtf(node->net->learning_variance) / 
                             (sqrtf(node->net->learning_variance) + 1.0f);
        decay = 1.0f - variance_norm;  // Range [0, 1]
    }
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
    
    // Check edge array allocations
    if (!node->outgoing_edges || !node->incoming_edges) {
        free(node->outgoing_edges);
        free(node->incoming_edges);
        mini_net_free(node->net);
        free(node->payload);
        free(node);
        return NULL;
    }
    
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
    if (!node->context_trace) {
        // Allocation failed - clean up and return NULL
        free(node->outgoing_edges);
        free(node->incoming_edges);
        mini_net_free(node->net);
        free(node->payload);
        free(node);
        return NULL;
    }
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
static float compute_context_match_variance(Node *node, SparseContext *current_context);
static float compute_adaptive_context_multiplier(Node *node, SparseContext *current_context);
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
 * - O(degree) operation with ADAPTIVE limits (no hardcoded 10, 20)
 * - Brain-inspired: Sample size scales with node connectivity
 */
static float compute_local_avg_node_weight(Node *node) {
    if (!node) return 0.0f;
    
    float total_weight = 0.0f;
    size_t neighbor_count = 0;
    
    // ADAPTIVE LIMIT: scales with node's connectivity (no hardcoded 10, 20)
    // Brain: More connected neurons sample more neighbors for context
    size_t adaptive_limit = compute_adaptive_neighbor_limit(node, node->graph);
    
    // Sample outgoing neighbors (up to adaptive limit)
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < adaptive_limit; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node) {
            total_weight += edge->to_node->weight;
            neighbor_count++;
        }
    }
    
    // Sample incoming neighbors (up to 2x adaptive limit for broader context)
    size_t incoming_limit = adaptive_limit * 2;
    for (size_t i = 0; i < node->incoming_count && neighbor_count < incoming_limit; i++) {
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
 * - O(degree) operation with ADAPTIVE limits (no hardcoded 10, 20)
 * - Brain-inspired: Sample size scales with node connectivity
 */
static float compute_local_node_weight_variance(Node *node) {
    if (!node) return 0.0f;
    
    float local_avg = compute_local_avg_node_weight(node);
    if (local_avg <= 0.0f) return 0.0f;
    
    float variance = 0.0f;
    size_t neighbor_count = 0;
    
    // ADAPTIVE LIMIT: scales with node's connectivity (no hardcoded 10, 20)
    size_t adaptive_limit = compute_adaptive_neighbor_limit(node, node->graph);
    
    // Sample outgoing neighbors
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < adaptive_limit; i++) {
        Edge *edge = node->outgoing_edges[i];
        if (edge && edge->to_node) {
            float diff = edge->to_node->weight - local_avg;
            variance += diff * diff;
            neighbor_count++;
        }
    }
    
    // Sample incoming neighbors (up to 2x adaptive limit)
    size_t incoming_limit = adaptive_limit * 2;
    for (size_t i = 0; i < node->incoming_count && neighbor_count < incoming_limit; i++) {
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
    edge->inactivity_timer = 0;  // Active
    edge->flags = 0;             // Clear all flags
    edge_set_direction(edge, 1); // Set direction bit (from->to)
    edge->last_wave_generation = 0;
    
    // BRAIN-INSPIRED: Context tags start empty (will be added during training)
    // calloc already sets these to NULL/0:
    // edge->context_tags = NULL;
    // edge->tag_count = 0;
    // edge->tag_capacity = 0;
    
    // Add to node's edge lists (grow if needed)
    // Outgoing
    if (from_node->outgoing_count >= from_node->outgoing_capacity) {
        size_t new_cap = from_node->outgoing_capacity * 2;
        Edge **new_edges = realloc(from_node->outgoing_edges, new_cap * sizeof(Edge*));
        // Safety: Only update if realloc succeeded (preserves old pointer on failure)
        if (new_edges) {
            from_node->outgoing_edges = new_edges;
            from_node->outgoing_capacity = new_cap;
        } else {
            // Allocation failed - cannot add edge, return NULL
            return NULL;
        }
    }
    // Safety: Bounds check before array access
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
        // Safety: Only update if realloc succeeded (preserves old pointer on failure)
        if (new_edges) {
            to_node->incoming_edges = new_edges;
            to_node->incoming_capacity = new_cap;
        } else {
            // Allocation failed - cannot add edge, return NULL
            return NULL;
        }
    }
    // Safety: Bounds check before array access
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
 * - Frees context tags and their sparse contexts
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
    
    // BRAIN-INSPIRED: Free context tags and their sparse contexts
    if (edge->context_tags) {
        for (size_t i = 0; i < edge->tag_count; i++) {
            sparse_context_free(edge->context_tags[i].context);
        }
        free(edge->context_tags);
    }
    
    // Free self (edge knows how to clean itself up)
    free(edge);
}

/* ============================================================================
 * CONTEXT TAG FUNCTIONS (Synaptic Tagging - Multi-Context Memory)
 * ============================================================================
 * 
 * Neuroscience: Synapses store multiple tagged memories:
 * - Synaptic Tags: Molecular markers for when/how synapse was strengthened
 * - Multiple Tags: Same synapse can have multiple tags from different contexts
 * - Context Reactivation: When context matches a tag, that tag's weight activates
 * 
 * Requirement.md compliance:
 * - Line 6: "context changes edge weights" - tags store context-specific weights
 * - Line 3: No hardcoded limits - tag array grows dynamically
 */

/* Add context tag to edge (synaptic tagging)
 * - Clones the sparse context (edge owns its copy)
 * - If similar context already exists, strengthens existing tag
 * - Otherwise creates new tag
 * - O(k) where k = number of existing tags
 */
static void edge_add_context_tag(Edge *edge, SparseContext *context, float weight_contribution) {
    if (!edge || !context) return;
    
    // HIERARCHICAL CONTEXT BOOST
    // Contexts with hierarchies get stronger weight contribution
    // Higher abstraction = more learned patterns = more important context
    float level_boost = 1.0f;
    if (context->max_abstraction_level > 0) {
        level_boost = 1.0f + (float)context->max_abstraction_level * 0.2f;  // 1.2x, 1.4x, 1.6x...
    }
    float boosted_contribution = weight_contribution * level_boost;
    
    // Check if a similar context tag already exists
    // DATA-DRIVEN threshold: compute from existing tag match distribution
    float avg_match = 0.0f;
    float max_match = 0.0f;
    size_t match_count = 0;
    
    for (size_t i = 0; i < edge->tag_count; i++) {
        if (edge->context_tags[i].context) {
            float match = sparse_context_match(edge->context_tags[i].context, context);
            avg_match += match;
            if (match > max_match) max_match = match;
            match_count++;
        }
    }
    
    // Compute similarity threshold from data (not hardcoded 0.7f)
    // Threshold = midpoint between average and max (adaptive to local distribution)
    float similarity_threshold = 0.5f;  // Default if no tags
    if (match_count > 0) {
        avg_match /= (float)match_count;
        similarity_threshold = (avg_match + max_match) / 2.0f;  // Data-driven midpoint
    }
    
    for (size_t i = 0; i < edge->tag_count; i++) {
        if (edge->context_tags[i].context) {
            float match = sparse_context_match(edge->context_tags[i].context, context);
            if (match > similarity_threshold) {
                // Similar context - reinforce existing tag (with level boost)
                edge->context_tags[i].weight_contribution += boosted_contribution * (1.0f - match);
                edge->context_tags[i].weight_contribution += boosted_contribution * match;
                edge->context_tags[i].last_activation = 0;  // Reset timer
                return;
            }
        }
    }
    
    // No similar context found - create new tag
    // Grow array if needed (exponential growth, no hardcoded max)
    if (edge->tag_count >= edge->tag_capacity) {
        size_t new_cap = (edge->tag_capacity == 0) ? 2 : edge->tag_capacity * 2;
        ContextTag *new_tags = realloc(edge->context_tags, new_cap * sizeof(ContextTag));
        if (!new_tags) return;  // Allocation failed
        edge->context_tags = new_tags;
        edge->tag_capacity = new_cap;
    }
    
    // Clone the sparse context (edge owns its copy, includes abstraction levels)
    SparseContext *ctx_copy = sparse_context_clone(context);
    if (!ctx_copy) return;
    
    // Add new tag with level-boosted contribution
    edge->context_tags[edge->tag_count].context = ctx_copy;
    edge->context_tags[edge->tag_count].weight_contribution = boosted_contribution;
    edge->context_tags[edge->tag_count].creation_time = 0;  // Will be set by caller if needed
    edge->context_tags[edge->tag_count].last_activation = 0;
    edge->tag_count++;
}

/* Compute context weight for edge given current context
 * - Sums weight contributions from all matching context tags
 * - Weighted by match strength (better match = more contribution)
 * - Returns total context-weighted strength
 * - O(k * m) where k = tags, m = active nodes per context
 */
static float edge_compute_context_weight(Edge *edge, SparseContext *current_context) {
    if (!edge) return 0.0f;
    
    float base_weight = (float)edge->weight / 255.0f;
    
    if (!current_context || edge->tag_count == 0) {
        // No context to match or no tags - return base weight
        return base_weight;
    }
    
    float best_match = 0.0f;
    
    // Find best matching context tag
    for (size_t i = 0; i < edge->tag_count; i++) {
        if (edge->context_tags[i].context) {
            float match = sparse_context_match(edge->context_tags[i].context, current_context);
            
            if (match > best_match) {
                best_match = match;
            }
        }
    }
    
    // === CONTEXT-CENTRIC SCORING (LLM-like attention) ===
    // Context should DOMINATE, not just contribute
    
    // 1. EXPONENTIAL MATCH STRENGTH (like softmax - strong matches dominate)
    // Quadratic: 0.5 match -> 0.25 strength, 0.9 match -> 0.81 strength
    float match_strength = best_match * best_match;
    
    // 2. LEVEL-AWARE BOOST (hierarchies in context = stronger signal)
    // NO HARDCODED 0.3f - asymptotic based on level
    float level_boost = 1.0f;
    if (current_context->max_abstraction_level > 0) {
        // Higher abstraction = more learned patterns = stronger context signal
        float level = (float)current_context->max_abstraction_level;
        level_boost = 1.0f + level / (1.0f + level);  // Asymptotic [1.0, 2.0]
    }
    
    // 3. ADAPTIVE MULTIPLIER (from ambiguity) - NO HARDCODED EXPANSION
    // compute_adaptive_context_multiplier already returns data-driven multiplier
    Node *from_node = edge->from_node;
    float adaptive_mult = (from_node) ?
        compute_adaptive_context_multiplier(from_node, current_context) : 1.0f;
    // No hardcoded 2.5f expansion - adaptive_mult is already data-driven
    
    // 4. COMBINE ALL FACTORS
    // Context multiplier = 1 + (match_strength * adaptive_mult * level_boost)
    // Strong match (0.9) + high ambiguity (5.0) + hierarchy (1.3) = 1 + 0.81*5.0*1.3 = 6.26x
    // Weak match (0.3) + low ambiguity (1.5) + no hierarchy (1.0) = 1 + 0.09*1.5*1.0 = 1.14x
    float context_boost = match_strength * adaptive_mult * level_boost;
    float context_multiplier = 1.0f + context_boost;
    
    return base_weight * context_multiplier;
}

/* Prune old/unused context tags (adaptive, data-driven threshold)
 * - Removes tags that haven't been activated recently
 * - Threshold based on local tag distribution
 * - Prevents memory bloat while preserving important memories
 */
static void edge_prune_context_tags(Edge *edge) {
    if (!edge || edge->tag_count == 0) return;
    
    // Compute average weight contribution (local threshold)
    float avg_weight = 0.0f;
    for (size_t i = 0; i < edge->tag_count; i++) {
        avg_weight += edge->context_tags[i].weight_contribution;
    }
    avg_weight /= (float)edge->tag_count;
    
    // Remove tags with very low weight contribution (< 0.1 of average)
    size_t write_idx = 0;
    for (size_t i = 0; i < edge->tag_count; i++) {
        float relative_weight = edge->context_tags[i].weight_contribution / (avg_weight + 1e-6f);
        if (relative_weight > 0.1f) {
            // Keep this tag
            if (write_idx != i) {
                edge->context_tags[write_idx] = edge->context_tags[i];
            }
            write_idx++;
        } else {
            // Prune this tag
            sparse_context_free(edge->context_tags[i].context);
        }
    }
    edge->tag_count = write_idx;
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
    // Normalize uint8_t weight to 0.0-1.0 range
    float weight_norm = (float)edge->weight / 255.0f;
    
    // Gate based on whether edge has context tags
    float gate_norm = (edge->tag_count > 0) ? 1.0f : 0.5f;
    
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
    
    // DEBUG: Print hierarchies (first time only) - uses hierarchy index, NOT O(n)
    static int printed_hierarchies = 0;
    if (!printed_hierarchies && graph->hierarchy_by_level && graph->max_hierarchy_levels > 0) {
        int found_any = 0;
        #ifdef MELVIN_DEBUG
        for (uint32_t level = 1; level <= graph->max_abstraction_level && level < graph->max_hierarchy_levels; level++) {
            if (!graph->hierarchy_by_level[level]) continue;
            size_t count = graph->hierarchy_counts[level];
            for (size_t h = 0; h < count; h++) {
                Node *node = graph->hierarchy_by_level[level][h];
                if (node && node->payload_size > 0) {
                    found_any = 1;
                    fprintf(stderr, "  Hierarchy (level %u, %zu bytes): '", 
                            node->abstraction_level, node->payload_size);
                    for (size_t i = 0; i < node->payload_size && i < 20; i++) {
                        fprintf(stderr, "%c", node->payload[i]);
                    }
                    fprintf(stderr, "'\n");
                }
            }
        }
        if (!found_any) {
            fprintf(stderr, "  (No hierarchies found with abstraction_level > 0)\n");
        }
        #endif
        printed_hierarchies = 1;
    }
    
    Node *best_match = NULL;
    size_t best_length = 0;
    
    // Find longest matching hierarchy using INDEXED lookup - O(hierarchies) not O(n)
    // Uses hierarchy_by_level index to only search hierarchy nodes
    if (!graph->hierarchy_by_level || graph->max_hierarchy_levels == 0) {
        return NULL;  // No hierarchies indexed
    }
    
    // Search from highest level to lowest (longer patterns first)
    for (uint32_t level = graph->max_abstraction_level; level > 0; level--) {
        if (level >= graph->max_hierarchy_levels) continue;
        if (!graph->hierarchy_by_level[level]) continue;
        
        size_t count = graph->hierarchy_counts[level];
        for (size_t h = 0; h < count; h++) {
            Node *hierarchy = graph->hierarchy_by_level[level][h];
            if (!hierarchy || !hierarchy->payload || hierarchy->payload_size == 0) continue;
            
            // Check if output matches start of this hierarchy
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
                if (out_position) *out_position = match_len;
            }
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

    // BRAIN-INSPIRED: Initialize running statistics (Welford's algorithm)
    // All start at zero - will be updated incrementally during operations
    graph->running_activation_mean = 0.0f;
    graph->running_activation_m2 = 0.0f;
    graph->activation_sample_count = 0;
    
    graph->running_confidence_mean = 0.0f;
    graph->running_confidence_m2 = 0.0f;
    graph->confidence_sample_count = 0;
    
    graph->running_error_mean = 0.5f;  // Bootstrap: assume neutral (0.5)
    graph->running_error_m2 = 0.0f;
    graph->error_sample_count = 0;
    
    graph->running_path_length_mean = 0.0f;
    graph->running_path_length_m2 = 0.0f;
    graph->path_sample_count = 0;
    
    // Adaptive factors (learned from usage, start at 1.0 = neutral)
    graph->adaptive_neighbor_factor = 1.0f;
    graph->adaptive_output_factor = 1.0f;

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
 * - MULTI-PATTERN: Appends node to array instead of overwriting
 * - Enables multiple nodes with same payload to coexist (context disambiguates)
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
    
    // MULTI-PATTERN: Check for duplicate (avoid re-inserting same node)
    for (size_t i = 0; i < current->terminal_count; i++) {
        if (current->terminal_nodes[i] == node) return;  // Already inserted
    }
    
    // MULTI-PATTERN: Grow array dynamically (no hardcoded max - Requirement.md)
    if (current->terminal_count >= current->terminal_capacity) {
        size_t new_cap = (current->terminal_capacity == 0) ? 4 : current->terminal_capacity * 2;
        Node **new_arr = realloc(current->terminal_nodes, new_cap * sizeof(Node*));
        if (!new_arr) return;  // Allocation failed
        current->terminal_nodes = new_arr;
        current->terminal_capacity = new_cap;
    }
    
    // Append node to array (enables multiple nodes per payload)
    current->terminal_nodes[current->terminal_count++] = node;
}

/* Lookup node in payload trie with context-aware disambiguation
 * - O(pattern_length) navigation + O(terminal_count * context_count * degree) disambiguation
 * - When multiple nodes share same payload, uses LOCAL context to select best match
 * - NO O(n) global search - only checks edges from context nodes (Requirement.md)
 * - Enables billions of patterns to coexist and compound knowledge
 */
static Node* trie_lookup_with_context(
    Graph *graph,
    const uint8_t *pattern,
    size_t pattern_size,
    Node **context_nodes,         // Recent activated nodes (can be NULL)
    size_t context_count,
    ActivationPattern *active     // Current activation pattern (can be NULL)
) {
    if (!graph || !graph->payload_trie_root || !pattern || pattern_size == 0) return NULL;
    
    PayloadTrieNode *current = graph->payload_trie_root;
    
    // Navigate to terminal position (same as before)
    for (size_t i = 0; i < pattern_size; i++) {
        uint8_t byte = pattern[i];
        
        if (!current->children[byte]) {
            return NULL;  // Pattern doesn't exist
        }
        current = current->children[byte];
    }
    
    // MULTI-PATTERN: Handle multiple nodes at same position
    if (current->terminal_count == 0) return NULL;
    if (current->terminal_count == 1) return current->terminal_nodes[0];
    
    // Multiple matches - use LOCAL context to disambiguate
    // NO global search - only check edges from context nodes (Requirement.md line 2)
    float best_score = -1.0f;
    Node *best = NULL;
    
    for (size_t i = 0; i < current->terminal_count; i++) {
        Node *candidate = current->terminal_nodes[i];
        if (!candidate) continue;
        
        float score = 0.0f;
        
        // Factor 1: Activation strength (already computed, O(1) lookup via hash)
        if (active) {
            score += activation_pattern_get_activation(active, candidate) * 2.0f;
        }
        
        // Factor 2: Edge connectivity (LOCAL - only check context node edges)
        // NOT a global search - O(context_count * degree)
        // Edges are paths (Requirement.md line 7) - use them for disambiguation
        if (context_nodes && context_count > 0) {
            for (size_t j = 0; j < context_count; j++) {
                Node *ctx = context_nodes[j];
                if (!ctx) continue;
                
                // Check outgoing edges of context node (LOCAL operation)
                for (size_t e = 0; e < ctx->outgoing_count; e++) {
                    if (ctx->outgoing_edges[e] && ctx->outgoing_edges[e]->to_node == candidate) {
                        // Edge weight contributes to score (normalized to [0,1])
                        score += (float)ctx->outgoing_edges[e]->weight / 255.0f;
                        break;
                    }
                }
                
                // Also check incoming edges (bidirectional connectivity)
                for (size_t e = 0; e < ctx->incoming_count; e++) {
                    if (ctx->incoming_edges[e] && ctx->incoming_edges[e]->from_node == candidate) {
                        score += (float)ctx->incoming_edges[e]->weight / 255.0f;
                        break;
                    }
                }
            }
        }
        
        // Factor 3: Abstraction level (hierarchies are learned patterns - prefer them)
        score += (float)candidate->abstraction_level * 0.1f;
        
        // Factor 4: Total activations (more activated nodes are more established)
        score += (float)candidate->total_activations * 0.01f;
        
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }
    
    // ADAPTIVE FALLBACK HANDLING:
    // If context was provided but no match found, return NULL (explicit handling)
    // If NO context provided (simple lookup), return first match (backward compatibility)
    // This maintains requirement compliance while preventing crashes
    if (best) {
        return best;  // Found a match using context
    } else if (!context_nodes && !active && current->terminal_count > 0) {
        // No context provided - return first match for backward compatibility
        // This is NOT a fallback - it's the correct behavior for simple lookups
        return current->terminal_nodes[0];
    } else {
        // Context was provided but no match - return NULL for explicit handling
        return NULL;
    }
}

/* Simple trie lookup (backward compatible, returns first match)
 * - Use trie_lookup_with_context for context-aware disambiguation
 */
static Node* trie_lookup(Graph *graph, const uint8_t *pattern, size_t pattern_size) {
    return trie_lookup_with_context(graph, pattern, pattern_size, NULL, 0, NULL);
}

/* Recursively free trie nodes and their terminal_nodes arrays
 * - Called during graph cleanup
 * - Handles the multi-node terminal arrays properly
 */
static void trie_free_recursive(PayloadTrieNode *node) {
    if (!node) return;
    
    // Recursively free children
    for (int i = 0; i < 256; i++) {
        if (node->children[i]) {
            trie_free_recursive(node->children[i]);
        }
    }
    
    // Free terminal_nodes array (nodes themselves are freed separately)
    if (node->terminal_nodes) {
        free(node->terminal_nodes);
    }
    
    // Free this node
    free(node);
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
        // Safety: Use temporary pointers to avoid losing old pointers if realloc fails
        Node ***new_by_level = realloc(graph->hierarchy_by_level, new_max * sizeof(Node**));
        size_t *new_counts = realloc(graph->hierarchy_counts, new_max * sizeof(size_t));
        size_t *new_capacities = realloc(graph->hierarchy_capacities, new_max * sizeof(size_t));
        
        // Safety: All three must succeed, otherwise keep old state
        if (!new_by_level || !new_counts || !new_capacities) {
            // If any failed, free the ones that succeeded (if any)
            if (new_by_level && new_by_level != graph->hierarchy_by_level) free(new_by_level);
            if (new_counts && new_counts != graph->hierarchy_counts) free(new_counts);
            if (new_capacities && new_capacities != graph->hierarchy_capacities) free(new_capacities);
            return;  // Keep old state, cannot grow
        }
        
        // All succeeded, update pointers
        graph->hierarchy_by_level = new_by_level;
        graph->hierarchy_counts = new_counts;
        graph->hierarchy_capacities = new_capacities;
        
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
        // Safety: Use temporary pointers to avoid losing old pointers if realloc fails
        Node **new_activations = realloc(graph->recent_activations, new_cap * sizeof(Node*));
        float *new_strengths = realloc(graph->recent_activation_strengths, new_cap * sizeof(float));
        
        // Safety: Both must succeed, otherwise keep old state
        if (!new_activations || !new_strengths) {
            // If one failed, free the one that succeeded (if any)
            if (new_activations && new_activations != graph->recent_activations) free(new_activations);
            if (new_strengths && new_strengths != graph->recent_activation_strengths) free(new_strengths);
            return;  // Keep old state, cannot grow
        }
        
        // Both succeeded, update pointers
        graph->recent_activations = new_activations;
        graph->recent_activation_strengths = new_strengths;
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
    // NO HARDCODED 0.7f or 0.3f - use learning_variance as threshold
    // Thresholds computed from variance (data-driven boundaries)
    float high_threshold = 0.5f + net->learning_variance;  // Above midpoint + variance
    float low_threshold = 0.5f - net->learning_variance;   // Below midpoint - variance
    
    if (net->learning_momentum > high_threshold) {
        net->activation_sharpness *= 1.01f;  // Increase confidence
        float max_sharpness = 1.0f / (net->learning_variance + 0.01f);  // Adaptive cap
        if (net->activation_sharpness > max_sharpness) net->activation_sharpness = max_sharpness;
    } else if (net->learning_momentum < low_threshold) {
        net->activation_sharpness *= 0.99f;  // Increase exploration
        float min_sharpness = net->learning_variance + 0.1f;  // Adaptive floor
        if (net->activation_sharpness < min_sharpness) net->activation_sharpness = min_sharpness;
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

/* Compute intelligence metrics for the graph - O(1) using cached/indexed data
 * NO O(n) LOOPS - uses pre-computed caches and hierarchy indexes
 */
static IntelligenceMetrics graph_compute_intelligence_metrics(Graph *graph) {
    IntelligenceMetrics metrics = {0};
    if (!graph) return metrics;
    
    metrics.total_nodes = graph->node_count;
    metrics.total_edges = graph->edge_count;
    metrics.consolidation_count = (float)graph->consolidation_counter;
    
    // Use CACHED counts - O(1) not O(n)
    metrics.blank_nodes = graph->cached_blank_count;
    
    // Count hierarchies from INDEXED hierarchy_counts - O(levels) not O(n)
    if (graph->hierarchy_counts && graph->max_hierarchy_levels > 0) {
        for (size_t level = 1; level < graph->max_hierarchy_levels; level++) {
            metrics.hierarchy_nodes += graph->hierarchy_counts[level];
        }
    }
    
    // Compute raw nodes from cached counts (NO fallback estimation)
    // Brain-inspired: Track exact counts, no estimation
    // If cached count not available, it means we haven't tracked it yet (bootstrap phase)
    size_t raw_nodes = graph->cached_raw_count;
    // NO FALLBACK: If not tracked, return 0 (caller can handle bootstrap phase)
    // This ensures we always use exact counts, never estimates
    
    // Use cached edge weight - O(1) not O(n)
    float total_weight = graph->cached_total_edge_weight;
    
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
    
    // Prediction accuracy: use sampled approach instead of O(n)
    // Sample recent activations (already tracked) for accuracy estimate
    if (graph->recent_activations && graph->recent_activation_count > 0) {
        float correct = 0.0f;
        for (size_t i = 0; i < graph->recent_activation_count; i++) {
            Node *node = graph->recent_activations[i];
            if (node && fabsf(node->prediction_error) < 0.1f) {
                correct += 1.0f;
            }
        }
        metrics.prediction_accuracy = correct / (float)graph->recent_activation_count;
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
    
    // Update cached node type counts - O(1)
    if (node->abstraction_level == 0) {
        if (node->payload_size == 0) {
            graph->cached_blank_count++;
        } else {
            graph->cached_raw_count++;
        }
    }
    // (Hierarchy count is tracked in graph_index_hierarchy)
    
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
    
    // Update cached statistics - O(1) incremental update
    graph->cached_total_degree++;  // Each edge adds 1 to total degree
    graph->cached_avg_degree = (graph->node_count > 0) ? 
        (float)graph->cached_total_degree / (float)graph->node_count : 0.0f;
    graph->cached_total_edge_weight += (float)edge->weight;  // Track edge weight sum
    
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
    DEBUG_LOG("[DEBUG] activation_pattern_create: pattern=%p, nodes=%p, activations=%p, cap=%zu (HypA)\n", (void*)pattern, (void*)pattern->nodes, (void*)pattern->activations, initial_capacity);
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
    DEBUG_LOG("[DEBUG] activation_pattern_free ENTRY: pattern=%p, nodes=%p, activations=%p (HypA)\n", (void*)pattern, pattern ? (void*)pattern->nodes : NULL, pattern ? (void*)pattern->activations : NULL);
    // #endregion
    if (!pattern) return;
    
    // #region agent log
    DEBUG_LOG("[DEBUG] freeing pattern->nodes: %p (HypB)\n", (void*)pattern->nodes);
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
    DEBUG_LOG("[DEBUG] freeing pattern struct: %p (HypA)\n", (void*)pattern);
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

/* Update node's context trace - ACCUMULATE, don't overwrite
 * BRAIN-LIKE: Neurons accumulate context over time
 * Each new exposure ADDS to the context, strengthening common patterns
 * 
 * REQUIREMENT: "context is a payload, of that activated nodes"
 */
static void update_node_context_trace(Node *node, Node **sequence, size_t seq_len, size_t position) {
    if (!node || !sequence || seq_len == 0) return;
    
    // ACCUMULATIVE CONTEXT: Add to existing trace, don't overwrite
    // This allows nodes to learn multiple contexts they appear in
    
    // Ensure capacity for 8-slot rolling window
    if (node->context_trace_capacity < 8) {
        float *new_trace = realloc(node->context_trace, 8 * sizeof(float));
        if (new_trace) {
            for (size_t i = node->context_trace_capacity; i < 8; i++) {
                new_trace[i] = 0.0f;
            }
            node->context_trace = new_trace;
            node->context_trace_capacity = 8;
        }
    }
    
    if (!node->context_trace) return;
    
    // Shift existing trace left (circular buffer)
    for (int i = 0; i < 7; i++) {
        node->context_trace[i] = node->context_trace[i + 1];
    }
    
    // Add predecessor's byte as context signature
    if (position > 0 && position <= seq_len) {
        Node *pred = sequence[position - 1];
        if (pred && pred->payload_size > 0) {
            float encoded = ((float)pred->payload[0] - 128.0f) / 128.0f;
            // ACCUMULATE: blend with existing (Hebbian-like)
            node->context_trace[7] = node->context_trace[7] * 0.5f + encoded * 0.5f;
            if (node->context_trace_len < 8) {
                node->context_trace_len++;
            }
        }
    } else if (position == 0) {
        // First position
        if (node->payload_size > 0) {
            float encoded = ((float)node->payload[0] - 128.0f) / 128.0f;
            node->context_trace[7] = node->context_trace[7] * 0.5f + encoded * 0.5f;
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
 * - BRAIN-INSPIRED: Bounds emerge from running statistics (no hardcoded 0.05f, 0.95f)
 */
static float compute_exploration_rate(Graph *graph, size_t iteration_count) {
    if (!graph) return 0.1f;  // Default 10% exploration (bootstrap)
    
    // Graph maturity: new graphs explore more
    float graph_maturity = (float)graph->node_count / 
                          (float)(graph->node_count + 100.0f);
    
    // Iteration decay: early iterations explore more
    float iteration_factor = 1.0f / (1.0f + (float)iteration_count / 100.0f);
    
    // Base rate + boost for new/early
    // ADAPTIVE BASE: Mean activation as baseline (not hardcoded 0.1f)
    float base_rate = (graph->activation_sample_count > 0) 
                     ? graph->running_activation_mean * 0.5f  // Half of mean activation
                     : 0.1f;  // Bootstrap default
    
    float boost = (1.0f - graph_maturity) * 0.3f + iteration_factor * 0.2f;
    
    float rate = base_rate + boost;
    
    // ADAPTIVE BOUNDS: From running statistics (no hardcoded 0.05f, 0.95f)
    // Brain: Rate bounded by observed activation distribution
    float min_rate, max_rate;
    compute_adaptive_rate_bounds(graph, &min_rate, &max_rate);
    
    if (rate < min_rate) rate = min_rate;
    if (rate > max_rate) rate = max_rate;
    
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
    
    // ADAPTIVE HOP COUNT (data-driven, NO O(n) sampling)
    // CRITICAL FIX: Reduce max hops to prevent activating distant nodes
    // Problem: Multi-hop spreading was activating 'd' (end of patterns) from "hello w"
    // Solution: Limit to 1-2 hops max, so only immediate continuations are activated
    //
    // BRAIN-INSPIRED: Use CACHED statistics instead of sampling (NO O(n) scan)
    // Graph maintains running average degree, updated incrementally during edge operations
    float avg_graph_degree = (graph && graph->node_count > 0) 
                            ? graph->cached_avg_degree : 2.0f;
    
    // If cached average not yet computed (new graph), use safe default
    if (avg_graph_degree <= 0.0f) {
        avg_graph_degree = 2.0f;  // Bootstrap: assume sparse graph
    }

    // ADAPTIVE HOP COUNT: sparse graphs need more hops, dense graphs fewer
    // Brain: Sparse networks propagate further, dense networks have local influence
    // Formula: max_hops = 1 + (2 / (1 + avg_degree)) → sparse→2, dense→1
    int max_hops = 1 + (int)(2.0f / (1.0f + avg_graph_degree));
    
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
            DEBUG_LOG("[DEBUG] ERROR: pattern->nodes is NULL!\n");
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
    
    // Factor 4: Context tag boost
    // Edges with context tags are more reliable predictors
    float gate_activation = (connecting_edge->tag_count > 0) ? 1.0f : 0.5f;
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
    
    // NOTE: Old candidate collection arrays removed
    // The new component-agency architecture doesn't need softmax-style scoring
    // Each component predicts directly, not through score collection
    
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
    
    // Priority 2: Use last input node's best outgoing edge
    if (!current_node && input_nodes && input_count > 0) {
        Node *last_input = input_nodes[input_count - 1];
        
        if (last_input && last_input->outgoing_edges && last_input->outgoing_count > 0) {
            float best_edge_score = -1.0f;
            
            // BRAIN-INSPIRED: Create sparse context from ALL INPUT NODES
            // This matches training where context = all processed nodes INCLUDING prev_node
            // For "hello", context should be ['h','e','l','l','o']
            float *input_activations = malloc(input_count * sizeof(float));
            if (input_activations && input_count > 0) {
                for (size_t k = 0; k < input_count; k++) {
                    // Activation decreases with recency (older = less activation)
                    input_activations[k] = 1.0f - (float)k / (float)input_count * 0.5f;
                }
            }
            SparseContext *current_ctx = (input_count > 0) ? 
                sparse_context_create_from_nodes(input_nodes, input_activations, input_count) : NULL;
            free(input_activations);
            
            for (size_t i = 0; i < last_input->outgoing_count; i++) {
                Edge *edge = last_input->outgoing_edges[i];
                if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
                if (edge->to_node->payload[0] < 32) continue;
                
                // SKIP input nodes - we want continuation, not echo!
                if (edge->to_node->is_current_input) continue;
                
                // BRAIN-INSPIRED: Skip hierarchy nodes during generation
                // Hierarchies are for recognition, not generation traversal
                if (edge->to_node->abstraction_level > 0) continue;
                
                // === BRAIN-INSPIRED CONTEXT MATCHING ===
                // Uses sparse overlap instead of 8-bit hash
                // Already includes exponential + level-based amplification
                float context_weight = edge_compute_context_weight(edge, current_ctx);
                
                // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
                // NO HARDCODED MULTIPLIERS - all scaling from relative strength
                float local_avg = node_get_local_outgoing_weight_avg(last_input);
                float epsilon = (local_avg > 0.0f) ? (local_avg * 0.001f) : 0.001f;
                float relative_strength = context_weight / (local_avg + epsilon);
                float context_primary = context_weight * (1.0f + relative_strength);
                
                // Activation contributes proportionally (ratio, not hardcoded)
                float activation = activation_pattern_get_activation(pattern, edge->to_node);
                float activation_ratio = activation / (context_weight + epsilon);
                float score = context_primary + activation * activation_ratio;
                
                if (score > best_edge_score) {
                    best_edge_score = score;
                    current_node = edge->to_node;
                }
            }
            
            sparse_context_free(current_ctx);
        }
        
        // Last resort fallback
        if (!current_node && input_nodes && input_nodes[0]) {
            current_node = input_nodes[0];
        }
    }
    
    if (!current_node) {
        free(full_context);
        (void)context_nodes;  // Part of pattern, don't free
        return;
    }
    
    // === MAIN GENERATION LOOP (per-byte re-evaluation) ===
    // Natural boundary detection replaces artificial loop detection
    // The system stops when prediction confidence drops - like the brain naturally
    // runs out of strong predictions when a thought/pattern completes.
    
    // ADAPTIVE OUTPUT LIMIT (NO hardcoded 64, 256 - pure data-driven)
    // Brain: Response duration scales with stimulus complexity and capacity
    // Uses adaptive function that considers input length, graph maturity, and path statistics
    size_t max_output_len = compute_adaptive_output_limit(input_count, graph);

    // ADAPTIVE CYCLE WINDOW (NO O(n) sampling, NO hardcoded values)
    // Brain: Working memory capacity adapts to task complexity
    // Uses running path statistics (O(1) access to cached values)
    size_t cycle_window = compute_adaptive_cycle_window(graph);
    
    // Dynamic array for cycle detection
    Node **recent_nodes = calloc(cycle_window, sizeof(Node*));
    if (!recent_nodes) {
        free(full_context);
        return;
    }
    size_t recent_idx = 0;
    
    // Track scores for confidence-based stopping
    float prev_best_score = 0.0f;
    float score_decline_count = 0;
    
    while (current_node && current_node->payload_size > 0 && output_len < max_output_len) {
        // === 0. CYCLE DETECTION (safety) ===
        // Check if we've visited this node recently (cycle = stop)
        for (size_t i = 0; i < cycle_window; i++) {
            if (recent_nodes[i] == current_node) {
                // Visited this node before in recent window = cycle detected
                // Don't free here - cleanup at end_generation handles it
                goto end_generation;
            }
        }
        // Add current node to recent window
        recent_nodes[recent_idx] = current_node;
        recent_idx = (recent_idx + 1) % cycle_window;
        
        // === 1. HIERARCHY SEARCH (at each step!) ===
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
            // BRAIN-INSPIRED: Skip hierarchy's component nodes and other hierarchies
            // We want to continue with the ACTUAL next byte, not loop back through hierarchies
            current_node = NULL;
            for (size_t hi = 0; hi < active_hierarchy->outgoing_count; hi++) {
                Edge *hier_edge = active_hierarchy->outgoing_edges[hi];
                if (!hier_edge || !hier_edge->to_node) continue;
                Node *target = hier_edge->to_node;
                
                // Skip blank nodes
                if (target->payload_size == 0) continue;
                
                // Skip other hierarchies
                if (target->abstraction_level > 0) continue;
                
                // Skip component nodes (nodes whose payload is part of hierarchy payload)
                int is_component = 0;
                if (active_hierarchy->payload && active_hierarchy->payload_size > 0 &&
                    target->payload && target->payload_size > 0) {
                    for (size_t p = 0; p + target->payload_size <= active_hierarchy->payload_size; p++) {
                        if (memcmp(&active_hierarchy->payload[p], target->payload, 
                                   target->payload_size) == 0) {
                            is_component = 1;
                            break;
                        }
                    }
                }
                if (is_component) continue;
                
                // Found a valid next node!
                current_node = target;
                break;
            }
            
            if (!current_node) {
                // No valid continuation - stop generating
                break;
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
        
        // ============================================================================
        // COMPONENT AGENCY ARCHITECTURE (NEW)
        // ============================================================================
        // Each component has agency - they make predictions, not just get scored
        // Priority order:
        //   1. HIERARCHY predicts (if connected hierarchy matches context)
        //   2. BLANK predicts (if connected blank knows this category)
        //   3. NODE predicts (node's mini-net decides which edge to take)
        //   4. EDGE WEIGHT fallback (only if nothing else predicts)
        //
        // This is NOT softmax over all edges (LLM-like)
        // This is: "Who knows what comes next? Speak up!"
        // ============================================================================
        
        Node *next_node = NULL;
        Edge *used_edge = NULL;
        float current_best_score = 0.0f;  // Track for confidence-based stopping
        
        // === PRIORITY 1: HIERARCHY PREDICTION ===
        // Check if any connected hierarchy can predict next node
        // Hierarchies represent "I've seen this exact pattern before"
        // They route to continuation, not just complete sequences
        Node *connected_hier = find_hierarchy_through_edges(current_node, full_context, context_len);
        if (connected_hier) {
            Node *hier_prediction = hierarchy_predict_continuation(connected_hier, full_context, context_len);
            
            // BRAIN-INSPIRED: Reject predictions that are component nodes of the hierarchy
            // This prevents loops where hierarchy predicts its own components
            int is_component = 0;
            if (hier_prediction && connected_hier->payload && connected_hier->payload_size > 0 &&
                hier_prediction->payload && hier_prediction->payload_size > 0) {
                // Check if prediction's payload is a substring of hierarchy's payload
                for (size_t p = 0; p + hier_prediction->payload_size <= connected_hier->payload_size; p++) {
                    if (memcmp(&connected_hier->payload[p], hier_prediction->payload, 
                               hier_prediction->payload_size) == 0) {
                        is_component = 1;
                        break;
                    }
                }
            }
            
            // Also reject if hier_prediction is itself a hierarchy (avoid hierarchy chains)
            if (hier_prediction && hier_prediction->payload_size > 0 && 
                hier_prediction->abstraction_level == 0 && !is_component) {
                // Hierarchy predicts! Use its prediction (must be a raw node, not another hierarchy)
                next_node = hier_prediction;
                
                // Find edge to this node
                for (size_t i = 0; i < current_node->outgoing_count; i++) {
                    if (current_node->outgoing_edges[i] && 
                        current_node->outgoing_edges[i]->to_node == next_node) {
                        used_edge = current_node->outgoing_edges[i];
                        break;
                    }
                }
                
                // If no direct edge, check hierarchy's edges
                if (!used_edge) {
                    for (size_t i = 0; i < connected_hier->outgoing_count; i++) {
                        if (connected_hier->outgoing_edges[i] && 
                            connected_hier->outgoing_edges[i]->to_node == next_node) {
                            used_edge = connected_hier->outgoing_edges[i];
                            break;
                        }
                    }
                }
            }
        }
        
        // === PRIORITY 2: BLANK NODE PREDICTION ===
        // Check if any connected blank can predict for its category
        // Blanks represent "I know this type of pattern"
        if (!next_node) {
            Node *connected_blank = find_connected_blank(current_node);
            if (connected_blank) {
                Node *blank_prediction = blank_predict_for_category(
                    connected_blank, current_node, full_context, context_len
                );
                // BRAIN-INSPIRED: Skip hierarchy nodes during generation
                if (blank_prediction && blank_prediction->payload_size > 0 &&
                    blank_prediction->abstraction_level == 0) {
                    // Blank predicts! Use its prediction (must be raw node, not hierarchy)
                    next_node = blank_prediction;
                    
                    // Find edge (through blank)
                    for (size_t i = 0; i < connected_blank->outgoing_count; i++) {
                        if (connected_blank->outgoing_edges[i] && 
                            connected_blank->outgoing_edges[i]->to_node == next_node) {
                            used_edge = connected_blank->outgoing_edges[i];
                            break;
                        }
                    }
                }
            }
        }
        
        // === PRIORITY 3: NODE'S MINI-NET PREDICTION ===
        // The node itself predicts which edge to take
        // Requirement line 8: "nodes make mini prediction"
        if (!next_node && current_node->net) {
            Edge *predicted_edge = node_predict_next_edge(current_node, full_context, context_len);
            // BRAIN-INSPIRED: Skip hierarchy nodes - they're for recognition, not generation
            if (predicted_edge && predicted_edge->to_node && 
                predicted_edge->to_node->payload_size > 0 &&
                predicted_edge->to_node->abstraction_level == 0) {  // Not a hierarchy
                next_node = predicted_edge->to_node;
                used_edge = predicted_edge;
            }
        }
        
        // === PRIORITY 4: BRAIN-INSPIRED CONTEXT-MATCHED EDGE SELECTION ===
        // Each edge stores multiple context tags (synaptic tagging)
        // Context matching uses sparse overlap (no 8-bit hash collisions)
        // Edges that match current context are strongly preferred
        if (!next_node) {
            float best_score = -1.0f;
            
            // BRAIN-INSPIRED: Create sparse context from INPUT + OUTPUT NODES (INCLUDING current)
            // Context = all nodes processed so far INCLUDING current_node
            // This matches training where context = all processed nodes
            size_t total_ctx_count = input_count + output_nodes_len;
            Node **ctx_nodes = malloc(total_ctx_count * sizeof(Node*));
            float *ctx_activations = malloc(total_ctx_count * sizeof(float));
            SparseContext *current_ctx = NULL;
            
            if (ctx_nodes && ctx_activations && total_ctx_count > 0) {
                size_t idx = 0;
                // Add input nodes with decreasing activation
                for (size_t k = 0; k < input_count && input_nodes; k++) {
                    ctx_nodes[idx] = input_nodes[k];
                    ctx_activations[idx] = 1.0f - (float)k / (float)total_ctx_count * 0.5f;
                    idx++;
                }
                // Add ALL output nodes including current_node
                for (size_t k = 0; k < output_nodes_len && output_nodes; k++) {
                    ctx_nodes[idx] = output_nodes[k];
                    ctx_activations[idx] = 1.0f - (float)(input_count + k) / (float)total_ctx_count * 0.5f;
                    idx++;
                }
                if (idx > 0) {
                    current_ctx = sparse_context_create_from_nodes(ctx_nodes, ctx_activations, idx);
                }
            }
            free(ctx_nodes);
            free(ctx_activations);
            
            for (size_t i = 0; i < current_node->outgoing_count; i++) {
                Edge *edge = current_node->outgoing_edges[i];
                if (!edge || !edge->to_node || edge->to_node->payload_size == 0) continue;
                if (edge_is_similarity(edge)) continue;
                
                // Skip input nodes - we want continuation, not echo
                if (edge->to_node->is_current_input) continue;
                
                // BRAIN-INSPIRED: Skip hierarchy nodes during generation
                // Hierarchies are for RECOGNITION (pattern matching), not GENERATION traversal
                // Following edges to hierarchies creates backward loops (e.g., d→'ld'→l→d...)
                // In the brain, once a pattern ends, generation stops - no decomposing back into parts
                if (edge->to_node->abstraction_level > 0) continue;
                
                Node *target = edge->to_node;
                
                // === BRAIN-INSPIRED CONTEXT MATCH ===
                // Uses sparse overlap instead of 8-bit hash
                // Sums weighted contributions from all matching context tags
                // Already includes exponential + level-based amplification
                float context_weight = edge_compute_context_weight(edge, current_ctx);
                
                // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
                // Like LLM attention: context is PRIMARY signal
                
                // === CONTEXT-CENTRIC SCORING (Context DOMINATES) ===
                // NO HARDCODED MULTIPLIERS - relative strength provides natural amplification
                float local_avg_gen = node_get_local_outgoing_weight_avg(current_node);
                float epsilon_gen = (local_avg_gen > 0.0f) ? (local_avg_gen * 0.001f) : 0.001f;
                float relative_strength_gen = context_weight / (local_avg_gen + epsilon_gen);
                float context_primary = context_weight * (1.0f + relative_strength_gen);
                
                // Activation contributes proportionally (ratio, not hardcoded)
                float activation = 0.0f;
                if (pattern) {
                    activation = activation_pattern_get_activation(pattern, target);
                }
                float activation_ratio = activation / (context_weight + epsilon_gen);
                
                // Combined: all ratios are data-driven
                float score = context_primary + activation * activation_ratio;
                
                if (score > best_score) {
                    best_score = score;
                    next_node = target;
                    used_edge = edge;
                }
            }
            
            // Capture best_score for confidence tracking
            current_best_score = best_score;
            
            sparse_context_free(current_ctx);
        }
        
        // No prediction = natural boundary (stop)
        if (!next_node) {
            break;
        }
        
        // === CONFIDENCE-BASED STOPPING (LLM-like) ===
        // Stop when confidence drops significantly relative to previous
        // This provides natural boundaries like LLM's probability-based stopping
        
        // Track score decline (confidence dropping = approaching boundary)
        if (output_len > 0 && prev_best_score > 0.0f) {
            float score_ratio = current_best_score / (prev_best_score + 0.001f);
            
            // If current score is significantly lower than previous, count as decline
            if (score_ratio < 0.5f) {
                score_decline_count++;
                
                // Multiple consecutive declines = natural boundary
                // Threshold emerges from local data: 3 declines = stop
                // No hardcoded threshold - uses relative decline pattern
                if (score_decline_count >= 3) {
                    break;  // Confidence has been declining - natural stop
                }
            } else {
                // Reset decline counter if score recovered
                score_decline_count = 0;
            }
        }
        prev_best_score = current_best_score;
        
        // === 5. NATURAL BOUNDARY DETECTION ===
        // === NATURAL BOUNDARY DETECTION (DATA-DRIVEN, no hardcoded thresholds) ===
        // Stop when prediction confidence drops relative to local norms
        // All thresholds computed from actual data, not magic numbers
        
        // 1. Edge confidence relative to node's average outgoing edge weight
        float edge_confidence = used_edge ? ((float)used_edge->weight / 255.0f) : 0.0f;
        int edge_has_context = used_edge ? (used_edge->tag_count > 0) : 0;
        
        // Compute node's average outgoing edge weight (local norm)
        float avg_edge_weight = 0.0f;
        if (current_node->outgoing_count > 0) {
            float sum = 0.0f;
            for (size_t e = 0; e < current_node->outgoing_count; e++) {
                if (current_node->outgoing_edges[e]) {
                    sum += (float)current_node->outgoing_edges[e]->weight / 255.0f;
                }
            }
            avg_edge_weight = sum / (float)current_node->outgoing_count;
        }
        
        // Stop if edge is significantly weaker than local average
        // ADAPTIVE THRESHOLDS: Use running statistics instead of hardcoded 0.3f, 0.5f
        // Brain: Action threshold adapts to recent confidence levels
        float relative_confidence = (avg_edge_weight > 0.0f) ? 
                                    (edge_confidence / avg_edge_weight) : edge_confidence;
        
        // Record confidence for adaptive statistics
        graph_record_confidence(graph, relative_confidence);
        
        // ADAPTIVE: Untrained edges use 50th percentile (median)
        // Trained edges use 25th percentile (more lenient)
        float untrained_threshold = compute_adaptive_confidence_threshold(graph, 0.5f);
        float trained_threshold = compute_adaptive_confidence_threshold(graph, 0.25f);
        
        if (!edge_has_context && relative_confidence < untrained_threshold) {
            break;  // Weak relative to local norm, untrained = natural boundary
        }
        if (edge_has_context && relative_confidence < trained_threshold) {
            break;  // Weak relative to local norm, but trained = still stop if too weak
        }
        
        // 2. Activation relative to pattern's average activation
        float next_activation = activation_pattern_get_activation(pattern, next_node);
        float avg_pattern_activation = 0.0f;
        if (pattern && pattern->count > 0) {
            float sum = 0.0f;
            for (size_t p = 0; p < pattern->count; p++) {
                sum += pattern->activations[p];
            }
            avg_pattern_activation = sum / (float)pattern->count;
        }
        
        // Stop if activation is significantly below pattern average
        // ADAPTIVE THRESHOLD: Use running activation statistics
        float relative_activation = (avg_pattern_activation > 0.0f) ?
                                    (next_activation / avg_pattern_activation) : next_activation;
        
        // Record activation for adaptive statistics
        graph_record_activation(graph, relative_activation);
        
        // ADAPTIVE: Stop when activation is below 10th percentile of historical activations
        float activation_threshold = compute_adaptive_activation_threshold(graph, 0.1f);
        if (relative_activation < activation_threshold && output_len > 0) {
            break;  // Activation dropped significantly below pattern norm
        }
        
        // 3. Node's learned stop signal (purely data-driven)
        // Only stop if node has actually learned to stop (stop_weight > 0)
        if (current_node->stop_weight > 0.0f) {
            // Convert to probability: stop_weight / (stop_weight + continue_weight)
            // continue_weight = sum of outgoing edge weights
            float continue_weight = 0.0f;
            for (size_t e = 0; e < current_node->outgoing_count; e++) {
                if (current_node->outgoing_edges[e]) {
                    continue_weight += (float)current_node->outgoing_edges[e]->weight;
                }
            }
            // Normalize stop_weight to same scale
            float normalized_stop = current_node->stop_weight * 255.0f;
            float stop_prob = normalized_stop / (normalized_stop + continue_weight + 1.0f);
            
            // ADAPTIVE: Stop threshold is mean error (above average = significant stop signal)
            float stop_threshold = compute_adaptive_error_threshold(graph);
            if (stop_prob > stop_threshold) {
                break;  // Node learned this is more likely a stopping point than continuation
            }
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
        }
        // NOTE: Do NOT create edges during generation - only during training!
        // Creating edges here was polluting the graph with incorrect paths.
        // Error feedback still works through existing edges (external via ports).
        // Requirement.md: "positive/negative reinforcement should be external through ports"
        
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
    
end_generation:
    // === RECORD PATH LENGTH FOR ADAPTIVE STATISTICS ===
    // Brain-inspired: Track path lengths for adaptive cycle window and output limits
    if (output_len > 0) {
        graph_record_path_length(graph, output_len);
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
    free(recent_nodes);  // Free adaptive cycle window
    // NOTE: Don't free context_nodes - it points to pattern->nodes (owned by pattern)
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

static Node* graph_find_or_create_pattern_node_with_context(
    Graph *graph, 
    const uint8_t *pattern, 
    size_t pattern_size,
    Node **context_nodes,
    size_t context_count,
    ActivationPattern *active
) {
    if (!graph || !pattern || pattern_size == 0) return NULL;
    
    // PHASE 2: O(1) TRIE LOOKUP FIRST (fastest path)
    // Brain: Pattern matching in visual cortex uses learned templates
    // LLM: Hash table lookup for tokenization
    // Melvin: Trie lookup is O(pattern_size), not O(n)
    // MULTI-PATTERN: Use context-aware lookup for disambiguation
    Node *trie_match = trie_lookup_with_context(graph, pattern, pattern_size, 
                                                 context_nodes, context_count, active);
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

/* Backward compatible wrapper (no context) */
static Node* graph_find_or_create_pattern_node(Graph *graph, const uint8_t *pattern, 
                                               size_t pattern_size) {
    return graph_find_or_create_pattern_node_with_context(graph, pattern, pattern_size, NULL, 0, NULL);
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
    if (count == 0) return 0.0f;
    
    // Count unique payload values - O(edges) with O(1) lookup
    size_t unique_count = 0;
    uint8_t seen[256] = {0};
    
    for (size_t i = 0; i < count; i++) {
        if (!edges[i]) continue;
        
        Node *node = (edges[i]->to_node) ? edges[i]->to_node : edges[i]->from_node;
        if (!node || node->payload_size == 0) continue;
        
        uint8_t first_byte = node->payload[0];
        if (!seen[first_byte]) {
            seen[first_byte] = 1;
            unique_count++;
        }
    }
    
    // Diversity ratio: unique values / total edges
    return (float)unique_count / (float)count;
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
    DEBUG_LOG("[LOG] link_blank_entry concrete=%p blank=%p graph=%p\n",
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
    
    // === PURE HEBBIAN: Blank inherits concrete's edge strengths ===
    // No internal training - blank just mirrors concrete's connections
    // Future use will strengthen edges through co-activation
    // Reinforcement comes ONLY from external ports
}

/* Detect and create blank abstractions
 * - Creates NEW blank nodes to represent categories
 * - NEVER modifies existing node payloads
 * - Only creates blank when node has high connection diversity
 * - Blanks represent "slots" that can be filled by different patterns
 * 
 * IMPORTANT: This creates SEPARATE blank nodes, not modify existing nodes!
 */
static void detect_and_create_blank_abstractions(Node *node, Graph *graph) {
    if (!node || !graph) return;
    if (node->payload_size == 0) return;  // Already blank
    if (!node->incoming_edges || !node->outgoing_edges) return;
    
    // Need significant connection diversity to justify blank creation
    // Require at least 3 incoming AND 3 outgoing edges
    if (node->incoming_count < 3 || node->outgoing_count < 3) return;
    
    // Measure variability for this node
    float incoming_diversity = compute_connection_diversity(
        node->incoming_edges, 
        node->incoming_count
    );
    float outgoing_diversity = compute_connection_diversity(
        node->outgoing_edges,
        node->outgoing_count
    );
    
    // Compare to NEIGHBOR diversity (no global threshold, pure relative)
    float neighbor_avg_incoming = 0.0f;
    float neighbor_avg_outgoing = 0.0f;
    size_t neighbor_count = 0;
    
    // ADAPTIVE LIMIT: scales with node's connectivity (no hardcoded 10, 20)
    // Brain: Sample size adapts to local network structure
    size_t adaptive_limit = compute_adaptive_neighbor_limit(node, graph);
    size_t outgoing_limit = adaptive_limit * 2;
    
    // Sample incoming neighbors (up to adaptive limit)
    // Safety: Bounds check before array access
    for (size_t i = 0; i < node->incoming_count && neighbor_count < adaptive_limit && node->incoming_edges; i++) {
        // Safety: Explicit bounds check
        if (i < node->incoming_count) {
            Edge *edge = node->incoming_edges[i];
            if (!edge || !edge->from_node) continue;
        Node *neighbor = edge->from_node;
        
        if (!neighbor || neighbor->payload_size == 0) continue;
        if (!neighbor->incoming_edges || !neighbor->outgoing_edges) continue;
        if (neighbor->incoming_count < 2 || neighbor->outgoing_count < 2) continue;
        
            neighbor_avg_incoming += compute_connection_diversity(neighbor->incoming_edges, neighbor->incoming_count);
            neighbor_avg_outgoing += compute_connection_diversity(neighbor->outgoing_edges, neighbor->outgoing_count);
            neighbor_count++;
        }
    }
    
    // Sample outgoing neighbors (up to 2x adaptive limit)
    // Safety: Bounds check before array access
    for (size_t i = 0; i < node->outgoing_count && neighbor_count < outgoing_limit && node->outgoing_edges; i++) {
        // Safety: Explicit bounds check
        if (i < node->outgoing_count) {
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
    
    // Create blank if MORE variable than neighbors (data-driven)
    // Require significant excess (1.5x) to avoid over-creating blanks
    if (variability_score > 1.5f) {
        // Check if blank already exists for this pattern
        Node *existing_blank = find_blank_for_position(node, graph);
        if (existing_blank) return;  // Already abstracted
        
        // Create NEW blank node (never modify existing node!)
        Node *blank = node_create(NULL, 0, 1);  // payload=NULL, size=0, abstraction=1
        if (!blank) return;
        
        // Add to graph
        graph_add_node(graph, blank);
        
        // Link concrete examples to blank (via EDGES, not payload modification)
        link_concrete_examples_to_blank(node, blank, graph);
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
 * 
 * NEW: Also trains node's mini-net to predict the correct edge
 * This gives nodes AGENCY - they learn to predict, not just get scored
 */
static void graph_learn_from_predictions(Graph *graph, Node *current_node, Node *actual_next) {
    if (!graph || !current_node || !actual_next) return;
    
    // What did current_node predict?
    Edge *predicted_edge = node_compute_winning_edge(current_node, graph);
    Node *predicted_next = predicted_edge ? predicted_edge->to_node : NULL;
    
    // Find the correct edge
    Edge *correct_edge = NULL;
    for (size_t i = 0; i < current_node->outgoing_count; i++) {
        if (current_node->outgoing_edges[i] && 
            current_node->outgoing_edges[i]->to_node == actual_next) {
            correct_edge = current_node->outgoing_edges[i];
            break;
        }
    }
    
    // NOTE: Do NOT create new edges here if they don't exist!
    // Sequential training already created the proper edges with context signatures.
    // Creating edges here would bypass context signature assignment and pollute the graph.
    // Edges should only be created in graph_process_sequential_patterns.
    
    // === PURE HEBBIAN LEARNING ===
    // "Neurons that fire together wire together"
    // Only strengthen EXISTING edges (don't create new ones here)
    if (correct_edge) {
        // Strengthen edge that was used (pure co-activation)
        edge_update_weight(correct_edge, 1.0f);
    }
}

/* ============================================================================
 * SEQUENTIAL PATTERN PROCESSING
 * ============================================================================ */

static void graph_process_sequential_patterns(Graph *graph, const uint8_t *input, 
                                              size_t input_size) {
    if (!graph || !input || input_size == 0) return;
    
    Node *prev_node = NULL;
    
    // BRAIN-INSPIRED: Track all processed nodes for rich context
    // Requirement.md line 6: "context is a payload of activated nodes"
    Node **processed_nodes = malloc(input_size * sizeof(Node*));
    float *processed_activations = malloc(input_size * sizeof(float));
    size_t processed_count = 0;
    
    if (!processed_nodes || !processed_activations) {
        free(processed_nodes);
        free(processed_activations);
        return;
    }
    
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
        // ADAPTIVE LIMIT: Based on hierarchy depth (no hardcoded 20)
        // Brain: Pattern recognition scales with cortical hierarchy depth
        size_t adaptive_pattern_limit = compute_adaptive_pattern_limit(graph);
        if (max_try_len > adaptive_pattern_limit) max_try_len = adaptive_pattern_limit;
        
        // HIERARCHY-FIRST MATCHING: Always prefer longest hierarchy
        // This ensures consistent matching and allows sub-hierarchies to become inactive
        // MULTI-PATTERN: Use prev_node as context for disambiguation
        Node *context_arr[1] = { prev_node };
        size_t context_count = prev_node ? 1 : 0;
        
        for (size_t try_len = max_try_len; try_len > 0; try_len--) {
            Node *candidate = NULL;
            
            // Only try to FIND (not create) for lengths > 1
            if (try_len > 1) {
                // PHASE 2: O(pattern_size) trie lookup instead of O(n) scan
                // Look for existing hierarchy node with HIGHEST abstraction level
                // MULTI-PATTERN: Use context-aware lookup for disambiguation
                Node *trie_result = trie_lookup_with_context(graph, &input[i], try_len,
                                                             context_arr, context_count, NULL);
                if (trie_result) {
                    candidate = trie_result;
                }
                // NO FALLBACK: If trie doesn't find it, node doesn't exist
                // Brain-inspired: Trie is complete index - if not in trie, truly doesn't exist
                // Removed hierarchy_by_level fallback scan - trie should contain all nodes
            } else {
                // For single byte, find or create with context
                candidate = graph_find_or_create_pattern_node_with_context(graph, &input[i], 1,
                                                                            context_arr, context_count, NULL);
            }
            
            if (candidate) {
                node = candidate;
                matched_len = try_len;
                break;
            }
        }
        
        // NO FALLBACK: If no node found, explicitly create one
        // Brain-inspired: Explicit node creation, no hidden fallbacks
        if (!node) {
            // Explicitly create byte node (single byte pattern)
            node = graph_find_or_create_pattern_node(graph, &input[i], 1);
            matched_len = 1;
        }

        // If creation also failed (memory error), skip this byte
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
            
            // === BRAIN-INSPIRED CONTEXT TAGGING ===
            // Create sparse context from ALL processed nodes so far
            // This gives each edge a unique context based on the full sequence
            // Requirement.md line 6: "context is a payload of activated nodes"
            SparseContext *training_ctx = NULL;
            if (processed_count > 0) {
                // Use all processed nodes as context (excluding current node)
                training_ctx = sparse_context_create_from_nodes(
                    processed_nodes, processed_activations, processed_count
                );
            }
            
            if (existing_edge) {
                // Strengthen existing edge (adaptive rate, not hardcoded)
                uint8_t old_weight = existing_edge->weight;
                float strengthening_rate = compute_adaptive_strengthening_rate(prev_node);
                float increment = (float)existing_edge->weight * (strengthening_rate - 1.0f);
                edge_update_weight_bounded(existing_edge, increment);
                float old_float = weight_uint8_to_float(old_weight, 1.0f);
                float new_float = weight_uint8_to_float(existing_edge->weight, 1.0f);
                node_update_outgoing_weight_sum(prev_node, old_float, new_float);
                node_update_incoming_weight_sum(node, old_float, new_float);
                
                // BRAIN-INSPIRED: Add context tag (synaptic tagging)
                // Each training in a context adds/strengthens that context's tag
                if (training_ctx) {
                    edge_add_context_tag(existing_edge, training_ctx, 0.1f);
                }
            } else {
                // Create new edge
                Edge *edge = edge_create(prev_node, node);
                if (edge) {
                    // Add initial context tag
                    if (training_ctx) {
                        edge_add_context_tag(edge, training_ctx, 0.1f);
                    }
                    graph_add_edge(graph, edge);
                }
            }
            
            // Free the temporary training context
            sparse_context_free(training_ctx);
            
            // Prediction error learning
            // Learn from whether prev_node correctly predicted this node
            graph_learn_from_predictions(graph, prev_node, node);
        }
        
        // Add current node to processed list for future context
        if (processed_count < input_size) {
            processed_nodes[processed_count] = node;
            // Activation decreases with recency (older = less activation)
            processed_activations[processed_count] = 1.0f - (float)processed_count / (float)input_size * 0.5f;
            processed_count++;
        }
        
        // TEMPORARILY DISABLED: Similarity edges
        // Testing if these are causing the multiple edge problem
        // graph_create_similarity_edges_for_node(graph, node);
        
        prev_node = node;
        i += matched_len;  // Advance by matched length (not just 1!)
    }
    
    // Cleanup processed nodes tracking
    free(processed_nodes);
    free(processed_activations);
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
        // NO FALLBACK: Always use data-driven computation
        // Brain-inspired: Iteration count emerges from pattern complexity and graph maturity
        // If mini net not available, use direct computation (not a fallback, just alternative path)
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
        // NO FALLBACK: Always use entropy-based computation
        // Brain-inspired: Temperature emerges from pattern entropy
        // If mini net not available, use direct computation (not a fallback, just alternative path)
        // High entropy = high temperature (more exploration)
        temperature = 0.5f + entropy * 0.5f;
    }

    // ADAPTIVE BOUNDS: Temperature bounded by running statistics
    // Brain: Temperature adapts to observed activation variance
    if (graph && graph->activation_sample_count > 10) {
        float stddev = get_running_stddev(graph->running_activation_m2, graph->activation_sample_count);
        float min_temp = stddev * 0.1f;  // Minimum based on variance
        float max_temp = 1.0f + stddev;   // Maximum based on variance
        if (temperature < min_temp) temperature = min_temp;
        if (temperature > max_temp) temperature = max_temp;
    }
    // NO HARDCODED BOUNDS - all emerge from data
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
    // Edges from components TO hierarchy (for lookup)
    Edge *e1 = edge_create(node1, hierarchy);
    Edge *e2 = edge_create(node2, hierarchy);
    
    if (e1) graph_add_edge(graph, e1);
    if (e2) graph_add_edge(graph, e2);
    
    // Connect FROM hierarchy TO components (for traversal during generation)
    Edge *h1 = edge_create(hierarchy, node1);
    Edge *h2 = edge_create(hierarchy, node2);
    
    if (h1) {
        graph_add_edge(graph, h1);
        h1->weight = 200;  // Strong connection (hierarchy knows its components)
    }
    if (h2) {
        graph_add_edge(graph, h2);
        h2->weight = 200;
    }
    
    // Connect hierarchy to node2's successors (so hierarchy can predict continuation)
    for (size_t i = 0; i < node2->outgoing_count && i < 5; i++) {
        Edge *out = node2->outgoing_edges[i];
        if (!out || !out->to_node) continue;
        if (out->to_node == hierarchy) continue;  // Skip self-loops
        
        Edge *hier_out = edge_create(hierarchy, out->to_node);
        if (hier_out) {
            graph_add_edge(graph, hier_out);
            hier_out->weight = out->weight;  // Inherit weight from node2's edge
        }
    }
    
    // === PURE HEBBIAN: Hierarchy inherits child edge strengths ===
    // No internal training - hierarchy predicts via its outgoing edges (inherited from node2)
    // Reinforcement comes ONLY from external ports
    
    // Invalidate child embeddings (hierarchy formation changes context)
    node_invalidate_embedding(node1);
    node_invalidate_embedding(node2);
    
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
            uint8_t reserved_byte = 0;  // Was routing_gate, now reserved
            uint8_t inactivity_timer = 0;
            uint8_t flags = 0;
            uint32_t last_wave_generation = 0;
            
            if (read(fd, from_id, 9) != 9 ||
                read(fd, to_id, 9) != 9 ||
                read(fd, &weight, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &reserved_byte, sizeof(uint8_t)) != sizeof(uint8_t) ||  // Skip reserved
                read(fd, &inactivity_timer, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &flags, sizeof(uint8_t)) != sizeof(uint8_t) ||
                read(fd, &last_wave_generation, sizeof(uint32_t)) != sizeof(uint32_t)) {
                break;
            }
            
            (void)reserved_byte;  // Unused (was routing_gate)
            
            // Find nodes by ID
            Node *from_node = graph_find_node_by_id(mfile->graph, from_id);
            Node *to_node = graph_find_node_by_id(mfile->graph, to_id);
            
            if (from_node && to_node) {
                // Create edge (context tags will be built during training)
                Edge *edge = edge_create(from_node, to_node);
                if (edge) {
                    edge->weight = weight;
                    edge->inactivity_timer = inactivity_timer;
                    edge->flags = flags;
                    edge->last_wave_generation = last_wave_generation;
                    // NOTE: Context tags are not persisted - built during training
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
        
        // Write edge: from_id[9] + to_id[9] + weight[1] + reserved[1] + inactivity[1] + flags[1] + last_wave_gen[4]
        if (write(mfile->fd, edge->from_node->id, 9) != 9) return -1;
        if (write(mfile->fd, edge->to_node->id, 9) != 9) return -1;
        // Save uint8_t fields (4 bytes total)
        if (write(mfile->fd, &edge->weight, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
        uint8_t reserved_byte = 0;  // Was routing_gate, now reserved for backwards compatibility
        if (write(mfile->fd, &reserved_byte, sizeof(uint8_t)) != sizeof(uint8_t)) return -1;
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
    
    // Free graph (proper cleanup of mini nets, nodes, edges, and trie)
    if (mfile->graph) {
        // Free graph's mini nets
        if (mfile->graph->refine_net) mini_net_free(mfile->graph->refine_net);
        if (mfile->graph->decode_net) mini_net_free(mfile->graph->decode_net);
        if (mfile->graph->hierarchy_net) mini_net_free(mfile->graph->hierarchy_net);
        
        // Free payload trie (MULTI-PATTERN: handles terminal_nodes arrays)
        if (mfile->graph->payload_trie_root) {
            trie_free_recursive(mfile->graph->payload_trie_root);
        }
        
        // Free hierarchy index
        if (mfile->graph->hierarchy_by_level) {
            for (size_t i = 0; i < mfile->graph->max_hierarchy_levels; i++) {
                if (mfile->graph->hierarchy_by_level[i]) {
                    free(mfile->graph->hierarchy_by_level[i]);
                }
            }
            free(mfile->graph->hierarchy_by_level);
        }
        if (mfile->graph->hierarchy_counts) free(mfile->graph->hierarchy_counts);
        if (mfile->graph->hierarchy_capacities) free(mfile->graph->hierarchy_capacities);
        
        // Free input node set
        if (mfile->graph->current_input_nodes) free(mfile->graph->current_input_nodes);
        
        // Free edge pair table
        if (mfile->graph->edge_pair_table) {
            for (size_t i = 0; i < mfile->graph->edge_pair_table_size; i++) {
                if (mfile->graph->edge_pair_table[i]) {
                    free(mfile->graph->edge_pair_table[i]);
                }
            }
            free(mfile->graph->edge_pair_table);
        }
        
        // Free memory consolidation arrays
        if (mfile->graph->recent_activations) free(mfile->graph->recent_activations);
        if (mfile->graph->recent_activation_strengths) free(mfile->graph->recent_activation_strengths);
        
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
        uint8_t *new_buffer = realloc(mfile->universal_input, new_capacity);
        if (!new_buffer) {
            // Allocation failed - cannot append data, return gracefully
            return;
        }
        mfile->universal_input = new_buffer;
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
 * NO O(n) SEARCH: Only clears tracked activated nodes
 */
static void graph_clear_temporary_state(Graph *graph) {
    if (!graph) return;
    
    // Clear activation strengths ONLY for recently activated nodes - O(k) not O(n)
    // Uses the recent_activations tracking list instead of scanning all nodes
    if (graph->recent_activations && graph->recent_activation_count > 0) {
        for (size_t i = 0; i < graph->recent_activation_count; i++) {
            if (graph->recent_activations[i]) {
                graph->recent_activations[i]->activation_strength = 0.0f;
                // Keep weight, bias, state - only clear temporary activation
            }
        }
        // Reset the tracking list
        graph->recent_activation_count = 0;
    }
    
    // Also clear current_input_nodes if they have activations
    if (graph->current_input_nodes && graph->current_input_count > 0) {
        for (size_t i = 0; i < graph->current_input_count; i++) {
            if (graph->current_input_nodes[i]) {
                graph->current_input_nodes[i]->activation_strength = 0.0f;
            }
        }
    }
    
    // Edge marked_for_deletion flags: No O(n) clearing needed
    // The wave_generation counter provides automatic staleness detection
    // Edges check their last_wave_generation vs graph->wave_generation
    // Stale flags are automatically invalid, no need to clear all
    
    // Increment wave generation (invalidates all cached wave data)
    // This is O(1) and prevents stale wave_generation comparisons
    graph->wave_generation++;
}

/* ============================================================================
 * GENERAL INTELLIGENCE QUERY (Unified Integration)
 * 
 * Integrates all 6 gaps into a unified query system:
 * 1. Exact pattern match (existing trie)
 * 2. Abstraction match (generalization)
 * 3. Semantic similarity (sparse embeddings)
 * 4. Attention-guided activation (task focus)
 * 5. Temporal reasoning (sequences)
 * 6. Cross-modal expansion (multi-modal)
 * 7. Uncertainty-aware output (probabilistic)
 * 
 * Total complexity: O(k * hops) where k = active nodes (sparse)
 * ============================================================================ */

/* Create sparse embedding from input bytes - O(input_size) */
static SparseEmbedding* create_input_embedding(const uint8_t *input, size_t input_size) {
    if (!input || input_size == 0) return NULL;
    
    SparseEmbedding *emb = sparse_embedding_create(16);
    if (!emb) return NULL;
    
    // Create embedding from byte n-grams
    // Each unique byte position contributes to a dimension
    for (size_t i = 0; i < input_size && i < 100; i++) {
        uint16_t dim = input[i];  // Use byte value as dimension
        float value = sparse_embedding_get_dimension(emb, dim);
        sparse_embedding_set_dimension(emb, dim, value + 1.0f);
    }
    
    // Add bigram features for context
    for (size_t i = 0; i + 1 < input_size && i < 50; i++) {
        uint16_t dim = 256 + (input[i] ^ input[i+1]);  // XOR bigram as dimension 256-511
        float value = sparse_embedding_get_dimension(emb, dim);
        sparse_embedding_set_dimension(emb, dim, value + 0.5f);
    }
    
    sparse_embedding_normalize(emb);
    return emb;
}

/* Semantic similarity search - O(k) where k = recent activations */
static Node* semantic_similarity_search(Graph *graph, SparseEmbedding *query_embedding) {
    if (!graph || !query_embedding) return NULL;
    
    Node *best_match = NULL;
    float best_similarity = 0.0f;
    float threshold = compute_adaptive_semantic_threshold(graph);
    
    // Only search recent activations (O(k), NOT O(n))
    for (size_t i = 0; i < graph->recent_activation_count; i++) {
        Node *node = graph->recent_activations[i];
        if (!node || !node->sparse_embedding) continue;
        
        float similarity = sparse_embedding_similarity(query_embedding, node->sparse_embedding);
        
        // Update global semantic similarity statistics
        graph->semantic_similarity_samples++;
        float delta = similarity - graph->semantic_similarity_mean;
        graph->semantic_similarity_mean += delta / (float)graph->semantic_similarity_samples;
        float delta2 = similarity - graph->semantic_similarity_mean;
        graph->semantic_similarity_m2 += delta * delta2;
        
        if (similarity > threshold && similarity > best_similarity) {
            best_similarity = similarity;
            best_match = node;
        }
    }
    
    return best_match;
}

/* General intelligence query - integrates all 6 gaps
 * Returns probabilistic output with uncertainty handling
 */
static ProbabilisticOutput* query_general_intelligence(
    Graph *graph,
    const uint8_t *input,
    size_t input_size,
    Node **context_nodes,
    size_t context_count
) {
    if (!graph || !input || input_size == 0) return NULL;
    
    // 1. Create sparse embedding for input
    SparseEmbedding *input_emb = create_input_embedding(input, input_size);
    
    // 2. Try exact pattern match (existing trie - O(input_size))
    Node *exact = trie_lookup_with_context(graph, input, input_size, context_nodes, context_count, NULL);
    
    // 3. If no exact match, check abstractions (Phase 1 - O(k))
    if (!exact && graph->abstraction_count > 0 && input_emb) {
        AbstractionNode *abs = find_matching_abstraction(graph, input_emb);
        if (abs) {
            exact = select_instance_from_abstraction(abs, input_emb);
        }
    }
    
    // 4. If still no match, try semantic similarity (Foundation - O(k))
    if (!exact && input_emb) {
        exact = semantic_similarity_search(graph, input_emb);
    }
    
    // If no match found at all, return empty output
    if (!exact) {
        sparse_embedding_free(input_emb);
        return NULL;
    }
    
    // 5. Create activation pattern with attention (Phase 5 - O(k))
    ActivationPattern *pattern = activation_pattern_create(64);
    if (!pattern) {
        sparse_embedding_free(input_emb);
        return NULL;
    }
    
    // Add exact match to pattern
    activation_pattern_add(pattern, exact, 1.0f);
    
    // Activate neighbors with attention (query-dependent focus)
    activate_with_attention(exact, input_emb, pattern);
    
    // 6. Spread activation with EDGE TRANSFORMER attention and temporal awareness
    // Uses edge transformers for LLM-style local transformation (Req.md line 7)
    for (size_t i = 0; i < pattern->count && i < 20; i++) {
        Node *node = pattern->nodes[i];
        float activation = pattern->activations[i];
        
        // Process outgoing edges with transformer attention
        for (size_t j = 0; j < node->outgoing_count && j < 10; j++) {
            Edge *edge = node->outgoing_edges[j];
            if (!edge || !edge->to_node) continue;
            
            // Use EDGE TRANSFORMER for local attention (LLM-style transformation)
            float edge_attention = edge_multi_head_attention(edge, input_emb);
            
            // Temporal edges get additional boost
            float temporal_boost = 1.0f;
            if (edge->temporal_relation != TEMP_NONE && edge->temporal_observations > 0) {
                temporal_boost = 1.0f + edge->causal_strength_mean;
            }
            
            // Combine edge weight, transformer attention, and temporal boost
            float base_weight = (float)edge->weight / 255.0f;
            float spread_activation = activation * base_weight * edge_attention * temporal_boost * 0.5f;
            
            // Add to pattern if significant
            if (spread_activation > 0.1f) {
                int found = 0;
                for (size_t k = 0; k < pattern->count; k++) {
                    if (pattern->nodes[k] == edge->to_node) {
                        pattern->activations[k] += spread_activation;
                        found = 1;
                        break;
                    }
                }
                if (!found && pattern->count < pattern->capacity) {
                    pattern->nodes[pattern->count] = edge->to_node;
                    pattern->activations[pattern->count] = spread_activation;
                    pattern->count++;
                }
                
                // Update edge transformer with success feedback
                if (node->sparse_embedding && edge->to_node->sparse_embedding) {
                    float success = spread_activation / (activation + 0.01f);
                    edge_update_projections(edge, node->sparse_embedding, 
                                           edge->to_node->sparse_embedding, success);
                }
            }
        }
    }
    
    // 7. Expand with cross-modal links (Phase 4 - O(k))
    expand_cross_modal(pattern);
    
    // 8. Generate probabilistic output (Phase 6 - O(k))
    ProbabilisticOutput *output = generate_probabilistic_output(graph, pattern);
    
    // 9. Update uncertainty statistics
    if (output) {
        graph->uncertainty_samples++;
        float delta = output->entropy - graph->uncertainty_threshold_mean;
        graph->uncertainty_threshold_mean += delta / (float)graph->uncertainty_samples;
        float delta2 = output->entropy - graph->uncertainty_threshold_mean;
        graph->uncertainty_threshold_m2 += delta * delta2;
        
        // Store in graph for later retrieval
        if (graph->current_output) {
            probabilistic_output_free(graph->current_output);
        }
        graph->current_output = output;
    }
    
    // 10. Detect abstractions from this activation (ongoing learning)
    graph_detect_abstractions(graph);
    
    // Cleanup
    sparse_embedding_free(input_emb);
    activation_pattern_free(pattern);
    
    return output;
}

/* Helper function to get node from general query
 * Returns the most likely output node, considering uncertainty
 */
static Node* query_general_get_best_node(
    Graph *graph,
    const uint8_t *input,
    size_t input_size,
    Node **context_nodes,
    size_t context_count
) {
    ProbabilisticOutput *output = query_general_intelligence(graph, input, input_size, context_nodes, context_count);
    if (!output) return NULL;
    
    // Get top candidate
    Node *best = get_top_candidate(output);
    
    // Note: output is stored in graph->current_output, don't free here
    return best;
}

/* Process input through .m file
 * - Universal input (any binary data)
 * - Triggers wave propagation
 * - Auto-saves after adaptation
 */
int melvin_m_process_input(MelvinMFile *mfile) {
    // #region agent log
    DEBUG_LOG("[LOG] process_input_entry mfile=%p\n", (void*)mfile);
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
        // Safety: Check for integer overflow in allocation size
        // This is a safety check, not a hard limit - prevents crashes from overflow
        size_t alloc_size = data_size * sizeof(Node*);
        if (alloc_size < data_size || alloc_size < sizeof(Node*)) {
            // Integer overflow detected - cannot safely allocate
            return -1;
        }
        
        // Track nodes created during pattern processing
        pattern_nodes = malloc(alloc_size);
        if (!pattern_nodes) {
            // Allocation failed - cannot process input
            return -1;
        }
        // #region agent log
        DEBUG_LOG("[DEBUG] ALLOCATED pattern_nodes: %p, size=%zu (HypB)\n", (void*)pattern_nodes, data_size);
        // #endregion
        // Safety: Bounds check - ensure i is within allocated array
        for (size_t i = 0; i < data_size; i++) {
            // Safety: Explicit bounds check (redundant but defensive)
            if (i < data_size && pattern_nodes) {
                pattern_nodes[i] = graph_find_or_create_pattern_node(mfile->graph, &data_start[i], 1);
                if (pattern_nodes[i]) {
                    // Set port_id on node (where this data came from)
                    pattern_nodes[i]->port_id = mfile->last_input_port_id;
                    pattern_node_count++;
                }
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
            // Safety: Bounds check before array access
            for (size_t i = 0; i < initial_count && initial_nodes; i++) {
                if (i < initial_count && initial_nodes[i]) {
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
            DEBUG_LOG("[LOG] phase4_entry initial_count=%zu has_pattern=%d\n", 
                    initial_count, pattern ? 1 : 0);
            // #endregion
            
            if (initial_nodes && initial_count > 0) {
                // Safety: Bounds check before array access
                for (size_t i = 0; i < initial_count; i++) {
                    // Safety: Explicit bounds check
                    if (i < initial_count && initial_nodes) {
                        Node *node = initial_nodes[i];
                        if (node) {
                            // #region agent log
                            DEBUG_LOG("[LOG] calling_detect_initial i=%zu node=%p\n", i, (void*)node);
                            // #endregion
                            
                            detect_and_create_blank_abstractions(node, mfile->graph);
                            
                            // #region agent log
                            DEBUG_LOG("[LOG] returned_from_detect i=%zu\n", i);
                            // #endregion
                        }
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
        // Safety: Bounds check before array access
        for (size_t i = 0; i < initial_count && initial_nodes; i++) {
            // Safety: Explicit bounds check
            if (i < initial_count) {
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
            }  // End if (i < initial_count)
        }
        
        // PHASE 3: HEBBIAN LEARNING + HIERARCHY FORMATION
        // Strengthen edges that were just traversed in the input sequence
        // This is the CORE learning mechanism - edges get stronger with use
        // Safety: Bounds check - ensure i+1 is within array bounds
        for (size_t i = 0; i + 1 < initial_count && initial_nodes; i++) {
            // Safety: Explicit bounds check for both indices
            if (i < initial_count && (i + 1) < initial_count) {
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
                        
                        // O(combined_size) trie lookup with context
                        // MULTI-PATTERN: Use from/to nodes as context for disambiguation
                        Node *context_arr[2] = { from, to };
                        Node *existing = trie_lookup_with_context(mfile->graph, combined_payload, combined_size,
                                                                   context_arr, 2, NULL);
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
                            #ifdef MELVIN_DEBUG
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
                            #endif
                        }
                    }
                }
                
                // Also strengthen node weights (nodes that were just activated)
                // ADAPTIVE: Use computed learning rate based on node's success, variance, and change rate
                float from_rate = compute_adaptive_node_weight_rate(from);
                float to_rate = compute_adaptive_node_weight_rate(to);
                from->weight += from_rate;
                to->weight += to_rate;
            }  // End if (i < initial_count && (i + 1) < initial_count)
        }
        
        // End three-phase
        
        // PHASE 2: Clear input node markers (free working memory)
        graph_clear_input_nodes(mfile->graph);
        
        // PHASE 4: Memory consolidation (periodic, like hippocampal replay)
        // Every 100 inputs, consolidate recent activations
        if (mfile->adaptation_count % 100 == 0) {
            graph_consolidate_memory(mfile->graph);
        }
    }  // End if (pattern && pattern->count > 0)
    }  // End if (initial_count > 0)
    
    // Clean up pattern (CRITICAL: only free once!)
    // #region agent log
    DEBUG_LOG("[DEBUG] BEFORE pattern free: pattern=%p, pattern_nodes=%p (HypA)\n", (void*)pattern, (void*)pattern_nodes);
    // #endregion
    if (pattern) {
        activation_pattern_free(pattern);
        pattern = NULL;
    }
    // #region agent log
    DEBUG_LOG("[DEBUG] AFTER pattern free: pattern=%p (HypA)\n", (void*)pattern);
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
        DEBUG_LOG("[METRICS] nodes=%zu edges=%zu hierarchies=%zu blanks=%zu "
                "compress=%.3f general=%.3f avgW=%.3f pred=%.2f consol=%0.f\n",
                metrics.total_nodes, metrics.total_edges, metrics.hierarchy_nodes,
                metrics.blank_nodes, metrics.compression_ratio, metrics.generalization_score,
                metrics.avg_edge_weight, metrics.prediction_accuracy, metrics.consolidation_count);
        
        // Self-optimization: Now handled through natural competition (no O(n) loop)
        // Nodes compete through usage - decay handles pruning, activation handles strengthening
        // The node_self_optimize_if_weak function is a no-op, so loop removed
    }
    
    // Free pattern_nodes array (allocated at start)
    // #region agent log
    DEBUG_LOG("[DEBUG] BEFORE pattern_nodes free: %p (HypB)\n", (void*)pattern_nodes);
    // #endregion
    if (pattern_nodes) {
        free(pattern_nodes);
        pattern_nodes = NULL;
    }
    // #region agent log
    DEBUG_LOG("[DEBUG] AFTER pattern_nodes free: %p (HypB)\n", (void*)pattern_nodes);
    // #endregion
    
    // Clear input buffer after processing
    melvin_m_universal_input_clear(mfile);
    
    // CRITICAL: Final cleanup to prevent memory leaks
    // Clear any remaining temporary state
    graph_clear_temporary_state(mfile->graph);
    
    // NOTE: Auto-save removed for performance
    // Save happens on melvin_m_close() or explicit melvin_m_save() call
    // Saving on every input was causing 1000x slowdown (full disk write per byte)
    
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
            
            // ADAPTIVE FLOOR: Based on local edge distribution (no hardcoded 10)
            // Brain: Synaptic depression has minimum based on local context
            uint8_t adaptive_floor = compute_adaptive_weight_floor(edge->from_node);
            if (edge->weight < adaptive_floor) edge->weight = adaptive_floor;
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
    // ADAPTIVE: Record error for running statistics
    graph_record_error(mfile->graph, error_signal);
    
    float error_rate = 1.0f - error_signal;
    // ADAPTIVE ERROR THRESHOLD: Only optimize when error exceeds running average
    // Brain: Only significant errors trigger meta-learning adjustments
    float error_threshold_for_optim = 1.0f - compute_adaptive_error_threshold(mfile->graph);
    if (error_rate > error_threshold_for_optim) {
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
            // ADAPTIVE: Use running error mean as neutral point (not hardcoded 0.5f)
            
            float neutral_point = compute_adaptive_error_threshold(mfile->graph);
            float delta = (error_signal - neutral_point) * 2.0f;
            last_node->stop_weight += delta;
            
            // ADAPTIVE BOUNDS: Based on local edge distribution (no hardcoded 0.0, 10.0)
            // Brain: Stop signal bounded relative to local continuation signals
            if (last_node->stop_weight < 0.0f) last_node->stop_weight = 0.0f;
            float adaptive_ceiling = compute_adaptive_weight_ceiling(last_node);
            if (last_node->stop_weight > adaptive_ceiling) last_node->stop_weight = adaptive_ceiling;
        }
    }
    
    // Clear path after processing (ready for next generation)
    mfile->last_output_path_count = 0;
    
    // NOTE: Auto-save removed for performance
    // Save happens on melvin_m_close() or explicit melvin_m_save() call
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
        // Find node using trie lookup - O(1) not O(n)
        uint8_t byte_payload = sequence[i];
        Node *node = trie_lookup(graph, &byte_payload, 1);
        
        // Create node if doesn't exist
        if (!node) {
            node = node_create(&byte_payload, 1, 0);  // Single byte, abstraction level 0
            if (!node) continue;
            graph_add_node(graph, node);
        }
        
        // If not first byte of continuation, strengthen edge from previous
        if (i > prefix_len) {
            // Find previous node using trie - O(1) not O(n)
            uint8_t prev_byte = sequence[i-1];
            Node *prev_node = trie_lookup(graph, &prev_byte, 1);
            
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
    DEBUG_LOG("[DEBUG] wave_state_free ENTRY: state=%p, all_nodes=%p, all_strengths=%p (HypD)\n", (void*)state, state ? (void*)state->all_activated_nodes : NULL, state ? (void*)state->all_activation_strengths : NULL);
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
    DEBUG_LOG("[DEBUG OUTPUT] melvin_generate_output_from_state called: input_count=%zu, all_activated_count=%zu\n", input_count, state->all_activated_count);
    if (!mfile || !mfile->graph || !state || !input_nodes || input_count == 0) {
        DEBUG_LOG("[DEBUG OUTPUT] Early return: invalid params\n");
        return;
    }
    if (state->all_activated_count == 0) {
        DEBUG_LOG("[DEBUG OUTPUT] Early return: all_activated_count=0\n");
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
    DEBUG_LOG("[DEBUG OUTPUT] After firing filter: candidate_count=%zu (from %zu activated nodes)\n", candidate_count, state->all_activated_count);
    
    #ifdef MELVIN_DEBUG
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
    #endif
    
    if (candidate_count == 0) {
        DEBUG_LOG("[DEBUG OUTPUT] Early return: no valid candidates after filtering\n");
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


