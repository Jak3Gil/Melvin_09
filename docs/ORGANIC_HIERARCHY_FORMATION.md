# Organic Multi-Level Hierarchy Formation

## Overview

Hierarchies form naturally from repeated patterns, compressing knowledge and saving compute. The system now creates multi-level hierarchies organically through pure competition, not hardcoded rules.

## How It Works

### Two-Phase Process

**Phase 1: Input Sequence Processing**
- Processes edges between consecutive input bytes
- Creates 2-byte hierarchies: "he", "el", "ll", "lo"
- Location: `src/melvin.c` lines 11439-11592

**Phase 2: Activation Pattern Processing (NEW)**
- Processes edges between ALL activated nodes (including hierarchies!)
- Creates 3+ byte hierarchies: "hel", "ell", "llo", "hello"
- Location: `src/melvin.c` lines 11594-11631

### The Organic Growth

**Iteration 1: "hello"**
```
Input: [h, e, l, l, o] (bytes)
Wave activates: [h, e, l, l, o]
Phase 1 creates: "he", "el", "ll", "lo" (2-byte hierarchies)
Phase 2 checks: h→e, e→l, l→l, l→o (same as Phase 1)
```

**Iteration 2: "hello"**
```
Input: [h, e, l, l, o] (bytes)
Wave activates: [h, e, l, l, o, "he", "el", "ll", "lo"] (includes hierarchies!)
Phase 1 creates: More 2-byte hierarchies (if needed)
Phase 2 checks: "he"→l, "el"→l, "ll"→o, "he"→"lo" (hierarchy + byte/hierarchy!)
Creates: "hel", "ell", "llo", "hello" (3+ byte hierarchies!)
```

**Iteration 3: "hello"**
```
Wave activates: [h, e, l, l, o, "he", "el", "ll", "lo", "hel", "ell", "llo", "hello"]
Phase 2 checks: "hel"→"lo" → creates "hello" (if not already exists)
Creates: Full pattern hierarchy!
```

## The Competition Mechanism

**Pure Competition (No Hardcoded Thresholds):**
```c
// Edge strength relative to local average
float relative_strength = edge->weight / local_avg_weight;

// If edge is stronger than average, form hierarchy
if (relative_strength > 1.0f) {
    create_hierarchy(from, to);
}
```

**What This Means:**
- Repeated patterns → stronger edges → hierarchies form automatically
- More training → stronger edges → larger hierarchies
- No threshold needed - pure competition

## Compute Savings

**Before (no hierarchies):**
```
Input "hello" → traverse: h→e→l→l→o (5 nodes, 4 edges)
Every time: 5 node lookups, 4 edge traversals
```

**After (with "hello" hierarchy):**
```
Input "hello" → match hierarchy "hello" (1 node lookup!)
One node represents 5 bytes = 5x compute savings
```

**The compression emerges naturally:**
- Strong edges = repeated patterns = hierarchy candidates
- Co-activation = nodes fire together = hierarchy formation
- Competition = strong edges win = natural selection

## Implementation Details

### Helper Function: `check_and_form_hierarchy()`

**Location:** `src/melvin.c` lines 10311-10373

**What it does:**
1. Checks if edge is strong enough (relative_strength > 1.0)
2. Checks if hierarchy already exists (trie lookup)
3. Creates hierarchy if needed
4. Handles reference hierarchies (reconstructs payloads)

**Works for:**
- (byte + byte) → 2-byte hierarchy
- (hierarchy + byte) → 3+ byte hierarchy
- (hierarchy + hierarchy) → larger hierarchy

### Phase 2: Activation Pattern Processing

**Location:** `src/melvin.c` lines 11594-11631

**Key Features:**
- Processes ALL activated nodes (not just input)
- Includes hierarchies in activation pattern
- Co-activation check: Both nodes must be activated
- Creates multi-level hierarchies organically

## Benefits

1. **Organic Growth:** Hierarchies form naturally from repeated patterns
2. **Multi-Level:** Creates 2-byte, 3-byte, 4-byte, 5-byte hierarchies automatically
3. **Compute Savings:** Larger hierarchies = fewer traversals = faster generation
4. **No Fallbacks:** Pure competition, no hardcoded thresholds
5. **Data-Driven:** Everything emerges from the data
6. **Memory Efficient:** Hierarchies compress patterns, reducing node count

## Requirements Compliance

**Requirement.md Line 19-25:**
- ✅ Hierarchies form naturally from repeated patterns
- ✅ Pure competition: edges stronger than local average form hierarchies
- ✅ No hardcoded thresholds: more repetitions → stronger edge → hierarchy forms automatically
- ✅ Self-growing: patterns that repeat often naturally consolidate
- ✅ Recursive: hierarchies can combine into higher-level hierarchies
- ✅ Data-driven: all formation decisions based on edge strength relative to local context

## Testing

**Expected Results:**
- After 10 iterations of "hello": Should have "he", "el", "ll", "lo" (2-byte)
- After 20 iterations: Should have "hel", "ell", "llo" (3-byte)
- After 50 iterations: Should have "hell", "ello" (4-byte)
- After 100 iterations: Should have "hello" (5-byte)

**Multi-character accuracy should improve:**
- Input "hel" → finds "hello" hierarchy → outputs "lo" ✓
- Input "he" → finds "hello" hierarchy → outputs "llo" ✓

## The Fix

**Before (broken):**
- Only processed `initial_nodes` (input bytes)
- Never checked edges between hierarchies
- Only created 2-byte hierarchies

**After (fixed):**
- Phase 1: Processes input sequence (2-byte hierarchies)
- Phase 2: Processes activation pattern (3+ byte hierarchies)
- Hierarchies naturally combine as patterns repeat

**No fallbacks needed - pure organic growth!**
