# Root Cause Summary: Why Nodes Are Failing

## What We Discovered

### Test Results
- Training: 200 iterations of "hello"
- Final graph: 18 nodes, 71 edges
- Hierarchies found: **ONLY 1** - "lo" (2 bytes)
- Expected hierarchies: "he", "el", "ll", "lo", "hel", "ell", "llo", "hello"

### The Fundamental Problem

**The system is NOT forming multi-level hierarchies.**

After 200 repetitions of "hello", the system should have:
1. **Level 1**: 2-byte hierarchies ("he", "el", "ll", "lo")
2. **Level 2**: 3-byte hierarchies ("hel", "ell", "llo")
3. **Level 3**: 4-byte hierarchies ("hell", "ello")
4. **Level 4**: 5-byte hierarchy ("hello")

But it only has:
- **Level 1**: "lo" (1 hierarchy out of 4 expected)
- **Level 2+**: NONE

### Why This Breaks Everything

1. **Hierarchy guidance fails**: When we input "hel" and expect "lo", the system looks for a hierarchy that matches "hel" + output. But no such hierarchy exists.

2. **Stop probability not reduced**: Without hierarchy guidance, the stop probability isn't reduced by 90%, so it stops after 1 byte.

3. **Position-aware learning insufficient**: Without hierarchies to guide, the system falls back to position-aware edge selection, which picks wrong edges (e→e instead of l→o).

### Why Aren't Hierarchies Forming?

From `FINAL_DIAGNOSIS.md`:
> **Wave propagation only traverses edges between input (byte) nodes, not between existing hierarchy nodes.**

When processing "hello":
- Wave starts at byte nodes: h, e, l, l, o
- Wave propagates through edges: h→e, e→l, l→l, l→o
- These are all byte-to-byte edges
- Hierarchy "lo" exists, but wave doesn't traverse through it
- So "ello" sequence is never seen as: e→l→(lo hierarchy)
- Instead it's seen as: e→l→l→o (all bytes)
- Higher-level hierarchies never form because wave never combines existing hierarchies

### The 18 Nodes

If we only have 5 byte nodes + 1 hierarchy ("lo"), where are the other 12 nodes?

Possibilities:
1. **Blank nodes**: Created for generalization
2. **Duplicate byte nodes**: Multiple nodes for same byte (shouldn't happen)
3. **Failed hierarchy attempts**: Nodes created but not marked as hierarchies
4. **Similarity nodes**: Created for pattern matching

## Our Fixes (Partial Success)

### Fix 1: Hierarchy-Aware Stop Probability ✓
- **Status**: Implemented correctly
- **Problem**: Can't help if hierarchies don't exist
- **Result**: No effect because no hierarchies found

### Fix 2: Ensure Hierarchy Edges Exist ✓
- **Status**: Implemented correctly
- **Problem**: Can't create edges if hierarchy doesn't exist
- **Result**: No effect because no hierarchies found

### Fix 3: Hierarchy Matching Bug Fix ✓
- **Status**: Fixed (`payload_size <= output_len` → `payload_size < output_len`)
- **Problem**: Doesn't matter if no hierarchies exist
- **Result**: No effect because only "lo" hierarchy exists

## The Real Solution

We need to fix **hierarchy formation**, not just hierarchy usage.

### Option A: Fix Wave Propagation
Make wave propagation traverse through hierarchy nodes, not just byte nodes.

**Challenge**: This is a fundamental architectural change to how wave propagation works.

### Option B: Post-Process Hierarchy Formation
After creating 2-byte hierarchies, run another pass to combine them into 3-byte, then 4-byte, etc.

**Challenge**: When? How often? Based on what signal?

### Option C: Accept Limitations
The system can only learn 2-byte patterns. Use position-aware learning and prediction error to make better decisions.

**Challenge**: Will it ever be truly intelligent without hierarchical abstraction?

## Immediate Next Steps

1. **Verify the 18 nodes**: What are the other 12 nodes if not hierarchies?
2. **Test with simpler patterns**: Does "aa" form a hierarchy after 200 iterations?
3. **Check hierarchy formation threshold**: Is `wave_compute_hierarchy_probability()` threshold too high?
4. **Consider Option B**: Implement recursive hierarchy formation as a post-processing step

## Conclusion

**Nodes are failing because hierarchies aren't forming.**

Our fixes for hierarchy-aware stop probability and ensuring hierarchy edges exist are correct, but they can't help if the hierarchies don't exist in the first place.

The root cause is that wave propagation doesn't traverse through existing hierarchy nodes, so multi-level hierarchies never form. This is the architectural limitation identified in `FINAL_DIAGNOSIS.md`.

To truly fix this, we need to either:
1. Change how wave propagation works (major refactor)
2. Add recursive hierarchy formation (new feature)
3. Make position-aware learning work without hierarchies (workaround)

