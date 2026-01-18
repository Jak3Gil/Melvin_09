# No Internal Feedback Implementation

## Summary

Removed all internal feedback during generation. Edges are **no longer strengthened** based on internal generation decisions. Learning happens only:
1. During training (Hebbian learning from co-occurrence in `create_sequential_edges`)
2. From external feedback (`melvin_m_feedback_error` - external system provides error signals)

## Changes Made

### 1. Removed STOP Edge Strengthening During Generation

**Before** (lines 1993-2011):
```c
if (!next) {
    if (current && graph && graph->stop_node) {
        Edge *stop_edge = find_edge_between(current, graph->stop_node);
        if (stop_edge) {
            // Internal feedback: Strengthen STOP edge when it wins
            float context_match = ...;
            float activation_used = ...;
            hebbian_strengthen_edge_precise(stop_edge, graph, activation_used, context_match);
        }
    }
    break;
}
```

**After** (lines 1992-1997):
```c
if (!next) {
    // STOP edge won or no next node - stop generation
    // NO INTERNAL FEEDBACK: STOP edges are not strengthened here
    // Learning comes from external feedback only (melvin_m_feedback_error)
    DEBUG_LOG("[GENERATE] Stopping: no next node or STOP edge won\n");
    break;
}
```

### 2. Removed Regular Edge Strengthening During Generation

**Before** (lines 2000-2016):
```c
// PRECISE HEBBIAN LEARNING: Context-based learning
float context_match = ...;
Edge *edge = find_edge_between(current, next);
if (edge) {
    // Internal feedback: Strengthen edge when used during generation
    float activation = ...;
    hebbian_strengthen_edge_precise(edge, graph, activation, context_match);
}
```

**After** (lines 1998-2002):
```c
// NO INTERNAL FEEDBACK: Edges are not strengthened during generation
// Learning happens only:
// 1. During training (create_sequential_edges - Hebbian learning from co-occurrence)
// 2. From external feedback (melvin_m_feedback_error - external system provides error signals)

current = next;
```

## Learning Sources

### 1. Training (create_sequential_edges)
- **When**: During `melvin_m_process_input` when pattern is processed
- **How**: Hebbian learning from co-occurrence - edges strengthen when nodes appear together
- **Location**: `create_sequential_edges()` (lines 769-815)

### 2. External Feedback (melvin_m_feedback_error)
- **When**: External system calls `melvin_m_feedback_error(mfile, error_signal)`
- **How**: External system provides error signals (0.0 = wrong, 1.0 = correct)
- **Location**: `melvin_m_feedback_error()` function

## Benefits

1. **No Internal Assumptions**: System doesn't assume its own decisions are correct
2. **External Control**: Learning happens only from external feedback (if provided)
3. **Pure Training**: Edges learn only from training data co-occurrence patterns
4. **No Runaway Strengthening**: STOP edges and regular edges can't over-strengthen from internal feedback

## Status

✅ **Implementation Complete**
- Removed STOP edge strengthening during generation
- Removed regular edge strengthening during generation
- All learning now comes from training (Hebbian) or external feedback
- Code compiles successfully
- No linter errors

## Test Results

Note: Error rates still show 100% after 50+ iterations with empty outputs. This suggests the issue is not just internal feedback, but may be related to:
- Activation spreading becoming too weak
- Normalization suppressing activations
- `decode_select_first` returning NULL (no candidates)

Further investigation needed to identify root cause of empty outputs after many training iterations.
