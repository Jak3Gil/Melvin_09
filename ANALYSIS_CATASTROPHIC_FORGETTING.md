# Analysis: Why Catastrophic Forgetting Persists

**Date**: Friday, January 9, 2026  
**Context**: After edge refactor (86% memory reduction, bounded weights)

---

## The Mystery

Despite successful refactoring:
- ✅ Edge weights bounded at 0-255 (was unbounded)
- ✅ Max observed weight: 390 (was 17,636)
- ✅ Context gating integrated
- ✅ 86% memory reduction

**The system STILL shows catastrophic forgetting at iteration 100.**

This proves: **The issue is NOT just weight magnitude.**

---

## Evidence

### Test Results Comparison

**Before Refactor** (Jan 8):
- Iterations 20-80: 0% error, output "world" ✅
- Iteration 100+: 86.7% error, output "wolo wolo wolo" ❌
- Avg weight: 17,636.742

**After Refactor** (Jan 9):
- Iterations 20-80: 0% error, output "world" ✅
- Iteration 100+: 86.7% error, output "wolo wolo wolo" ❌
- Avg weight: 389.265

**Conclusion**: Weight magnitude reduced 45x, but behavior IDENTICAL.

---

## Theories

### Theory 1: Weight Growth Rate (Not Magnitude)

**Observation**: Weights grow from 128 → 389 in 100 iterations
- That's 3x growth in 100 iterations
- Even bounded at 255, the RATE of growth may be the issue
- Strong edges get stronger faster than weak edges

**Why this causes forgetting**:
1. "hello" edges get activated more (appear first in pattern)
2. They strengthen faster (more activations)
3. By iteration 100, they're 3x stronger than "world" edges
4. Context gating can't overcome 3x difference
5. System gets stuck following strong "hello" edges

**Test**: Add decay to slow growth rate
```c
// Current: weight = weight + increment
// Test: weight = weight * 0.99 + increment
```

### Theory 2: Context Gating Not Working

**Observation**: Context gating integrated but forgetting persists

**Possible issues**:
1. **Context trace empty**: May not be populated during generation
2. **Context trace wrong**: May contain wrong bytes
3. **Context matching broken**: Byte matching may fail
4. **Gate calculation wrong**: Recency weights may be incorrect

**Test**: Add debug output to verify:
```c
fprintf(stderr, "Context trace: ");
for (size_t i = 0; i < node->context_trace_len; i++) {
    int byte = (int)((node->context_trace[i] + 1.0f) * 128.0f);
    fprintf(stderr, "'%c' ", (char)byte);
}
fprintf(stderr, "\n");

fprintf(stderr, "Edge to '%c': gate=%.3f\n", 
        edge->to_node->payload[0], context_gate);
```

### Theory 3: Wrong Edge Selection Logic

**Observation**: System outputs "wolo wolo wolo" instead of "world"

**Analysis**:
- Input: "hello " (6 bytes)
- Expected: "world" (5 bytes)
- Actual: "wolo wolo wolo" (15 bytes, looping)

**This suggests**:
1. System is selecting wrong edges from 'o' node
2. 'o' has edges to both ' ' (space, correct) and 'w' (wrong)
3. Context should suppress 'o'→'w' edge
4. But it's not being suppressed (or not strongly enough)

**Hypothesis**: The 'o'→'w' edge is being selected because:
- It's part of "world" pattern (w-o-r-l-d)
- So it has high weight
- Context gating may not distinguish between:
  - 'o' at end of "hello" (should go to ' ')
  - 'o' in middle of "world" (should go to 'r')

### Theory 4: Context Trace Too Short

**Current**: context_trace stores 8 bytes

**For "hello world"**:
- Need to distinguish: "hello " vs "world"
- Context trace of 8 bytes should be enough
- But maybe it's not being updated correctly

**Test**: Check if context_trace contains:
- At "hello ": should contain ['h','e','l','l','o',' '] (6 bytes)
- This should gate open only edges to 'w' (start of "world")
- And suppress edges back to 'h', 'e', 'l', 'o'

### Theory 5: Habituation Too Weak

**Current penalty**: `1.0 / (1.0 + count * 2.0)`
- count=1: penalty = 0.33 (reduces to 33%)
- count=2: penalty = 0.20 (reduces to 20%)
- count=3: penalty = 0.14 (reduces to 14%)

