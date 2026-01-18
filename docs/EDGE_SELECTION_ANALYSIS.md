# Edge Selection Analysis: How Edges Are Chosen

## Summary

Investigated how edges are chosen during output generation. Found **two different code paths** with **absolute/relative value mismatches** that may be causing incorrect edge selection.

## Code Paths

### Path 1: `generate_from_pattern()` (ACTIVE)
**Location**: `melvin.c` line 11482 (called from `melvin_m_process_input()`)

**Edge Selection**: Uses softmax + probabilistic sampling (lines 8726-8860)

**How it works:**
1. **Priority 1**: Hierarchy guidance (if inside learned hierarchy)
2. **Priority 2**: Blank node prediction
3. **Priority 3**: Node's mini-net prediction
4. **Priority 4**: Softmax over all edges:
   - Collects all candidates with scores
   - Applies softmax to get probabilities
   - Samples probabilistically

**Score Computation** (lines 8784-8814):
```c
float context_weight = edge_compute_context_weight(edge, current_ctx);  // ABSOLUTE
float base_weight = (float)edge->weight / 255.0f;  // ABSOLUTE
float relative_strength_gen = context_weight / (local_avg_gen + epsilon_gen);  // RELATIVE

// Mini-net gets BOTH absolute and relative:
float inputs[] = {
    context_weight,      // ❌ ABSOLUTE
    base_weight,         // ❌ ABSOLUTE
    activation,         // ❌ ABSOLUTE
    relative_strength_gen,  // ✅ RELATIVE
    local_avg_gen,      // ✅ RELATIVE (context)
    context_count,
    context_level
};
score = mini_net_forward(current_node->net, inputs, 7, NULL, 0);
```

**Problem**: Mixing absolute and relative values in mini-net inputs!

---

### Path 2: `node_compute_winning_edge_with_context()` (NOT USED)
**Location**: `melvin.c` lines 6111-6256

**Status**: Defined but **never called** (only declaration found, no call sites)

**Edge Selection**: Winner-take-all (deterministic)

**How it works:**
1. **Priority 1**: Hierarchy guidance
2. **Priority 2**: Context-weighted selection:
   - Uses `edge_compute_context_weight()` (absolute)
   - Divides by `local_avg` to make relative (line 6230)
   - Winner-take-all: highest `context_value` wins

**Score Computation** (lines 6223-6250):
```c
float context_weight = edge_compute_context_weight(edge, current_ctx);  // ABSOLUTE
float local_avg = node_get_local_outgoing_weight_avg(node);
context_value = (local_avg > epsilon) ? (context_weight / local_avg) : context_weight;  // Makes relative
```

**Problem**: `edge_compute_context_weight()` returns absolute, then divided by local_avg (double-scaling issue)

---

## Core Issue: `edge_compute_context_weight()` Returns Absolute Value

**Location**: `melvin.c` lines 5566-5680

**Current Implementation:**
```c
float base_weight = (float)edge->weight / 255.0f;  // Base weight [0, 1]

// ... softmax over context tags ...
float context_probability = ...;  // Probability [0, 1]
float context_boost = context_probability * adaptive_mult * level_boost * attention_score;
float context_multiplier = 1.0f + context_boost;  // Multiplier [1.0, ~3.0]

return base_weight * context_multiplier;  // ❌ ABSOLUTE: [0, ~3.0]
```

**Problem**: Returns absolute weight, not relative boost factor!

**What it should return:**
- Relative boost factor: `context_multiplier` (1.0 = no boost, 2.0 = double strength)
- NOT: `base_weight * context_multiplier` (absolute value)

---

## The Double-Scaling Problem

**Example:**
1. Edge has `weight = 200` (strong edge)
2. `base_weight = 200/255 = 0.78`
3. Context matches well: `context_multiplier = 2.0`
4. `edge_compute_context_weight()` returns: `0.78 * 2.0 = 1.56` (absolute)

**Then in edge selection:**
- Path 1 (softmax): Uses `1.56` directly in score (absolute)
- Path 2 (winner-take-all): Divides by `local_avg` (e.g., `1.56 / 0.3 = 5.2`)

**Issue**: 
- Path 1: Mixes absolute (`context_weight`) with relative (`relative_strength_gen`) in mini-net
- Path 2: Double-scaling (absolute value then divided by local_avg)

---

## Context Tag Creation

**Location**: `melvin.c` lines 9739, 9784, 9842, 9853

**When**: During training (wave propagation)

**How**:
```c
// During training, when edge is used:
edge_add_context_tag(existing_edge, training_ctx, 0.1f);
```

**What it does**:
- Creates/updates context tags on edges
- Each tag stores: `context` (SparseContext), `weight_contribution` (how often trained)
- Uses relative competition: only merges if best match is 1.5x better than second-best

**Status**: ✅ Context tags ARE being created during training

---

## The Real Problem

### Issue 1: Absolute vs Relative Mismatch
- `edge_compute_context_weight()` returns absolute value
- Edge selection tries to make it relative by dividing by `local_avg`
- But absolute value already includes `base_weight`, creating double-scaling

### Issue 2: Mixed Values in Mini-Net
- Mini-net receives both absolute (`context_weight`, `base_weight`) and relative (`relative_strength_gen`) values
- This confuses the mini-net: it doesn't know which scale to use

### Issue 3: Context Matching May Not Be Working
- Context tags are created during training ✅
- But `edge_compute_context_weight()` uses softmax over tags
- If tags don't match current context, `context_probability = 0.0`
- Then `context_boost = 0.0`, so `context_multiplier = 1.0` (no boost)
- Result: Context matching has no effect!

---

## What Should Happen

1. **`edge_compute_context_weight()` should return relative boost factor:**
   ```c
   // Return: context_multiplier (1.0 = no boost, 2.0 = double strength)
   return context_multiplier;  // NOT base_weight * context_multiplier
   ```

2. **Edge selection should use relative values consistently:**
   ```c
   float context_boost = edge_compute_context_weight(edge, current_ctx);  // Relative [1.0, ~3.0]
   float base_relative = edge->weight / local_avg;  // Relative strength
   float final_score = base_relative * context_boost;  // All relative!
   ```

3. **Mini-net should receive only relative values:**
   ```c
   float inputs[] = {
       base_relative,        // ✅ RELATIVE
       context_boost,        // ✅ RELATIVE
       relative_strength,    // ✅ RELATIVE
       local_avg,           // ✅ CONTEXT (for reference)
       // No absolute values!
   };
   ```

---

## Next Steps

1. **Fix `edge_compute_context_weight()`**: Return relative boost factor, not absolute weight
2. **Fix edge selection**: Use relative values consistently
3. **Fix mini-net inputs**: Remove absolute values, use only relative
4. **Test**: Verify context matching actually affects edge selection

This should fix the edge selection issues and make context matching work properly.
