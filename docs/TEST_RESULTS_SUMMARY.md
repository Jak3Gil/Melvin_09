# Test Results Summary

## Current System State
- **Implementation:** Passive context (no context tag storage)
- **Edge Size:** 48 bytes
- **Memory Savings:** 90-96% vs stored context
- **Date:** Post-passive context refactor

---

## Quick Stats

| Test | Accuracy | Status |
|------|----------|--------|
| test_associations | 0% | ❌ Gibberish |
| test_simple | 0% | ❌ Garbage |
| test_different_inputs | 20% | ⚠️ Single-char only |
| test_overlapping_sequences | 34.6% | ⚠️ Single-char only |
| test_two_bytes | 25% | ⚠️ Partial |
| **Overall Average** | **~16%** | ❌ **Needs improvement** |

---

## Key Findings

### ✅ What Works:
1. **Graph structure:** Edges form correctly (36:1 ratio)
2. **Single-character inputs:** 60-100% accuracy
3. **Simple sequences:** a→bc, b→cd work perfectly
4. **Memory efficiency:** 48 bytes per edge (vs 464-1,296)

### ❌ What Fails:
1. **Multi-character inputs:** 0-20% accuracy
2. **Context-dependent decisions:** Can't differentiate
3. **Longer sequences:** Gibberish after 1-2 bytes
4. **Overall accuracy:** Only 16% (unacceptable)

---

## Root Cause

**Passive context requires target pre-activation, but targets aren't activated:**

```
Input: "hello" → predict next

Current (1 pass):
- Wave activates: [h,e,l,l,o]
- At node 'o', choosing next edge
- Target 'w': activation = 0.0 (not reached yet!)
- Target 'x': activation = 0.0 (not reached yet!)
- No context signal → random selection → garbage

Needed (3-5 passes):
- Pass 1: Activate [h,e,l,l,o]
- Pass 2: Spread to neighbors (including 'w')
- Pass 3: 'w' now has activation = 0.6
- At node 'o', choosing next edge
- Target 'w': activation = 0.6 ✅
- Target 'x': activation = 0.1
- Clear winner → correct selection
```

---

## Recommendation

**Implement adaptive multi-pass wave propagation:**
1. Multiple passes (3-5) before decision
2. Mini-transformer attention during spread
3. Mini-net confidence evaluation
4. Adaptive thresholds

**Expected result:**
- Memory: Still 48 bytes ✅
- Accuracy: 40-60% (vs current 16%) ✅
- Compute: 3-10x more (acceptable) ✅

See `ADAPTIVE_COMPUTE_DESIGN.md` for full implementation details.
