# Mini-Net Context Learning Implementation

## Summary

Implemented two critical fixes to enable mini-nets to learn context-specific patterns:

1. **Context as Input**: Context is now part of the input vector (not a separate signal)
2. **Training During Learning**: Mini-nets are now updated when edges are strengthened

## Problem

Previously, mini-nets had two issues:

1. **Context was separate**: Passed as `context_signal` parameter, only used for forget gate
   - Mini-net couldn't learn: "When context_count=5 and base_weight=0.8, that's good"
   - Context wasn't part of the weighted sum, so it couldn't influence predictions

2. **No training**: `mini_net_update()` existed but was never called during edge strengthening
   - Mini-nets stayed at zero weights (neutral initialization)
   - Couldn't learn which input combinations lead to good predictions

## Solution

### Fix 1: Context as Input (Minimal Storage)

Changed from:
```c
float inputs[5] = {context_weight, base_weight, activation, relative_strength, local_avg};
float context_signal[2] = {context_count, context_level};
score = mini_net_forward(net, inputs, 5, context_signal, 2);
```

To:
```c
float inputs[7] = {
    context_weight,
    base_weight,
    activation,
    relative_strength,
    local_avg,
    context_count,      // Context as input (computed on-demand)
    context_level       // Context as input (computed on-demand)
};
score = mini_net_forward(net, inputs, 7, NULL, 0);
```

**Storage impact**: ZERO
- Context values are computed on-demand from `SparseContext` (already exists)
- Just 2 more floats in the input array (stack-allocated, temporary)
- No additional storage in nodes or edges

### Fix 2: Training During Learning

Added mini-net training in `graph_process_sequential_patterns()` (lines 10329-10349 and 10369-10389):

```c
// After strengthening an edge (existing or new):
if (prev_node->net && training_ctx) {
    // Compute the same inputs used during prediction
    float context_weight = edge_compute_context_weight(edge, training_ctx);
    float base_weight = (float)edge->weight / 255.0f;
    float activation = node->activation_strength;
    float local_avg = node_get_local_outgoing_weight_avg(prev_node);
    float epsilon = (local_avg > 0.0f) ? (local_avg * 0.001f) : 0.001f;
    float relative_strength = context_weight / (local_avg + epsilon);
    float context_count = (float)training_ctx->count / 100.0f;
    float context_level = (float)training_ctx->max_abstraction_level / 10.0f;
    
    float inputs[] = {
        context_weight, base_weight, activation, 
        relative_strength, local_avg,
        context_count, context_level
    };
    
    // Positive outcome: this edge was correct (ground truth from training data)
    mini_net_update(prev_node->net, inputs, 7, 1.0f, 1.0f);
}
```

**How it works**:
- When an edge is used in training (co-activation), we know it's correct
- Compute the same inputs that would be used during prediction
- Update mini-net with `outcome=1.0` (positive reinforcement)
- Mini-net learns: "These inputs → good prediction"

## Results

### Simple Test (PASS ✅)
- Pattern: "AB"
- Query: "A"
- Output: "B"
- **Accuracy: 100%**

The mini-net successfully learns that after "A", "B" should follow.

### Complex Test (FAIL ❌)
- Patterns: 20 associations like "hello world", "cat meow", etc.
- Queries: Partial patterns like "hello", "cat", etc.
- Output: Mostly wrong or empty
- **Accuracy: 0%**

## Current Issue

The complex test fails because:

1. **Query mismatch**: Training uses full patterns ("hello world"), but queries use partial patterns ("hello")
2. **Context difference**: During training, context includes all previous nodes. During generation from a partial query, context is different
3. **Possible missing generation logic**: The generation phase might not be using the same prediction mechanism as training

## Next Steps

Need to investigate why generation fails on partial patterns:

1. Check if `generate_from_pattern()` is using the mini-net correctly
2. Verify that context during generation matches context during training
3. Debug why partial pattern queries don't activate the correct continuations
4. Consider if we need to train on partial patterns as well (not just full patterns)

## Files Modified

- `src/melvin.c`:
  - Lines 3571-3587: Updated `node_predict_next_edge_sparse()` to include context as input
  - Lines 9002-9014: Updated first generation call to include context as input
  - Lines 9382-9400: Updated subsequent generation calls to include context as input
  - Lines 10329-10349: Added mini-net training for existing edges
  - Lines 10369-10389: Added mini-net training for new edges

## Alignment with Requirements

✅ **No hardcoded limits**: Context size is adaptive (from `SparseContext`)
✅ **Data-driven**: Mini-nets learn from data, not hardcoded formulas
✅ **Minimal storage**: No additional storage overhead
✅ **Local decisions**: Each node's mini-net makes local predictions
✅ **Hebbian learning**: Strengthening based on co-activation (outcome=1.0 for used edges)
