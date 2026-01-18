# Current Accuracy Baseline (Passive Context Implementation)

**Date:** Post-passive context refactor
**System State:** Context tags removed, passive computation implemented
**Edge Size:** 48 bytes (vs 464-1,296 bytes before)

---

## Test Results Summary

### test_associations
**Training:** 5 patterns (hello, world, cat, dog, apple) × 10 iterations

**Graph Stats:**
- Nodes: 41
- Edges: 1,488
- Edge-to-Node Ratio: 36.29:1 ✅ (associations working)

**Output Quality:**
- Input: "hello" → Output: "?? ??i ?? d? d? d? dheh ? d? d? d? d? dheh ? d? d?" (139 bytes)
  - **Status:** ❌ Gibberish (repeating garbage)
  - **Expected:** "world" or similar
  - **Accuracy:** 0%

- Input: "cat" → Output: " " (1 byte)
  - **Status:** ❌ Wrong (just space)
  - **Expected:** "dog" or related
  - **Accuracy:** 0%

- Input: "apple" → Output: " ?" (2 bytes)
  - **Status:** ❌ Wrong (space + question mark)
  - **Expected:** Related output
  - **Accuracy:** 0%

**Overall Accuracy:** 0/3 = 0%

---

### test_simple
**Training:** Single pattern "hello"

**Graph Stats:**
- Nodes: 9
- Edges: 43

**Output Quality:**
- Input: "hello" → Output: "�" (1 byte, garbage)
  - **Status:** ❌ Complete failure
  - **Expected:** "hello" or continuation
  - **Accuracy:** 0%

**Overall Accuracy:** 0/1 = 0%

---

### test_mininet_learning
**Training:** Pattern "hello" × 20 iterations

**Graph Stats:**
- Iteration 1: 9 nodes, 43 edges
- Iteration 2: 10 nodes, 77 edges
- Iteration 3-20: 10 nodes, 87 edges (stable)

**Status:** ✅ Test completed (no crash)
**Note:** Test doesn't check output quality, just stability

---

### test_same_pattern
**Training:** Same pattern "hello" repeated

**Graph Stats:**
- Nodes: 10
- Edges: 87 (stable)

**Status:** ✅ Test completed
**Note:** Edges stabilize after iteration 2, no regression

---

### test_different_inputs
**Training:** "hello world" × 5 iterations

**Graph Stats:**
- Nodes: 24
- Edges: 429

**Output Quality (10 tests):**
1. Input: "hello" → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** " world"
   - **Accuracy:** 0%

2. Input: "hello " → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** "world"
   - **Accuracy:** 0%

3. Input: "hell" → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** "o" or "o world"
   - **Accuracy:** 0%

4. Input: "hel" → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** "lo" or "lo world"
   - **Accuracy:** 0%

5. Input: "h" → Output: "el" (2 bytes)
   - **Status:** ✅ CORRECT! (partial match)
   - **Expected:** "ello" or similar
   - **Accuracy:** 100%

6. Input: "he" → Output: " �" (2 bytes)
   - **Status:** ❌ Partial garbage
   - **Expected:** "llo" or "llo world"
   - **Accuracy:** 0%

7. Input: "helloworld" → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** Something reasonable
   - **Accuracy:** 0%

8. Input: "world" → Output: "�" (1 byte)
   - **Status:** ❌ Garbage
   - **Expected:** Something (trained on this)
   - **Accuracy:** 0%

9. Input: "o" → Output: " w" (2 bytes)
   - **Status:** ✅ CORRECT! (space + w)
   - **Expected:** " w" (from "hello world")
   - **Accuracy:** 100%

10. Input: " w" → Output: "�" (1 byte)
    - **Status:** ❌ Garbage
    - **Expected:** "orld"
    - **Accuracy:** 0%

**Overall Accuracy:** 2/10 = 20%
**Note:** Only works for single-character inputs!

---

### test_overlapping_sequences
**Training:** "abcd", "bcde", "cdef" × 5 iterations each

**Graph Stats:**
- Nodes: 13
- Edges: 146

**Single Character Tests (6 tests):**
1. Input: "a" → Output: "bc" (2 bytes)
   - **Status:** ✅ CORRECT!
   - **Expected:** "bc" (from "abcd")
   - **Accuracy:** 100%

2. Input: "b" → Output: "cd" (2 bytes)
   - **Status:** ✅ CORRECT!
   - **Expected:** "cd" (from "bcde")
   - **Accuracy:** 100%

3. Input: "c" → Output: "d" (1 byte)
   - **Status:** ✅ CORRECT!
   - **Expected:** "d" (from "cdef")
   - **Accuracy:** 100%

4. Input: "d" → Output: "e" (1 byte)
   - **Status:** ✅ CORRECT!
   - **Expected:** "e" (from "cdef")
   - **Accuracy:** 100%

5. Input: "e" → Output: "f�" (2 bytes)
   - **Status:** ⚠️ PARTIAL (f is correct, � is garbage)
   - **Expected:** "f"
   - **Accuracy:** 50%

