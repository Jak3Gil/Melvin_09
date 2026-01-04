# Edge-to-Node Ratio Analysis

## Problem Identified

**Current State**: 786 nodes, 388 edges = **2:1 ratio** (0.5 edges per node)

**Expected State**: Biological neural networks have:
- Human brain: ~86 billion neurons, ~100-1000 trillion synapses
- **Ratio: 1,000-10,000 edges per node**
- Even simple networks: **10-100 edges per node minimum**

## Root Cause

### Only Co-Activation Edges Are Created

Looking at `graph_process_sequential_patterns()` (line 1905):

```c
// Process byte-by-byte
for (size_t i = 0; i < input_size; i++) {
    Node *node = graph_find_or_create_pattern_node(graph, &input[i], 1);
    
    // Create co-activation edge from previous node
    if (prev_node) {
        Edge *edge = edge_create(prev_node, node);  // ONLY sequential edges
        graph_add_edge(graph, edge);
    }
    prev_node = node;
}
```

**Problem**: Only creates edges between sequential bytes (prev → current)
- For 333 bytes of input, creates ~333 edges
- But creates ~333 nodes (one per byte)
- Result: ~1 edge per node = 2:1 ratio

### Missing Edge Types (Per README)

The README specifies 4 types of edges, but only 1 is implemented:

1. ✅ **Co-Activation Edges**: Implemented (sequential patterns)
2. ❌ **Similarity Edges**: NOT implemented (should connect similar patterns)
3. ❌ **Context Edges**: NOT implemented (should connect nodes in same wave path)
4. ❌ **Homeostatic Edges**: NOT implemented (should prevent isolation)

### Missing Edge Creation Mechanisms

1. **Edge Creation from Novelty** (Line 3418): `// TODO: Implement edge creation from novelty`
2. **Similarity Edge Creation**: No function to create edges between similar patterns
3. **Context Edge Creation**: No function to create edges from wave propagation paths
4. **Homeostatic Edge Creation**: No function to prevent node isolation

## Expected Behavior

### Similarity Edges

Should be created when:
- Two nodes have similar payloads (byte similarity)
- Similarity exceeds adaptive threshold
- Creates bidirectional or unidirectional edge
- Weight based on similarity strength

**Expected Impact**: 
- If each node connects to 10 similar nodes: 786 nodes × 10 = 7,860 edges
- Ratio: **10:1** (much better, still conservative)

### Context Edges

Should be created when:
- Nodes appear in same wave propagation path
- Nodes activate together during wave
- Creates edges to capture contextual relationships

**Expected Impact**:
- Each wave path connects ~5-10 nodes
- Multiple paths create many context edges
- Could add **5-10 edges per node** = **3,930-7,860 edges**

### Homeostatic Edges

Should be created when:
- Node has too few connections (< threshold)
- Connects to nearby nodes (local context)
- Maintains minimum connectivity

**Expected Impact**:
- Ensures every node has at least 3-5 connections
- Adds **2-4 edges per isolated node**
- Could add **500-1,000 edges**

## Output Analysis

### Current Outputs

From test results:
- **No output generated (pure thinking mode)**
- Output size: 0 bytes
- System processes but doesn't generate output

### Why No Output?

Possible reasons:
1. Output generation requires activated nodes with sufficient weight
2. Temperature might be too low (no exploration)
3. Wave propagation might not be activating output candidates
4. Output readiness threshold might be too high

### Expected Outputs

For inputs like:
- "cat cat cat" → Should output something related to "cat"
- "hello world hello world" → Should output continuation or echo
- "repeat repeat repeat" → Should recognize repetition pattern

## Solutions Needed

### 1. Implement Similarity Edge Creation

```c
// After creating/finding a node, check for similar nodes
static void create_similarity_edges(Graph *graph, Node *new_node) {
    // Find similar nodes (payload similarity)
    for (size_t i = 0; i < graph->node_count; i++) {
        Node *other = graph->nodes[i];
        if (other == new_node) continue;
        
        float similarity = compute_pattern_similarity(new_node, other);
        float threshold = compute_adaptive_similarity_threshold(new_node);
        
        if (similarity > threshold) {
            // Create similarity edge
            Edge *edge = edge_create(new_node, other);
            if (edge) {
                edge->weight = similarity * local_avg;  // Medium weight
                graph_add_edge(graph, edge);
            }
        }
    }
}
```

### 2. Implement Context Edge Creation

```c
// After wave propagation, create edges between activated nodes
static void create_context_edges_from_wave(Graph *graph, Node **activated, size_t count) {
    // Create edges between nodes that activated together
    for (size_t i = 0; i < count; i++) {
        for (size_t j = i + 1; j < count; j++) {
            // Check if edge exists
            if (!edge_exists(activated[i], activated[j])) {
                Edge *edge = edge_create(activated[i], activated[j]);
                if (edge) {
                    edge->weight = context_weight;  // Weak context weight
                    graph_add_edge(graph, edge);
                }
            }
        }
    }
}
```

### 3. Implement Homeostatic Edge Creation

```c
// Ensure nodes have minimum connectivity
static void create_homeostatic_edges(Graph *graph, Node *node) {
    if (node->outgoing_count >= 3) return;  // Already connected
    
    // Find nearby nodes (local context)
    // Create weak edges to maintain connectivity
    // ...
}
```

### 4. Fix Output Generation

Need to investigate why outputs aren't being generated:
- Check output readiness calculation
- Check temperature computation
- Check wave propagation activation
- Check output candidate selection

## Expected Results After Fixes

### Edge Count Projections

**Current**: 388 edges (2:1 ratio)

**After Similarity Edges**: 
- 786 nodes × 10 similar = 7,860 edges
- **Total: ~8,000 edges** (10:1 ratio)

**After Context Edges**:
- 786 nodes × 5 context = 3,930 edges
- **Total: ~12,000 edges** (15:1 ratio)

**After Homeostatic Edges**:
- 200 isolated nodes × 3 = 600 edges
- **Total: ~12,600 edges** (16:1 ratio)

**Target Ratio**: **10-20:1** (edges per node)
- This is still conservative compared to biological networks
- But much better than current 2:1 ratio

## Action Items

1. ✅ **Implement Similarity Edge Creation** - Connect similar patterns
2. ✅ **Implement Context Edge Creation** - Connect nodes in wave paths
3. ✅ **Implement Homeostatic Edge Creation** - Prevent isolation
4. ✅ **Fix Output Generation** - Investigate why no outputs
5. ✅ **Test Edge Ratios** - Verify 10-20:1 ratio achieved

## Biological Reference

- **Cortical neurons**: 1,000-10,000 synapses per neuron
- **Cerebellar neurons**: 100,000+ synapses per neuron
- **Average brain**: ~1,000 synapses per neuron

Our target of **10-20 edges per node** is:
- 50-100x less than biological networks
- But appropriate for initial implementation
- Can grow organically as system learns

