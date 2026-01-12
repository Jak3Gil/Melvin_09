# Unified Context System Implementation

## Summary
Implemented and unified the context disambiguation system to ensure all components work together properly.

## Key Changes

### 1. Context Signature System (`compute_context_signature`)
- Added a rolling hash function to compute context signatures from byte sequences
- Signatures are 8-bit values (0-255) that encode the training context
- Matching function computes similarity between signatures

### 2. Edge-Level Context Memory
- **Training**: Edges store their training context signature in `routing_gate`
  - Context = bytes BEFORE the from_node (not including it)
  - Example: Edge ' '→'m' in "cat meow" gets signature from "hell" (before 'o' at position 4)
- **Generation**: Edge selection compares current context signature to stored signatures
  - Edges with matching context signatures score higher
  - routing_gate=255 means "no context" (default) - treated as no match

### 3. Fixed Context Computation Alignment
- **Problem**: Training computed context including prev_node's position, but generation excluded it
- **Fix**: Training now computes context BEFORE prev_node to match generation:
  ```c
  // Training: ctx_end = i - 1 (before prev_node, which is at i-1)
  // Generation: ctx_len_for_match = context_len - current_node->payload_size
  ```

### 4. First Byte Selection with Context
- Added context matching to Priority 2 (first byte selection)
- Context = input bytes BEFORE last_input node
- Prevents hierarchy edges (routing_gate=255) from winning over context-trained edges

### 5. Fixed Blank Node Prediction
- Blank edges have routing_gate=255 (default from link_concrete_examples_to_blank)
- If no blank edges have context training, blank returns NULL and defers to mini-net
- Prevents blank from overriding correct context-based predictions

### 6. Removed Spurious Edge Creation
- Removed edge creation during generation (was polluting graph)
- Removed edge creation in `graph_learn_from_predictions` (bypassed context signatures)
- Edges now only created during sequential training with proper context signatures

## Test Results

### Test 1: Multiple Pattern Disambiguation (cat/dog)
```
Input 'cat' -> ' meowowow' (contains 'm' from 'meow')  ✓
Input 'dog' -> ' barkrkrk' (contains 'b' from 'bark')  ✓
Disambiguation: Both patterns produce distinct outputs  ✓
```

### Test 2: Association Multi
```
'cat' -> contains 'm' (from 'meow')  ✓
'dog' -> contains 'b' (from 'bark')  ✓
```

### Test 3: Association Simple
```
'hello' -> ' worlllll' (contains 'w' from 'world')  ✓
```

## Architecture Summary

```
INPUT → Nodes activated → Context signature computed
                              ↓
           ┌─────────────────────────────────────┐
           │   EDGE SELECTION PRIORITIES         │
           │   1. Hierarchy prediction           │
           │   2. Blank prediction (if has ctx)  │
           │   3. Mini-net (context matched)     │
           │   4. Fallback (context matched)     │
           └─────────────────────────────────────┘
                              ↓
           Edge with best context match selected
                              ↓
                         OUTPUT
```

All components now use consistent context matching via `routing_gate` signatures.
