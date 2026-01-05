# Hierarchy Consolidation Implementation

## What Was Implemented

Added hierarchy consolidation as part of **wave propagation's structure decisions** (following README principle that all decisions go through wave propagation).

### Location
- `melvin.c` lines ~3341-3360: In `wave_propagate()` after hierarchy creation
- `melvin.c` lines ~5488-5500: In `melvin_m_process_input()` after hierarchy creation

### How It Works

When a higher-level hierarchy is created:
1. Check if component nodes are hierarchies (abstraction_level > 0)
2. If yes, accelerate their inactivity timers
3. Acceleration factor is data-driven: based on new hierarchy's abstraction level
4. Higher-level hierarchies = more compression = faster consolidation

```c
// DATA-DRIVEN: Acceleration factor based on new hierarchy's abstraction level
float consolidation_factor = (float)hierarchy->abstraction_level / 
                           ((float)hierarchy->abstraction_level + 1.0f);
float acceleration = 0.5f * (1.0f + consolidation_factor);  // Range: 0.5 to 1.0

// If component is a hierarchy, mark for faster decay
if (from->abstraction_level > 0) {
    from->inactivity_timer += acceleration;
}
if (to->abstraction_level > 0) {
    to->inactivity_timer += acceleration;
}
```

### Design Principles Followed

✓ **Part of wave propagation**: Consolidation happens as a structure decision, not separate logic
✓ **Local signals only**: Uses the nodes being combined (no O(n²) scan)
✓ **Data-driven**: Acceleration adapts to hierarchy level
✓ **Intelligent forgetting**: Aligns with README's "unused patterns decay naturally"

## Test Results

After 1000 iterations of "hello":
- Graph stabilizes at 58 nodes (after ~100 iterations)
- No consolidation detected (node count doesn't decrease)

## Why Consolidation Isn't Working Yet

### Root Cause
The sub-hierarchies are still being **activated** during input processing:
- Greedy longest-match might still match "he", "el", "ll", "lo" if "hello" doesn't exist yet
- Even if "hello" exists, if it's not being matched consistently, sub-hierarchies stay active
- Active nodes don't decay (inactivity_timer resets on activation)

### The Problem
Consolidation accelerates decay, but:
1. Nodes only self-destruct when **inactive** AND timer > threshold
2. If hierarchies are still being matched/activated, they never become inactive
3. Acceleration helps, but doesn't force removal

## Potential Solutions

### Option A: More Aggressive Consolidation
When a hierarchy is created, immediately check if component hierarchies are ONLY used for this hierarchy:
- If yes: Mark for immediate removal (not just faster decay)
- If no: Keep them (they're used elsewhere)

### Option B: Hierarchy-First Matching Enforcement
Ensure greedy longest-match ALWAYS prefers the longest hierarchy:
- If "hello" exists, never match "he" + "l" + "lo"
- This makes sub-hierarchies inactive naturally
- Consolidation then works as designed

### Option C: Explicit Redundancy Check
After creating a hierarchy, check if it subsumes other hierarchies:
- If "hello" contains "he", mark "he" as redundant
- More aggressive than just accelerating timers

## Current Status

✓ **Implementation complete**: Consolidation logic is in place
✓ **Part of wave propagation**: Follows README architecture
✓ **Data-driven**: Uses adaptive signals
✗ **Not yet effective**: Graph doesn't consolidate (58 nodes remain)

## Next Steps

1. **Debug why hierarchies stay active**: Check if "hello" is being matched consistently
2. **Consider Option B**: Ensure hierarchy-first matching always prefers longest hierarchy
3. **Or Option C**: Add explicit redundancy checking for subsumed hierarchies

The foundation is correct - consolidation is part of wave propagation's structure decisions. The issue is that hierarchies aren't becoming inactive, so they can't be pruned.

