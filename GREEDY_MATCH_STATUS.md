# Greedy Longest-Match Implementation Status

## What Was Implemented

Changed `graph_process_sequential_patterns()` to use **greedy longest-match** instead of byte-by-byte processing:

1. **Try to match longest hierarchy first**: For each position in input, try decreasing pattern lengths
2. **Prefer hierarchies**: If a hierarchy node matches, use it
3. **Advance by matched length**: Skip forward by the size of the matched pattern
4. **Create edges between all node types**: Edges can now connect byte→hierarchy, hierarchy→byte, hierarchy→hierarchy

## Results

### Positive:
- ✓ Hierarchies ARE being created: "he", "el", "ll", "lo"
- ✓ Hierarchies ARE being matched and used in subsequent iterations
- ✓ Edges ARE being created between different node types (e.g., h→(el), (el)→(lo))
- ✓ Graph grows from 5 nodes (bytes only) to 14 nodes (bytes + hierarchies)

### Problem:
- ✗ Only **2-byte hierarchies** are created (level 1)
- ✗ No **multi-level hierarchies** (level 2+) like "hel", "ello", "hello"
- ✗ Graph stops growing after ~50 iterations, stays at 14 nodes even after 1000 iterations
- ✗ Output generation still fails

## Debug Output Analysis

### Hierarchy Creation (first 50 iterations):
```
[DEBUG] Creating hierarchy: prob=0.685, threshold=0.300, from='l' + 'o'
[DEBUG] Creating hierarchy: prob=0.700, threshold=0.306, from='e' + 'l'
[DEBUG] Creating hierarchy: prob=0.731, threshold=0.318, from='h' + 'e'
[DEBUG] Creating hierarchy: prob=0.772, threshold=0.334, from='l' + 'o' (duplicate!)
[DEBUG] Creating hierarchy: prob=0.788, threshold=0.353, from='l' + 'l'
```

### Hierarchy Matching (after hierarchies exist):
```
[DEBUG] Matched hierarchy at pos 3: 'lo' (level 1, 2 bytes)
[DEBUG] Matched hierarchy at pos 1: 'el' (level 1, 2 bytes)
[DEBUG] Matched hierarchy at pos 0: 'he' (level 1, 2 bytes)
```

### Parsing Pattern:
Input "hello" is parsed as:
- Iteration 1-10: h + e + l + l + o (all bytes)
- Iteration 11+: (he) + l + (lo) OR h + (el) + l + o OR h + e + (ll) + o (varies)

## Why Multi-Level Hierarchies Aren't Forming

### Expected Behavior:
After creating "he" and "el" hierarchies, the next iterations should:
1. Parse "hello" as: (he) + l + (lo)
2. Create edge: (he)→l
3. Wave propagation sees (he)→l edge
4. Computes hierarchy probability for (he)→l
5. If probability > threshold, creates "hel" hierarchy (3 bytes, level 2)

### Actual Behavior:
- Edges ARE being created: (he)→l, l→(lo), etc.
- But wave propagation is NOT creating higher-level hierarchies from them
- Hierarchy probability might be too low
- OR the signals (dominance, repetition, maturity, compression) aren't strong enough

### Possible Root Causes:

#### 1. Hierarchy Probability Threshold Too High
- Initial threshold: 0.300
- Threshold adapts upward: 0.306, 0.318, 0.334, 0.353...
- By iteration 50, threshold might be too high for level-2 hierarchies

#### 2. Mixed Abstraction Levels Reduce Signals
- Edge (he)→l connects level-1 hierarchy to level-0 byte
- Signals might be computed differently for mixed-level edges
- Dominance, repetition, maturity might be lower

#### 3. Parsing Variability
- "hello" is sometimes parsed as (he) + l + (lo)
- Sometimes as h + (el) + l + o
- Sometimes as h + e + (ll) + o
- This variability might prevent any single edge from dominating enough to trigger hierarchy creation

#### 4. Duplicate Hierarchies
- "lo" hierarchy was created twice
- This suggests the duplicate detection isn't working
- Might be wasting slots in the graph

## Next Steps

### Option A: Lower Hierarchy Threshold for Higher Levels
Make the threshold adaptive based on abstraction level:
- Level 1 (byte→byte): threshold = 0.3
- Level 2 (mixed or hier→hier): threshold = 0.2
- Level 3+: threshold = 0.1

### Option B: Boost Signals for Higher-Level Hierarchies
When computing hierarchy probability, boost signals if:
- Either node has abstraction_level > 0
- This encourages recursive composition

### Option C: Force Consistent Parsing
Instead of greedy longest-match, use a consistent strategy:
- Always prefer hierarchies at the same position
- This ensures the same edges get strengthened repeatedly

### Option D: Debug Hierarchy Probability Calculation
Add logging to see what signals are being computed for mixed-level edges:
- What is the dominance signal for (he)→l?
- What is the repetition signal?
- Why isn't it reaching the threshold?

## Recommendation

Start with **Option D**: Add detailed logging to understand why hierarchy probability is too low for level-2 hierarchies. Then implement Option A or B based on what we learn.

The greedy longest-match implementation is working correctly - hierarchies are being matched and used. The problem is in the hierarchy formation logic, not the parsing logic.

