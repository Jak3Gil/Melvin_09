# Node Failure Fixes - Implementation Plan

## Root Cause
Stop probability is stopping output generation too early because it doesn't know we're inside a learned hierarchy sequence.

## Fixes to Implement

### Fix 1: Hierarchy-Aware Stop Probability (CRITICAL)
- Location: `wave_compute_stop_probability()` in melvin.c
- Change: Check if we're inside a hierarchy before computing stop signals
- If inside hierarchy, reduce all stop signals by 90% (multiply by 0.1)
- Rationale: Don't stop in the middle of a learned pattern

### Fix 2: Ensure Hierarchy Edges Exist
- Location: `node_compute_winning_edge_with_context()` in melvin.c
- Change: If hierarchy says "output X" but no edge exists, create it
- Rationale: Hierarchies represent learned patterns - edges should exist

### Fix 3: Sequence Completion Signal
- Location: New function `compute_sequence_completion_signal()`
- Change: Add signal that checks if we're completing a learned pattern
- Integrate into stop probability calculation
- Rationale: System should know when it's successfully completing vs. lost

## Implementation Order
1. Fix 1 (critical - stops early termination)
2. Fix 2 (ensures hierarchy guidance works)
3. Fix 3 (adds feedback about pattern completion)

## Expected Results
- Input: "hel" → Output: "lo" (completes "hello")
- Error rate decreases over training
- Hierarchies guide output generation correctly

