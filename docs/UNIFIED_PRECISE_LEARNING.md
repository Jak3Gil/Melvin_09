# Unified Precise Learning Implementation

## Summary

Removed all internal accuracy scoring. Error feedback is now **external only** (via `melvin_m_feedback_error`). Precise learning is unified to use **context-based signals only** (embeddings + context functions) during generation.

## Key Changes

### 1. Removed Internal Accuracy Assumptions

**Before:**
```c
// Generation assumed accuracy = 1.0 (wrong!)
float accuracy = 1.0f;  // Assume correct (will be refined with error feedback)
hebbian_strengthen_edge_precise(edge, graph, activation, context_match, accuracy);
```

**After:**
```c
// Generation uses context-only learning (NO internal scoring)
hebbian_strengthen_edge_precise(edge, graph, activation, context_match, 0.0f);  // 0.0 = context-only
```

### 2. Unified Precise Learning

**`hebbian_strengthen_edge_precise()` now supports:**
- **Context-only learning** (`prediction_accuracy = 0.0`): Uses only context_match + activation
- **External error feedback** (`prediction_accuracy > 0.0`): Uses external error signal from `melvin_m_feedback_error()`

**Learning signals:**
1. **Hebbian**: Co-activation strengthening (base signal)
2. **Context**: Bonus from context match (from embeddings/context functions)
3. **Error**: External error signal ONLY (when provided via error feedback)

### 3. Error Feedback Uses Precise Learning

**Before:**
```c
// Simple increment/decrement (not unified)
if (error_signal > 0.5f) {
    hebbian_strengthen_edge(edge, graph);  // Simple
} else {
    edge->weight--;  // Simple
}
```

**After:**
```c
// Uses precise learning with context computation
// - Computes context_match from embeddings/context functions
// - Uses external error_signal as prediction_accuracy
// - Unified with generation learning
hebbian_strengthen_edge_precise(edge, graph, activation, context_match, error_signal);
```

## How It Works

### During Generation (Context-Only Learning)

1. **Compute context_match** from embeddings/context functions:
   - `node_compute_context_embedding()` → node embedding modulated by context
   - `compute_context_embedding()` → aggregate context embedding
   - `sparse_embedding_similarity()` → context match strength

2. **Apply precise learning** with context-only:
   ```c
   hebbian_strengthen_edge_precise(edge, graph, activation, context_match, 0.0f);
   ```
   - Uses: Hebbian + Context bonus
   - No error signal (context-only)

### During Error Feedback (External Error Signal)

1. **Reconstruct context** from output path:
   - Create activation pattern from `last_output_path`
   - Compute context_match from embeddings/context functions

2. **Apply precise learning** with external error signal:
   ```c
   hebbian_strengthen_edge_precise(edge, graph, activation, context_match, error_signal);
   ```
   - Uses: Hebbian + Context bonus + External error signal
   - `error_signal` comes from external system (not computed internally)

## Benefits

1. **No Internal Scoring**: System doesn't assume correctness - only learns from context
2. **Unified Learning**: Same precise learning function for generation and error feedback
3. **Context-Driven**: All learning uses context signals (embeddings + context functions)
4. **External Feedback**: Error signals come from external system only (not internally computed)

## Files Modified

- `src/melvin.c`:
  - `hebbian_strengthen_edge_precise()`: Made `prediction_accuracy` optional (0.0 = context-only)
  - `generate_output()`: Removed `accuracy = 1.0f` assumption, uses context-only learning
  - `create_sequential_edges()`: STOP edge training uses context-only learning
  - `melvin_m_feedback_error()`: Now uses precise learning with context computation + external error signal

## Testing

✅ `test_association_debug`: PASS - Context-only learning works
✅ Compilation: No errors
✅ Linting: No errors

## Status

**Implementation Complete**
- ✅ Removed all internal accuracy assumptions
- ✅ Unified precise learning (context-only + external error)
- ✅ Error feedback uses precise learning with context
- ✅ All learning uses context-based signals (embeddings + context functions)
