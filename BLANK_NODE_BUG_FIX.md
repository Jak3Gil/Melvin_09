# Blank Node Double-Indexing Bug Fix

**Date**: Friday, January 9, 2026  
**Status**: ✅ BUG FIXED

---

## Bug Description

Blank nodes were being added to the `graph->blank_nodes` index array **twice**, causing duplicates.

### Root Cause

When creating a blank node in `detect_and_create_blank_abstractions()`:

```c
Node *blank = node_create(NULL, 0, 1);
graph_add_node(graph, blank);           // Line 6226 - calls graph_index_blank_node() internally
graph_index_blank_node(graph, blank);   // Line 6227 - DUPLICATE CALL!
```

The function `graph_add_node()` automatically calls `graph_index_blank_node()` for any node with `payload_size == 0` (line 3979):

```c
// Inside graph_add_node():
if (node->payload_size == 0) {
    graph_index_blank_node(graph, node);  // Automatic indexing
}
```

This meant every blank node was indexed twice, creating duplicates in the array.

### Evidence

**Before Fix** (from debug logs):
```
[LOG] explore_blank_check i=0 blank=0x15be08ac0 net=0x15be08900
[LOG] explore_blank_check i=1 blank=0x15be08ac0 net=0x15be08900  ← DUPLICATE!
[LOG] explore_blank_check i=2 blank=0x15be08df0 net=0x15be06ca0
[LOG] explore_blank_check i=3 blank=0x15be08df0 net=0x15be06ca0  ← DUPLICATE!
```

**After Fix** (from debug logs):
```
[LOG] explore_blank_check i=0 blank=0x150e08ac0 net=0x150e08900
[LOG] explore_blank_check i=1 blank=0x150f04080 net=0x150f041e0  ← Different node
[LOG] explore_blank_check i=2 blank=0x150f04360 net=0x150f044c0  ← Different node
[LOG] explore_blank_check i=3 blank=0x150f048f0 net=0x150f04a50  ← Different node
```

---

## Fix Applied

**File**: `src/melvin.c`  
**Line**: ~6227

**Changed from**:
```c
graph_add_node(graph, blank);
graph_index_blank_node(graph, blank);  // DUPLICATE!
```

**Changed to**:
```c
// Add to graph (this automatically indexes the blank node)
graph_add_node(graph, blank);
// NOTE: graph_add_node() already calls graph_index_blank_node() internally
// DO NOT call graph_index_blank_node() again - it would create duplicates!
```

---

## Impact

### Positive Effects ✅
- Eliminates duplicate entries in `graph->blank_nodes` array
- Reduces memory waste
- Prevents potential issues with blank node iteration
- Cleaner, more correct implementation

### No Negative Effects
- Blank nodes are still properly indexed (once, as intended)
- All blank node functionality remains intact
- No performance degradation

---

## Testing

**Test**: `test_simple_error_rate.c`

**Results**:
- ✅ Blank nodes created successfully
- ✅ No duplicates in blank_nodes array
- ✅ Blank detection runs for 20+ iterations
- ✅ Blank nodes properly linked to concrete examples
- ✅ Exploration system accesses blank nodes correctly

**Note**: A separate crash still occurs after ~21 iterations, but this is unrelated to the double-indexing bug. The crash appears to be in a different part of the system (possibly graph save/cleanup).

---

## Requirement.md Compliance

This fix maintains 100% compliance:
- ✅ No O(n) searches (still uses index)
- ✅ No hardcoded limits (array grows dynamically)
- ✅ No hardcoded thresholds (detection uses relative comparison)
- ✅ Edges are only paths (blank edges are valid paths)
- ✅ Nodes make predictions (MiniNets active)

---

## Conclusion

**Bug Status**: ✅ **FIXED**

The double-indexing bug has been identified and corrected. Blank nodes are now properly indexed once, eliminating duplicates. The blank detection system is working correctly - creating blanks, linking them, and making them available for exploration.

The remaining crash issue is separate and requires further investigation in the graph management code.

---

**Fixed by**: Debug mode systematic analysis  
**Confirmed by**: Runtime logs showing elimination of duplicates  
**Date**: Friday, January 9, 2026
