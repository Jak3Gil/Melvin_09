# Hierarchy Learning Debug Summary

## What We Fixed

### 1. Input Processing Bug ✓
**Problem**: Universal input was skipping the first byte as "port_id"
- Input "hello" → processed as "ello" (missing 'h')
- Input "hel" → processed as "el" (missing 'h')

**Fix**: Changed `melvin_m_process_input` to process ALL bytes for universal input
```c
// OLD: data_start = input + 1; (skipped first byte)
// NEW: data_start = input; (process all bytes)
```

**Result**: ✓ Now correctly processes all input bytes

### 2. Output Start Node Bug ✓
**Problem**: Output generation started from FIRST input node instead of LAST
- Input [h, e, l] → started from 'h' instead of 'l'
- Should continue from last input character

**Fix**: Changed output generation to start from last input node
```c
// OLD: Node *start_node = input_nodes[0];
// NEW: Node *start_node = input_nodes[input_count - 1];
```

**Result**: ✓ Now starts from correct position

### 3. Hierarchy Matching Bug ✓
**Problem**: `find_active_hierarchy()` only checked OUTPUT, not INPUT + OUTPUT
- Input "hel" + output "" → only checked "" against hierarchies
- Should check "hel" against hierarchies

**Fix**: Build full_sequence from input_nodes + output before calling find_active_hierarchy
```c
// Build full_sequence: INPUT + OUTPUT
for (size_t i = 0; i < input_count; i++) {
    full_sequence[full_len++] = input_nodes[i]->payload[0];
}
for (size_t i = 0; i < output_len; i++) {
    full_sequence[full_len++] = output[i];
}
Node *active_hierarchy = find_active_hierarchy(graph, full_sequence, full_len, ...);
```

**Result**: ✓ Now checks full sequence

## What's Still Broken

### Core Problem: Hierarchies Not Being Created

**Observation**: 
- Graph grows (5 bytes → 13 nodes, 60 nodes after 500 iterations)
- But `find_active_hierarchy()` finds 0 hierarchies with `abstraction_level > 0`
- Extra nodes are NOT hierarchies!

**Debug Output**:
```
[DEBUG find_active_hierarchy] Searching for sequence: 'hello' (len=5)
[DEBUG] Checked 0 hierarchies, best_match=0x0
```

This means:
1. Nodes are being created
2. But they don't have `abstraction_level > 0`
3. So they're not hierarchy nodes

### Why Aren't Hierarchies Being Created?

The hierarchy creation process:
1. Wave propagation collects signals (dominance, repetition, maturity, compression)
2. Computes probability using learned weights
3. Compares to threshold (starts at 0.6 = 60%)
4. Creates hierarchy if probability > threshold

**Hypothesis**: Signals are too weak to reach 60% threshold

**Signals needed**:
- **Dominance**: Edge weight / local average (how much stronger than neighbors)
- **Repetition**: Edge activation frequency
- **Maturity**: Pattern stability (low variance)
- **Compression**: Would combining save computation

**Problem**: Early in training, these signals are weak:
- Edges haven't been repeated enough → low repetition signal
- Weights are still changing → low maturity signal
- No edge is dominant yet → low dominance signal

### What Are The Extra Nodes?

If not hierarchies, the extra 8 nodes (13 - 5 = 8) must be:
1. **Blank nodes** (generalization nodes)
2. **Duplicate byte nodes** (e.g., multiple 'l' nodes)
3. **Similarity nodes** (created by similarity edge logic)

## Proposed Solutions

### Solution 1: Lower Initial Hierarchy Threshold
```c
// Current: graph->adaptive_hierarchy_threshold = 0.6f;
// Proposed: graph->adaptive_hierarchy_threshold = 0.3f;  // Start more permissive
```

**Rationale**: Let hierarchies form earlier, then adapt threshold based on experience

### Solution 2: Boost Repetition Signal
After N repetitions of the same pattern, the repetition signal should be very high.

**Check**: Is edge weight actually increasing with repetitions?

### Solution 3: Force Hierarchy Creation After Threshold
After seeing the same sequence N times (e.g., 50), force hierarchy creation regardless of probability.

**Rationale**: If we've seen "hello" 100 times, we KNOW it's a pattern worth abstracting

### Solution 4: Debug Hierarchy Signals
Add logging to see what signals are actually being generated:
```c
fprintf(stderr, "Hierarchy signals: dominance=%.2f, repetition=%.2f, maturity=%.2f, compression=%.2f, prob=%.2f\n",
        signals[0], signals[1], signals[2], signals[3], probability);
```

## Current Test Results

**Error Rate**: 100% (no improvement over 500 iterations)
**Output Quality**: Random single bytes ('e', 'a', 'o', etc.)
**Hierarchies Created**: 0 (despite node growth)

**Conclusion**: The system is learning edges (graph grows) but not forming hierarchies, so it can't guide intelligent output generation.

## Next Steps

1. Add debug logging to `wave_collect_hierarchy_signals` and `wave_compute_hierarchy_probability`
2. Check if signals are reaching threshold
3. If not, lower threshold or boost signals
4. Verify hierarchies are actually created
5. Verify `find_active_hierarchy` finds them
6. Verify hierarchy guidance works in output generation

## Expected Behavior After Fix

**Training**: "hello" × 100
- Hierarchies created: [he], [el], [ll], [lo], [hel], [ell], [llo], [hello]
- All with `abstraction_level > 0`

**Testing**: Input "hel"
- `find_active_hierarchy("hel", 3)` → finds [hello] hierarchy
- Hierarchy says: position 3 = 'l', position 4 = 'o'
- Output: "lo" ✓

**Result**: Error rate drops from 100% to 0%

