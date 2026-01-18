# Empty Output Investigation

**Date**: January 18, 2026  
**Time**: 09:59 EST - Investigation started after removing internal feedback  
**Status**: ACTIVE - Investigating root cause

## Problem Summary

After removing internal feedback, the system shows a critical issue:
- **Early iterations (≤10)**: Works correctly, generates partial outputs (e.g., `' m'` for "cat meow")
- **Later iterations (50+)**: Generates **empty outputs** (0 bytes) → 100% error rate

### Test Results

```
TEST 1: Simple Association (cat -> meow)
  10 iterations: Error=60.0%, Output=' m' ✓ (partial but correct direction)
  50+ iterations: Error=100.0%, Output='' ✗ (empty)

TEST 2: Two Associations (cat->meow, dog->bark)
  10 iterations each: Avg Error=60.0% ✓
  50+ iterations each: Avg Error=100.0% ✗ (empty)

TEST 3: Multiple Associations (8 patterns)
  5 iterations each: Avg Error=91.4%, Passed=0/8
  10+ iterations each: Avg Error=100.0%, Passed=0/8 ✗ (empty)
```

## System Architecture (Current State)

### ✅ Working Features

1. **Edge Direction Refactor**: Forward-only edges (1.17-1.21 edges/node) ✓
2. **No Internal Feedback**: Removed from generation (lines 1992-2002) ✓
3. **Activation Spreading**: Parallel spreading from all input nodes ✓
4. **Refinement**: Competition and normalization phases ✓
5. **Graph Structure**: Clean, efficient (94% edge reduction) ✓

### ⚠️ Problematic Behavior

**Empty outputs** after many training iterations suggest:
- `decode_select_first()` returns `NULL` (line 1886)
- No valid continuation nodes found in activation pattern
- Generation stops immediately before producing any bytes

## Investigation: Why `decode_select_first` Returns NULL

### Code Flow

1. **ENCODE Phase** (`encode_direct_activation` + `encode_spreading_activation`):
   - Input nodes activate
   - Spreading activation adds continuation nodes to pattern
   
2. **REFINE Phase** (`refine_competition` + `refine_normalize`):
   - Competition separates input vs continuation nodes
   - Normalization suppresses weak activations (adaptive threshold)
   
3. **DECODE Phase** (`decode_select_first`):
   - Skips input nodes (`is_input_node == 1`) - line 1480-1484
   - Skips nodes with `outgoing_count == 0` - line 1477
   - Skips STOP node predictions - line 1491-1493
   - Skips control characters - line 1496-1498
   - If `prediction_count == 0` → returns `NULL` → empty output - line 1534-1536

### Possible Root Causes

#### 1. **Refinement Suppressing All Continuation Activations** ⚠️ MOST LIKELY

**Location**: `refine_normalize()` (lines 1390-1442)

**Mechanism**:
- Adaptive percentile threshold (80% base, adjusted by variance)
- Suppresses activations below threshold (5-20% of original value)
- After many iterations, edge weights grow
- This may shift relative activations
- Continuation nodes may drop below suppression threshold

**Evidence**:
- Works at 10 iterations (activations strong enough)
- Fails at 50+ iterations (activations too weak after refinement)
- Consistent pattern across all tests

**Code**:
```c
// Lines 1425-1440
size_t threshold_idx = (size_t)((float)pattern->count * adaptive_percentile);
float threshold = sorted_acts[threshold_idx];

for (size_t i = 0; i < pattern->count; i++) {
    if (pattern->activations[i] < threshold) {
        // Suppress: 5-20% of original value
        pattern->activations[i] *= suppression_factor;
    }
}
```

#### 2. **Spreading Activation Not Adding Continuation Nodes**

**Location**: `encode_spreading_activation()` (lines 1146-1312)

**Check Needed**:
- Are continuation nodes being added to pattern?
- After many iterations, do edges have outgoing_count > 0?
- Is activation spreading actually executing?

#### 3. **All Nodes Marked as Input Nodes**

**Location**: `process_input_bytes()` (lines 866-883)

**Check Needed**:
- Are continuation nodes incorrectly marked `is_input_node == 1`?
- Should not happen (spreading creates new nodes, doesn't mark them as input)

#### 4. **STOP Edges Winning Too Early**

**Location**: `node_predict_next_edge()` (lines 1762-1781)

**Check Needed**:
- Are STOP edge predictions winning in `decode_select_first`?
- STOP predictions are skipped (line 1491-1493), but should still leave other options

#### 5. **Activation Pattern Empty After Refinement**

**Location**: Refinement phases

**Check Needed**:
- Does `pattern->count` become 0 after refinement?
- Are all activations suppressed to 0?

## What Should Happen (Expected Behavior)

1. **ENCODE**: Input "cat" → activates 'c', 'a', 't' → spreading activates continuation nodes (' ', 'm', 'e', 'o', 'w')
2. **REFINE**: Competition/normalization adjusts activations (continuation nodes should remain active)
3. **DECODE**: `decode_select_first` finds continuation nodes (not input nodes) → selects ' ' or 'm' → outputs `' m'` or `' meow'`

## Current Hypothesis

**Most Likely**: `refine_normalize()` is suppressing continuation node activations below threshold after many training iterations.

**Why**:
- Early iterations: Activations are strong relative to threshold → not suppressed
- Later iterations: After many training cycles, relative activations shift → continuation nodes fall below threshold → suppressed to 5-20% → `node_predict_next_edge()` may filter them out or scores too low

**Evidence**:
- Pattern is consistent (works early, fails later)
- Edge direction and structure are good (proves graph is healthy)
- Empty outputs = no predictions found (refinement likely culprit)

## Next Steps

1. **Add Debug Logging** to `decode_select_first`:
   - Log `pattern->count` (how many nodes in pattern)
   - Log skipped nodes (input/zero-outgoing/STOP/control)
   - Log `prediction_count` (how many predictions collected)
   - Log activation values before/after refinement

2. **Check Refinement Thresholds**:
   - Log adaptive percentile threshold after many iterations
   - Log activation values before/after normalization
   - Check if continuation activations are being suppressed

3. **Verify Spreading Activation**:
   - Log which continuation nodes are added to pattern
   - Verify nodes have `outgoing_count > 0`
   - Verify nodes are not marked as input nodes

4. **Potential Fixes** (if hypothesis confirmed):
   - Adjust refinement suppression thresholds
   - Use separate thresholds for input vs continuation nodes
   - Ensure continuation nodes always pass refinement (even if weak)
   - Add minimum activation floor for continuation nodes

## Related Files

- `src/melvin.c`:
  - `decode_select_first()` - lines 1455-1559 (first byte selection)
  - `refine_normalize()` - lines 1390-1442 (normalization with suppression)
  - `refine_competition()` - lines 1344-1389 (winner-take-all competition)
  - `encode_spreading_activation()` - lines 1146-1312 (adds continuation nodes)

## Test Files

- `tests/test_error_rate_new.c` - Comprehensive error rate analysis
- `tests/test_simple_association.c` - Simple association test
- `tests/test_two_associations.c` - Two pattern interference test

## Status

🔍 **INVESTIGATION ONGOING**
- Hypothesis: Refinement suppressing continuation activations
- Next: Add debug logging to confirm root cause