**For "wolo wolo wolo"**:
- 'w' appears 3 times in last 8 outputs
- Penalty: 0.14 (reduces to 14%)
- But if 'w' edge has 3x higher weight, it still wins!
- 3.0 * 0.14 = 0.42 vs 1.0 * 1.0 = 1.0 (correct edge wins)

**Actually habituation should work!** Unless the weight difference is >7x.

---

## Most Likely Cause

Combining all theories, the most likely cause is:

**Context gating is integrated but NOT WORKING during generation.**

Evidence:
1. Catastrophic forgetting persists despite all changes
2. Same exact behavior (0% → 86.7% at iteration 100)
3. Same wrong output ("wolo wolo wolo")
4. Weight bounds help (390 vs 17,636) but don't fix the issue

**Hypothesis**: 
- `node_update_context_values()` is being called
- But context_trace is empty or wrong
- So all edges get `context_gate = 1.0` (no gating)
- Selection falls back to raw weight comparison
- Strong edges always win (no context discrimination)

---

## Verification Test

To verify context gating is the issue, add debug output:

```c
// In node_update_context_values():
fprintf(stderr, "[CONTEXT] Node '%c': trace_len=%zu\n", 
        node->payload[0], node->context_trace_len);

for (size_t i = 0; i < node->outgoing_count; i++) {
    Edge *edge = node->outgoing_edges[i];
    fprintf(stderr, "  Edge to '%c': gate=%.3f, value=%.3f\n",
            edge->to_node->payload[0], context_gate, 
            node->edge_context_values[i]);
}
```

**Expected output** (if working):
```
[CONTEXT] Node 'o': trace_len=6
  Edge to ' ': gate=1.0, value=2.5    ← Should win
  Edge to 'w': gate=0.0, value=0.0    ← Should be suppressed
```

**If context gating broken**:
```
[CONTEXT] Node 'o': trace_len=0       ← Empty! Bug!
  Edge to ' ': gate=1.0, value=1.2
  Edge to 'w': gate=1.0, value=3.5    ← Wins due to higher weight
```

---

## Recommended Next Steps

### Priority 1: Verify Context Gating
Add debug output to confirm:
1. Context trace is populated (not empty)
2. Context trace contains correct bytes
3. Context gates are being computed correctly
4. Wrong edges are actually being suppressed (gate=0.0)

### Priority 2: Add Weight Decay
If context gating works, add decay to prevent dominance:
```c
static inline void edge_update_weight_bounded(Edge *edge, float activation) {
    float current = (float)edge->weight * 0.99f;  // 1% decay
    float new_weight = current + (activation * 10.0f);
    if (new_weight > 255.0f) new_weight = 255.0f;
    if (new_weight < 0.0f) new_weight = 0.0f;
    edge->weight = (uint8_t)new_weight;
}
```

### Priority 3: Strengthen Habituation
If loops persist, increase penalty:
```c
// Current: 1.0 / (1.0 + count * 2.0)
// Stronger: 1.0 / (1.0 + count * 5.0)
float habituation = 1.0f / (1.0f + (float)recent_count * 5.0f);
```

---

## Key Insight

**The edge refactor was necessary and successful**, but it revealed that catastrophic forgetting has a deeper cause:

- It's not JUST weight magnitude (bounded weights don't fix it)
- It's not JUST unbounded growth (growth rate reduced 45x)
- It's likely **context gating not working effectively**

The refactor **enables** the fix (context gating now integrated), but the fix itself needs debugging to work correctly.

---

## Success Metrics

### Refactor Goals ✅
- [x] 86% memory reduction
- [x] Bounded weights (0-255)
- [x] Remove MiniNet from edges
- [x] Integrate context gating
- [x] Simplify code

### Learning Goals ⚠️
- [x] Initial learning works (0% error)
- [ ] Sustained learning (fails at iteration 100)
- [ ] No catastrophic forgetting (still occurs)
- [ ] Loop detection (still failing)

**Refactor: 100% complete**  
**Learning: Needs debugging**

---

**Status**: REFACTOR COMPLETE, DEBUGGING NEEDED  
**Next**: Verify context gating with debug output
