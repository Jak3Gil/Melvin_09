# Passive Context Implementation Results

## Changes Implemented

### 1. Removed Context Tag Storage from Edges
**Before:**
- Edge stored ContextTag array (100-1000 bytes per edge)
- Each tag stored full SparseContext copy
- Massive memory overhead

**After:**
- Edge has context_tags fields but they're DEPRECATED (always NULL, tag_count = 0)
- No context storage in edges
- Context computed passively from existing data

### 2. Implemented Passive Context Computation
**New function: `edge_compute_context_weight()`**
```c
// Passive signals (no storage):
float target_activation = edge->to_node->activation_strength;  // Wave propagation
float embedding_sim = sparse_embedding_similarity(from_emb, to_emb);  // Semantic
return target_activation + embedding_sim;  // Combined passive signal
```

**Signals used:**
1. Target activation (from wave propagation)
2. Embedding similarity (semantic context)
3. All derived from existing data - zero storage

### 3. Updated Edge Selection
**In `node_predict_next_edge_sparse()`:**
```c
// Start with learned edge weight
float score = base_relative;

// Add passive context signals
score += target_activation;  // Wave propagation context
score += embedding_sim;      // Semantic context

// Mini-net refines
if (node->net) {
    float refinement = mini_net_forward(node->net, inputs);
    score += refinement;
}
```

### 4. Removed Context Tag Additions
- Commented out all `edge_add_context_tag()` calls
- No context tags created during training
- Edges rely on passive computation only

---

## Test Results

### test_associations:
```
Nodes: 41
Edges: 1488
Edge-to-Node Ratio: 36.29:1 ✅

Input: "hello"
Output: "?? ??i ?? d? d? d? dheh ? d? d? d? d? dheh ? d? d?" (139 bytes)
❌ Gibberish

Input: "cat"
Output: " " (1 byte)
❌ Wrong

Input: "apple"
Output: " ?" (2 bytes)
❌ Wrong
```

### test_simple:
```
Nodes: 9
Edges: 43
Output: '�' (garbage)
❌ Wrong
```

---

## Analysis: Why Passive Context Isn't Working

### Problem 1: Target Activation is Zero
**During generation:**
- Input: "hello"
- Wave propagation activates nodes
- But by the time we're selecting edges, target nodes may not be activated yet
- `target_activation = 0.0` for most edges
- No context signal!

**Example:**
- At node 'h', choosing next edge
- Target 'e' hasn't been activated yet (we haven't reached it)
- `edge->to_node->activation_strength = 0.0`
- No context boost!

### Problem 2: Embeddings Not Discriminating
**Embedding similarity:**
- Computed from payload bytes
- All single-byte nodes have similar embeddings
- 'e' vs 'x' might have similarity 0.5 vs 0.4 (not enough difference)
- Can't reliably distinguish

### Problem 3: No Training Signal
**Without context tags:**
- Edges don't remember "I was trained in context [h,e,l,l,o]"
- Can't distinguish "h→e in 'hello'" from "h→e in 'help'"
- All edges score similarly based on weight alone

---

## The Fundamental Issue

**Passive context requires:**
1. Target nodes to be pre-activated (wave propagation)
2. Embeddings to be discriminating (semantic similarity)
3. Some memory of training context

**Current reality:**
1. Target nodes NOT pre-activated during generation
2. Embeddings not discriminating enough for single bytes
3. No memory of training context (removed context tags)

**Result:**
- All edges score similarly
- Selection is random
- Outputs are gibberish

---

## Memory Savings

**Before:**
- Edge: 48 bytes + context_tags (100-1000 bytes)
- Total: 464-1,296 bytes per edge

**After:**
- Edge: 48 bytes (context_tags present but empty)
- Total: 48 bytes per edge

**Savings: 90-96% memory reduction** ✅

**But accuracy is 0%** ❌

---

## Conclusion

**Passive context is:**
- Memory efficient ✅ (90-96% reduction)
- Brain-like ✅ (computation, not storage)
- Requirement.md compliant ✅ (no per-edge storage)

**But doesn't work for accuracy:**
- Target nodes not pre-activated ❌
- Embeddings not discriminating ❌
- No training context memory ❌

**The trade-off:**
- Storage: Excellent (48 bytes vs 1,296 bytes)
- Accuracy: Terrible (0% vs ~20%)

**Next steps:**
1. Pre-activate target nodes during wave propagation
2. Improve embedding discrimination
3. Or: Add minimal context memory (not full SparseContext)
4. Or: Accept that we need some context storage for accuracy
