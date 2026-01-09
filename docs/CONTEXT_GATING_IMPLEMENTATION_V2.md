# Context Gating Implementation V2

## Summary

Implemented context-aware edge gating based on pipeline audit findings:
1. **Context gating using edge strength + target activation**
2. **Limited multi-hop spreading** (1-2 hops max, was 2-5)
3. **Simplified gating logic** (no context_trace decoding)

## Changes Made

### 1. Context Gating in Edge Scoring (lines 5268-5350)

**Previous approach:**
- Used target activation as primary signal (5x boost)
- No differentiation between strong/weak edges

**New approach:**
```c
// Compute edge strength relative to neighbors
float edge_strength = edge->weight / (local_avg + epsilon);

// Detect weak edge to highly activated target (WRONG PATH)
if (edge_strength < 1.0f && target_activation > 5.0f) {
    context_match = 0.1f;  // Heavy suppression
}
// Detect strong edge to activated target (CORRECT PATH)
else if (edge_strength > 1.5f && target_activation > 1.0f) {
    context_match = 2.0f;  // Boost
}
// Strong edge to non-activated target (UNEXPLORED)
else if (edge_strength > 1.5f && target_activation < 0.5f) {
    context_match = 1.0f;  // Neutral
}

score *= context_match;
```

**Rationale:**
- Weak edge + high activation = distant node (wrong)
- Strong edge + high activation = trained path (correct)
- Strong edge + low activation = valid but unexplored

### 2. Limited Multi-Hop Spreading (lines 4440-4455)

**Previous:**
```c
int max_hops = 2 + (int)(avg_graph_degree / 3.0f);  // Range: 2-5 hops
if (max_hops > 5) max_hops = 5;
```

**New:**
```c
int max_hops = (avg_graph_degree > 3.0f) ? 2 : 1;  // Range: 1-2 hops
```

**Rationale:**
- Multi-hop spreading was activating distant nodes like 'd' (end of patterns)
- Limit to 1-2 hops so only immediate continuations are activated

### 3. Reduced Activation Boost (line 5330)

**Previous:**
```c
score *= (1.0f + target_activation * 5.0f);  // 5x boost
```

**New:**
```c
score *= (1.0f + target_activation * 2.0f);  // 2x boost
```

**Rationale:**
- Activation is now secondary to context gating
- Reduced boost prevents over-weighting of activated nodes

## Test Results

### Simple Pattern Test
- **Before**: 0% error ("world" ✓)
- **After**: 86.7% error ("wolo wolo wolo" ✗)
- **Status**: REGRESSION - simple patterns fail

### Conditional Test
- **Before**: "orlll" or "wododbyeldodbyel"
- **After**: "wododbyeldodbyel" (unchanged)
- **Status**: NO IMPROVEMENT

## Analysis

### Why It Still Fails

The context gating logic is based on:
- Edge strength (relative to local average)
- Target activation (from spreading)

But the problem is:
1. **All edges have similar strength** after training
   - l→l: ~50 iterations
   - l→o: ~50 iterations
   - l→d: ~50 iterations
   - Relative strengths: all ≈ 1.0

2. **Target activations are still high** even with limited spreading
   - After "hello w", spreading still reaches:
     - 'o' from w→o (1-hop) ✓
     - 'r' from w→o→r (2-hop) ✓
     - But also 'l' and 'd' from other paths

3. **Context gating thresholds are arbitrary**
   - `edge_strength < 1.0f && target_activation > 5.0f` → suppress
   - But what if edge_strength = 1.1 and activation = 4.9?
   - These thresholds are **hardcoded**, violating requirements

### The Fundamental Issue

**The system cannot distinguish paths using only:**
- Edge weights (all similar)
- Target activations (spreading activates multiple paths)
- Local context (context_trace not effectively used)

**What's needed:**
- **Path-specific information** encoded in edges
- **Sequence memory** beyond first-order transitions
- **Context matching** that doesn't rely on hardcoded thresholds

### Why Simple Patterns Regressed

The context gating logic suppresses edges incorrectly:
- In "hello world", after generating "w", the edge w→o has:
  - Edge strength: ~1.0 (trained equally with other edges)
  - Target activation: varies based on spreading
- If activation is high, it might get suppressed by the "weak edge + high activation" rule
- This causes the system to pick wrong edges

## Conclusion

The implementation is **theoretically sound** but **practically ineffective** because:

1. **Equal edge weights** → no differentiation signal
2. **Hardcoded thresholds** → violates "no hardcoded limits" requirement
3. **Context_trace unused** → the actual context signal is ignored
4. **First-order Markov** → fundamental limitation

The real solution requires:
- **Using context_trace properly** to match edge targets against expected continuations
- **Higher-order transitions** that encode P(next | path) not just P(next | current)
- **Learning path signatures** without hardcoded thresholds

The current approach tried to work around the fundamental limitation (first-order Markov) with heuristics (edge strength + activation), but heuristics with hardcoded thresholds violate the core principles.
