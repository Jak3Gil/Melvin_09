# Autoregressive Context Building Implementation

## Summary

Implemented true autoregressive generation: output 1 byte → add to context → re-spread → predict next byte.

## Key Changes

### 1. Removed Fallback Mechanisms (Requirement Compliance)
- **Removed section 4b**: Pattern activation candidates (violated "No Fallbacks")
- **Removed section 4c**: Blank node candidates (also a fallback)
- **Now uses ONLY edges** from current node (Requirement line 7: "edges are the only paths")

### 2. Activation-Based Scoring
Changed from context_trace matching to target node activation:

```c
// Before: Context gating based on payload byte matching
float context_gate = (matches context_trace) ? recency_weight : 0.0f;

// After: Activation-based (spreading activation IS the context)
float target_activation = activation_pattern_get_activation(pattern, target);
if (target_activation > 0.0f) {
    score *= (1.0f + target_activation * 5.0f);  // 5x boost for activated nodes
} else {
    score *= 0.01f;  // 99% suppression for non-activated
}
```

**Rationale**: Requirement line 6 says "context is a payload, of that activated nodes". Activated nodes ARE the context. Edges leading to highly activated nodes should win.

### 3. Simplified Spreading in `activation_pattern_update_with_node`
Changed from complex multi-hop spreading to simple 1-hop:

```c
// Before: Multi-hop spreading with edge_transform_activation
float transformed = edge_transform_activation(edge, 1.0f, graph);

// After: Simple edge weight spreading
float spread_activation = edge->weight;  // Raw learned weights
activation_pattern_add(pattern, edge->to_node, spread_activation);
```

**Rationale**: Autoregressive means each output primes ONLY the immediate next output, not the entire future path.

### 4. Decay Old Spread Activations
Added logic to distinguish sequence nodes (input + generated) from spread nodes (multi-hop):

```c
// Decay spread nodes heavily (90%), keep sequence nodes
for (size_t i = 0; i < pattern->count; i++) {
    int in_sequence = /* check if node is in pattern->sequence */;
    if (!in_sequence) {
        pattern->activations[i] *= 0.1f;  // Clear old spreading
    }
}
```

**Rationale**: Prevents distant nodes (like 'd' at end of patterns) from dominating through multi-hop spreading during ENCODE.

### 5. Update `context_trace` During Generation (CRITICAL FIX)
Added missing `context_trace` update for generated nodes:

```c
// After generating next_node, update its context_trace
if (output_nodes && output_nodes_len > 0) {
    update_node_context_trace(next_node, output_nodes, output_nodes_len, output_nodes_len - 1);
}
```

**Rationale**: This was the KEY missing piece! Without this, generated nodes don't know what came before them. Now:
- 'l' after "wor" has `context_trace = [w,o,r]`
- 'l' after "hel" has `context_trace = [h,e,l]`
- These are DIFFERENT contexts, enabling disambiguation

## Test Results

### Simple Pattern Test (`test_simple_error_rate.c`)
- **Before**: 0% error ("world" ✓)
- **After**: 86.7% error ("wolo wolo wolo" ✗)
- **Status**: REGRESSION - simple patterns now fail

### Conditional Test (`test_conditional_vs_memorization.c`)
- **Before**: "orlll" (self-loop)
- **After**: "wododbyeldodbyel" (mixing patterns)
- **Status**: STILL FAILING - different failure mode

## Analysis

### What Changed
1. ✅ Removed fallbacks (requirement compliance)
2. ✅ Activation-based scoring (aligns with requirements)
3. ✅ Autoregressive spreading (1-hop only)
4. ✅ Context_trace updates during generation
5. ✅ Decay of old spread activations

### What's Still Broken
The output "wododbyeldodbyel" shows the system is now:
- Mixing characters from different patterns ('w' from "world", 'o', 'd' from "goodbye", 'b', 'y', 'e', 'l')
- Creating loops ("odbyel" repeats)
- Not following any single trained path

### Root Cause
The fundamental issue remains: **Node reuse + equal edge weights = ambiguity**

When at node 'l':
- Edge l→l (from "hello")
- Edge l→o (from "hello")  
- Edge l→d (from "world")

All have similar weights (trained ~50 times each). Even with activation-based scoring, if multiple targets are activated (due to spreading), the system can't distinguish which path to follow.

### Why Simple Patterns Now Fail
The decay of spread activations (90%) is too aggressive. For "hello world", after generating "w", we spread to "o", but then decay it heavily. This makes "o" less activated than other nodes that happen to be in the pattern from ENCODE.

## Conclusion

The changes implement the user's request for autoregressive context building, but reveal a deeper architectural issue:

**Melvin's graph-based Markov chain with node reuse cannot solve conditional branching with first-order transitions alone.**

The system needs either:
1. **Higher-order transitions**: Edges that encode P(next | previous, current) not just P(next | current)
2. **Path-specific edge weights**: Edges remember which sequence they belong to
3. **Stronger context signals**: The `context_trace` needs to more strongly influence edge selection

The current approach (activation-based scoring) is theoretically sound but practically insufficient because:
- Spreading activates multiple competing paths
- Edge weights alone don't encode path membership
- Context_trace is populated but not effectively used in scoring