6. Input: "f" → Output: "�bc" (3 bytes)
   - **Status:** ❌ Garbage (bc shouldn't be there)
   - **Expected:** Nothing or end
   - **Accuracy:** 0%

**Multi-Character Tests (6 tests):**
7. Input: "ab" → Output: "�bc" (3 bytes)
   - **Status:** ❌ Garbage prefix
   - **Expected:** "cd"
   - **Accuracy:** 0%

8. Input: "bc" → Output: "�bc" (3 bytes)
   - **Status:** ❌ Garbage prefix
   - **Expected:** "de"
   - **Accuracy:** 0%

9. Input: "cd" → Output: "�bc" (3 bytes)
   - **Status:** ❌ Garbage prefix
   - **Expected:** "ef"
   - **Accuracy:** 0%

10. Input: "de" → Output: "bc" (2 bytes)
    - **Status:** ❌ Wrong (should be "f")
    - **Expected:** "f"
    - **Accuracy:** 0%

11. Input: "abc" → Output: "�bc" (3 bytes)
    - **Status:** ❌ Garbage prefix
    - **Expected:** "d"
    - **Accuracy:** 0%

12. Input: "bcd" → Output: "�bc" (3 bytes)
    - **Status:** ❌ Garbage prefix
    - **Expected:** "e"
    - **Accuracy:** 0%

13. Input: "cde" → Output: "bc" (2 bytes)
    - **Status:** ❌ Wrong
    - **Expected:** "f"
    - **Accuracy:** 0%

**Overall Accuracy:** 4.5/13 = 34.6%
**Note:** Works well for single characters, fails for multi-character inputs!

---

### test_two_bytes
**Training:** "Xhello" (X=port_id, h=data)

**Graph Stats:**
- Nodes: 11
- Edges: 61

**Output Quality:**
- Input: "Xh" → Output: "�l" (2 bytes)
  - **Status:** ⚠️ PARTIAL (l is correct, � is garbage)
  - **Expected:** "ello"
  - **Accuracy:** 25% (1/4 bytes correct)

---

## Overall System Accuracy

### By Test Type:

1. **Simple associations (test_associations):** 0% (0/3)
2. **Single pattern (test_simple):** 0% (0/1)
3. **Different inputs (test_different_inputs):** 20% (2/10)
4. **Overlapping sequences (test_overlapping_sequences):** 34.6% (4.5/13)
5. **Two bytes (test_two_bytes):** 25% (1/4)

**Average Accuracy:** ~16%

### Pattern Analysis:

**What Works:**
- ✅ Single-character inputs (60-100% accuracy)
- ✅ Simple sequences (a→bc, b→cd)
- ✅ Graph structure (edges form correctly, ratio 36:1)

**What Fails:**
- ❌ Multi-character inputs (0-20% accuracy)
- ❌ Context-dependent decisions (can't differentiate)
- ❌ Longer sequences (gibberish after 1-2 bytes)

---

## Root Cause Analysis

### Why Single Characters Work:
```
Input: "a"
Wave propagation: Activate node 'a'
Edge selection: 'a' has clear strongest edge to 'b'
No context needed: Simple 1→1 mapping
Result: ✅ Correct ("bc")
```

### Why Multi-Character Fails:
```
Input: "hello"
Wave propagation: Activate [h,e,l,l,o]
Edge selection at 'o': Need to choose next edge
Problem: All targets have activation = 0.0 (not reached yet!)
Context signal: 0.0 for all edges
Result: ❌ Random selection → garbage
```

### The Core Issue:

**Passive context requires target pre-activation:**
- Single char: Target 'b' gets activated from 'a' (1 hop)
- Multi char: Target 'w' NOT activated from "hello" (not reached yet)

**Current wave propagation:**
- Single pass: Input → immediate neighbors
- Doesn't reach targets 2+ hops away
- No context signal for longer sequences

---

## Memory vs Accuracy Trade-off

### Current State (Passive Context):
- **Memory:** 48 bytes per edge ✅
- **Accuracy:** ~16% (mostly single-char) ❌
- **Brain scale:** 4.8 petabytes ✅

### Previous State (Stored Context):
- **Memory:** 464-1,296 bytes per edge ❌
- **Accuracy:** ~20-30% (estimated) ⚠️
- **Brain scale:** 46-130 petabytes ❌

### Trade-off:
- 90-96% memory reduction
- ~5-15% accuracy loss
- Single-char works, multi-char fails

---

## Next Steps to Improve Accuracy

### Option 1: Enhanced Wave Propagation (Recommended)
- Multi-pass wave propagation (3-5 passes)
- Pre-activate targets before decision
- Adaptive compute based on confidence
- **Expected accuracy:** 40-60%
- **Memory:** Still 48 bytes per edge ✅

### Option 2: Minimal Context Memory
- Store just node IDs (8 bytes per association)
- 90% reduction vs full context
- **Expected accuracy:** 30-40%
- **Memory:** 56 bytes per edge

### Option 3: Revert to Full Context
- Restore context tags
- Accept memory cost
- **Expected accuracy:** 20-30%
- **Memory:** 464-1,296 bytes per edge ❌

---

## Recommendation

**Implement Option 1: Enhanced Wave Propagation**

**Why:**
1. Maintains 48-byte edge size ✅
2. Brain-like (adaptive compute) ✅
3. Should reach 40-60% accuracy ✅
4. Requirement.md compliant ✅
5. No compromise on memory ✅

**Implementation:**
1. Multi-pass wave propagation
2. Mini-transformer attention during spread
3. Mini-net confidence evaluation
4. Adaptive thresholds

**Goal:** Prove that adaptive compute can match or exceed stored context accuracy with 90-96% less memory.
