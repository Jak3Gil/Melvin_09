# Consolidation Test Results

## Fixes Implemented

### 1. Hierarchy-First Matching (Greedy Longest-Match)
**Location**: `melvin.c` lines 3882-3923

**Change**: Modified `graph_process_sequential_patterns()` to prefer hierarchies with higher abstraction levels when multiple hierarchies match the same pattern.

```c
// Before: Just find any matching hierarchy
// After: Find hierarchy with HIGHEST abstraction level
Node *best_candidate = NULL;
uint32_t best_level = 0;

for (size_t h = 0; h < graph->node_count; h++) {
    Node *hier = graph->nodes[h];
    if (memcmp(hier->payload, &input[i], try_len) == 0) {
        if (!best_candidate || hier->abstraction_level > best_level) {
            best_candidate = hier;
            best_level = hier->abstraction_level;
        }
    }
}
```

**Goal**: Ensure longest/highest-level hierarchies are matched consistently, making sub-hierarchies inactive.

### 2. Passive Decay Acceleration for Unused Hierarchies
**Location**: `melvin.c` lines 1710-1739

**Change**: Modified `compute_adaptive_node_timer_increment()` to accelerate decay for hierarchies with low usage ratios.

```c
if (node->abstraction_level > 0) {
    float usage_ratio = (float)node->total_activations / (node->incoming_weight_sum + epsilon);
    
    if (usage_ratio < 0.1f) {
        float acceleration = 1.0f / (usage_ratio + 0.1f);  // Up to 10x faster decay
        base_increment *= acceleration;
    }
}
```

**Goal**: Hierarchies that aren't providing memory benefit (components used more than the hierarchy) decay faster.

## Test Results

### Test 1: Consolidation Over 1,000 Iterations
**Input**: "hello" repeated 1,000 times
**Results**:
- Graph stabilizes at **58 nodes, 115 edges** by iteration 100
- No consolidation observed (node count doesn't decrease)
- Output: "lllloooolllooooooooo" (INCORRECT)

### Test 2: Consolidation Over 10,000 Iterations
**Input**: "hello" repeated 10,000 times
**Results**:
- Graph remains at **58 nodes, 115 edges** (no change from iteration 100)
- No consolidation even after 10,000 iterations
- Output: "lllloooolllooooooooo" (INCORRECT)

### Test 3: Error Rate Test
**Input**: "hello world" repeated 200 times
**Results**:
- Error rate: **63.6%** (no improvement)
- Graph grows to 225 nodes
- Output: Random "llllooo..." patterns

## Root Cause Analysis

### Problem 1: Wrong Hierarchies Are Forming
**Debug output shows**:
```
[DEBUG] Creating hierarchy: from='l' + 'o' → "lo" ✓ CORRECT
[DEBUG] Creating hierarchy: from='o' + 'lo' → "olo" ✗ WRONG
[DEBUG] Creating hierarchy: from='o' + 'olo' → "oolo" ✗ WRONG
```

**Why**: Wave propagation traverses edges in any order (not just sequential). When output feedback creates edges like o→l, o→lo, these get used to form useless hierarchies.

**Expected**: "he", "el", "ll", "lo" → "hel", "ell", "llo" → "hello"
**Actual**: "lo" → "olo" → "oolo" (recursive "o" patterns)

### Problem 2: Hierarchies Not Found During Output Generation
**Debug output shows**:
```
[DEBUG] Edge selection: full_seq='hello' (5 bytes), hierarchy=NOT FOUND
```

**Why**: `find_active_hierarchy()` is called but returns NULL. The "hello" hierarchy doesn't exist (only "lo", "olo", "oolo").

**Result**: Output generation falls back to random edge selection, producing "llllooo..." loops.

### Problem 3: Sub-Hierarchies Not Decaying
**Why**: Sub-hierarchies like "lo", "olo" are still being matched during input processing (greedy longest-match finds them), so they remain active. Active nodes don't decay.

**Passive decay only works if**:
1. Hierarchies become inactive (not matched)
2. Their usage_ratio is low

**Current state**: Sub-hierarchies are still matched, so they never become inactive, so they never decay.

## Why Fixes Didn't Work

### Fix 1 (Hierarchy-First Matching) Didn't Help Because:
- It prefers higher abstraction levels, but "hello" hierarchy doesn't exist
- It still matches "olo" and "oolo" because they're the highest-level hierarchies available
- The problem isn't matching priority, it's that the WRONG hierarchies are being created

### Fix 2 (Passive Decay) Didn't Help Because:
- Hierarchies are still being matched during input processing, so they remain active
- Active nodes don't decay (inactivity_timer resets to 0 on activation)
- Even with 10x acceleration, if a node is activated every iteration, it never decays

## The Real Problem

**Wave propagation creates hierarchies from ANY edge traversal, not just sequential input patterns.**

When wave propagation traverses:
- l → o (from input "hello") → creates "lo" hierarchy ✓
- o → l (from output feedback "llllooo") → strengthens o→l edge
- o → lo (from wave propagation) → creates "olo" hierarchy ✗

The system can't distinguish between:
1. **Forward edges** (from sequential input) - should form hierarchies
2. **Feedback edges** (from output) - should NOT form hierarchies
3. **Random traversal edges** (from wave propagation) - should NOT form hierarchies

## Potential Solutions

### Option A: Track Edge Source
Add a field to `Edge` to track whether it was created from:
- Input processing (sequential pattern)
- Output feedback
- Wave propagation

Only create hierarchies from input-sourced edges.

**Pros**: Precise control
**Cons**: Violates "no extra tracking" principle

### Option B: Validate Hierarchy Payloads
Before creating a hierarchy from node1 + node2, check if the combined payload appears as a substring in recent inputs.

**Pros**: Data-driven validation
**Cons**: Requires storing recent inputs (more tracking)

### Option C: Require Higher Edge Weight for Hierarchies
Only create hierarchies from edges that have been strengthened many times (e.g., weight > 10.0).

**Pros**: Simple, no new tracking
**Cons**: May delay useful hierarchy formation

### Option D: Disable Output Feedback
Stop feeding output back as input, eliminating feedback edges.

**Pros**: Eliminates source of wrong edges
**Cons**: Breaks continuous learning loop (violates README)

### Option E: Hierarchy Formation Only During Input Processing
Move hierarchy formation from `wave_propagate()` to `graph_process_sequential_patterns()`.

**Pros**: Hierarchies only form from sequential input patterns
**Cons**: Changes architecture (hierarchies should emerge from wave propagation per README)

## Recommendation

**Option C (Require Higher Edge Weight)** is the simplest fix that aligns with the README:
- No new tracking variables
- Data-driven (uses existing edge weights)
- Hierarchies form from frequently repeated patterns (high weight = many repetitions)
- Random feedback edges won't have high enough weight

**Implementation**:
```c
// In wave_collect_hierarchy_signals():
// Signal 2: Repetition - require MUCH higher weight
float min_weight_for_hierarchy = local_avg * 5.0f;  // Must be 5x stronger than average
if (edge->weight < min_weight_for_hierarchy) {
    signals[1] = 0.0f;  // Zero out repetition signal
}
```

This would prevent "olo" from forming unless the o→lo edge has been strengthened 5x more than average edges, which would only happen if "olo" actually appears frequently in input.

## Next Steps

1. Implement Option C (higher weight threshold)
2. Test with "hello" repeated 1,000 times
3. Verify that only "lo" (and eventually "hello") hierarchies form
4. Verify that output improves ("hel" → "lo" instead of "llllooo")
5. If successful, test with "hello world" to verify multi-word patterns

