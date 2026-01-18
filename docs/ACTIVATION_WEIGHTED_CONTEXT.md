# Activation-Weighted Context Embedding Implementation

## Change Made

**Location**: `compute_context_embedding()` (line ~2946)

**Before**: Used only first 10 nodes from activation pattern
```c
size_t max_nodes = pattern->count < 10 ? pattern->count : 10;
for (size_t i = 0; i < max_nodes; i++) {
    // ... aggregate embeddings ...
}
```

**After**: Use ALL activated nodes, weighted by their activation strength
```c
// Use ALL nodes in pattern, weighted by their activation strength
for (size_t i = 0; i < pattern->count; i++) {
    Node *node = pattern->nodes[i];
    float activation = pattern->activations[i];
    
    // Skip nodes with negligible activation (decayed to near-zero)
    if (activation < 0.001f) continue;
    
    // ... aggregate embeddings weighted by activation ...
}
```

## Rationale

- **Activation = temporal relevance**: High activation = recent/current context, low = decayed
- **Continuous environment**: In real usage (vision, audio, continuous input), activation dissipates over time
- **All nodes matter**: The "top 10" limit was arbitrary and discarded potentially relevant context

## Test Results

### ✅ Single Pattern
- Input: "cat"
- Output: "meow" (correct)
- Simple associations work perfectly

### ⚠️ Multi-Pattern Interference
- Pattern 1: "cat meow" (trained 20x)
- Pattern 2: "dog bark" (trained 20x)
- Test "cat" → "meog meog meog meog" (incorrect)
- Test "dog" → "meog meog meog meog" (incorrect)

### ⚠️ Non-Overlapping Characters
- Pattern 1: "abc xyz" (trained 20x)
- Pattern 2: "def uvw" (trained 20x)
- Test "abc" → "xyz" (correct, missing space)
- Test "def" → "xyz" (incorrect, should be "uvw")

## Analysis: Why Interference Persists

The activation-weighted context embedding improves context representation, but **multi-pattern interference remains** due to:

1. **Shared characters**: 'o', 'e', 'a' appear in both "meow" and "dog"/"bark"
2. **Weak differentiation**: Context embeddings are averaged over many nodes, diluting the signal
3. **First pattern bias**: The first pattern trained creates strong edges that dominate later patterns

## What This Reveals About Scale

At scale (GB of data, millions of nodes), this interference will be **pervasive**:
- Nodes will be reused across millions of contexts
- Byte-level nodes ('a', 'e', 'o') will have thousands of outgoing edges
- Context differentiation becomes critical for correct routing

## Next Steps to Address Interference

The current context embedding is a **global average** over activated nodes. At scale, we need:

1. **Stronger context signals**: Not just embedding similarity, but structural path matching
2. **Edge-specific context memory**: Edges remember "I was strengthened in context X"
3. **Competitive inhibition**: Edges from different patterns should suppress each other
4. **Hierarchical chunking**: "meow" and "bark" should become single nodes, not byte sequences

The fundamental issue: **byte-level nodes are too granular for multi-pattern disambiguation**.
