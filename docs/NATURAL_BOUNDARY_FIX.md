# Natural Boundary Detection Fix

## Summary
Replaced artificial loop detection with natural boundary detection. The system now stops generating output when prediction confidence drops, like how the brain naturally runs out of strong predictions when a thought/pattern completes.

## Problem
Output was looping (e.g., "hello" → " worlllll" instead of " world"):
1. Complex loop detection algorithm was buggy and didn't catch simple repetitions
2. System had no way to naturally know when to stop
3. Hierarchies were making spurious predictions at pattern boundaries
4. Edges without context training (routing_gate=255) were being selected

## Solution

### 1. Natural Boundary Detection (replaces loop detection)
Instead of detecting loops, stop when prediction confidence drops:
- **Edge weight threshold**: If best edge weight < 60% for untrained edges (gate=255), stop
- **Context-trained threshold**: If best edge weight < 40% for trained edges (gate<255), stop  
- **Activation decay**: If next node activation < 5%, stop
- **Learned stop signal**: If node's stop_weight > 70%, stop

### 2. Context Window Alignment
Fixed mismatch between training and generation context computation:
- Both now use last 8 bytes of context for signature computation
- Prevents different signatures for same context

### 3. Hierarchy Context Matching
Fixed hierarchy matching to check END of context, not START:
- Hierarchy "he" now only fires when context ENDS with "he"
- Prevents false matches when "he" appears early in context

### 4. Hierarchy Edge Filtering
Hierarchies now skip edges without context training:
- routing_gate=255 edges are ignored in hierarchy predictions
- Only context-trained edges can be selected

### 5. Cycle Detection (backup)
Added rolling window cycle detection as final safety:
- Track last 16 visited nodes
- If same node visited twice, stop generation

## Test Results

```
Test 1 (Association Simple):
  Input: "hello"
  Output: " world" (6 bytes) ✓

Test 2 (Association Multi):
  Input: "cat" → Output contains 'm' (from 'meow') ✓
  Input: "dog" → Output contains 'b' (from 'bark') ✓

Test 3 (Disambiguation):
  cat/dog produce distinct outputs ✓
```

## Key Insight
The brain doesn't need a "loop detector" because it naturally runs out of strong predictions when a thought/pattern completes. The system now does the same - stops when confidence/activation drops, not when we detect we're looping.
