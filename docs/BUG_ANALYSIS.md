# Bug Analysis: Multi-Pattern Crash

## Problem
System crashes when processing a second different pattern after successfully processing the first pattern.

## Symptoms
- ✅ Single pattern works fine (tested 20 iterations)
- ✅ Same pattern repeated works (graph grows 4→23 nodes)
- ✗ Second different pattern crashes (segfault in `melvin_m_process_input`)

## Investigation

### Crash Location
```
EXC_BAD_ACCESS (code=1, address=0x136200000)
frame #0: melvin_m_process_input + 9384
```

This is a memory access violation - trying to read from invalid memory.

### Hypothesis 1: Dangling Pointer in best_edge Cache
**Status**: PARTIALLY FIXED

Added cache invalidation when edges are removed:
```c
// In node_remove_edge_from_list()
if (is_outgoing && node->best_edge == edge) {
    node->best_edge = NULL;
    node->best_edge_value = -1.0f;
    node->context_generation = 0;
}
```

Added safety check in `node_compute_winning_edge()` to verify edge is still in list.

**Result**: Still crashes

### Hypothesis 2: edge_context_values Array Size Mismatch
**Status**: FIXED

Problem: Array allocated with `outgoing_count` but accessed with indices up to `outgoing_capacity`.

Solution: Track capacity separately and grow array when capacity grows:
```c
// Added to Node struct:
size_t edge_context_capacity;

// In node_update_context_values():
if (node->edge_context_capacity < node->outgoing_capacity) {
    node->edge_context_values = realloc(..., node->outgoing_capacity * sizeof(float));
    node->edge_context_capacity = node->outgoing_capacity;
}
```

**Result**: Still crashes

### Hypothesis 3: Memory Corruption in Non-Refactored Code
**Status**: INVESTIGATING

The crash happens at offset +9384 in `melvin_m_process_input`, which is a large function.
This could be in:
- Pattern matching code
- Node creation code  
- Edge creation code
- Output generation code

The crash happens specifically when processing a DIFFERENT pattern, suggesting:
- Pattern matching finds different nodes
- Creates new edges between existing and new nodes
- Some pointer/array becomes invalid

### Next Steps

1. **Simplify test**: Process just 1 iteration of each pattern
2. **Add debug output**: Print what's happening before crash
3. **Check array bounds**: Verify all array accesses are within bounds
4. **Memory sanitizer**: Compile with AddressSanitizer to catch the exact issue

## Alignment with README

The bug fixes follow README principles:

✅ **Self-Regulation**: Nodes maintain their own cache validity
✅ **Adaptive Growth**: Arrays grow based on capacity, not hardcoded sizes
✅ **Local Measurements**: Cache invalidation is local to the node

The remaining bug is likely in code we didn't refactor, not in the O(1) edge selection logic.

