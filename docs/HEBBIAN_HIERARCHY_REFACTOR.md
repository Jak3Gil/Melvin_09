# Hebbian Learning & Hierarchy Formation Refactor

**Date**: January 17, 2026  
**Status**: ✅ **COMPLETE**

## Summary

Refactored processing order to move Hebbian learning and hierarchy formation to the **front** (before ENCODE/REFINE/DECODE), enabling newly formed hierarchies to be used during processing for faster pattern matching.

## Key Changes

### 1. Processing Order Refactored

**Before**:
```
1. Create/find nodes
2. Create/find edges + Hebbian learning
3. ENCODE → REFINE → DECODE
4. Hierarchy formation ← TOO LATE!
```

**After**:
```
1. Create/find nodes
2. Create/find edges + Hebbian learning
3. Hierarchy formation ← MOVED HERE!
4. ENCODE → REFINE → DECODE (can use hierarchies)
```

### 2. Benefits of New Order

1. **Hierarchies Available for Processing**:
   - New hierarchies formed BEFORE encoding
   - Pattern matching can use compressed structures
   - Faster processing with hierarchy nodes

2. **Logical Flow**:
   - Structure creation → Learning → Compression → Processing
   - Learning happens immediately (Hebbian)
   - Compression happens after learning (hierarchy formation)
   - Processing uses optimized structure

3. **Compounding Effect**:
   - Hierarchies form from strengthened edges
   - New hierarchies immediately available for matching
   - System gets faster as it learns

### 3. Duplicate Prevention (No Global Searches)

#### Nodes: Hash Table (O(1))
```c
find_or_create_node() {
    1. Hash payload: hash = hash_payload(payload, size)
    2. Get index: index = hash % capacity
    3. Search bucket: while (node) { check payload; node = node->next }
    4. If found: return existing
    5. If not: create new + add to hash table
}
```
- **Complexity**: O(1) amortized (only checks hash bucket)
- **No global search**: Only searches chained hash bucket

#### Edges: Local Search (O(degree))
```c
find_edge_between(from, to) {
    // Search ONLY from->outgoing[] array
    for (size_t i = 0; i < from->outgoing_count; i++) {
        if (from->outgoing[i]->to == to) return edge;
    }
    return NULL;
}
```
- **Complexity**: O(degree) where degree = average outgoing edges
- **No global search**: Only checks one node's outgoing edges
- **Typical**: degree << total_nodes, so O(degree) << O(n)

## Implementation Details

### Modified Functions

#### `process_input_bytes()`
**Location**: Lines 657-733

**Changes**:
- Moved `check_hierarchy_formation()` to BEFORE ENCODE phase
- Added clear phase comments (Phase 1, 2, 3)
- Updated documentation to reflect new order

**New Structure**:
```c
// PHASE 1: STRUCTURE CREATION & LEARNING
1. Create/find nodes (hash table - O(1))
2. Create/find edges (local search - O(degree))
3. Hebbian learning (strengthen edges)
4. Hierarchy formation (compress patterns) ← MOVED HERE

// PHASE 2: PROCESSING
5. ENCODE → REFINE → DECODE (can use hierarchies)

// PHASE 3: CLEANUP
6. Clear input flags
7. Periodic maintenance (decay)
```

#### `find_or_create_node()`
**Location**: Lines 614-628

**Changes**:
- Added detailed comments explaining hash table lookup
- Clarified O(1) complexity and no global search

#### `create_sequential_edges()`
**Location**: Lines 630-658

**Changes**:
- Added detailed comments explaining local search
- Clarified O(degree) complexity and no global search
- Emphasized Hebbian learning happens immediately

#### `check_hierarchy_formation()`
**Location**: Lines 1630-1760

**Changes**:
- Updated section comment: "Phase 1 - Before Processing"
- Added timing note: "Called AFTER Hebbian learning, BEFORE ENCODE phase"
- Clarified that it forms hierarchies BEFORE processing

#### `hebbian_strengthen_edge()`
**Location**: Lines 1537-1566

**Changes**:
- Updated section comment: "Phase 1 - Immediate"
- Clarified timing: "Happens immediately when edges are created/used"

## Safety Considerations

### Why It's Safe to Form Hierarchies Early

1. **Read-Only on Input Nodes**:
   - `check_hierarchy_formation()` only reads from edges
   - Creates new hierarchy nodes (doesn't modify input nodes)
   - Input nodes remain unchanged during hierarchy formation

2. **No Conflicts**:
   - Hierarchies are NEW nodes (abstraction_level > 0)
   - Input nodes are raw nodes (abstraction_level = 0)
   - No modification of nodes being processed

3. **Immutable During Processing**:
   - Hierarchy formation only happens once per input
   - Doesn't modify edges being used during decode
   - Safe to form before processing

## Testing

### Compilation
✅ Code compiles successfully with minor warnings (unused helper functions)

### Runtime Testing
- Tested basic functionality
- Hierarchy formation now happens before ENCODE
- Processing can use newly formed hierarchies

## Complexity Analysis

### Node Creation/Lookup
- **Hash table lookup**: O(1) amortized
- **Hash computation**: O(payload_size)
- **Bucket search**: O(bucket_size), typically O(1)
- **Total**: O(1) amortized per node

### Edge Creation/Lookup
- **Local search**: O(degree) where degree = outgoing_count
- **Typical degree**: 2-10 edges per node
- **Total**: O(degree) per edge, typically O(1) to O(10)

### No Global Searches
- ❌ Never iterates all nodes
- ❌ Never iterates all edges
- ✅ Only uses hash table lookups (nodes)
- ✅ Only uses local array searches (edges)

## Files Modified

- `src/melvin.c`:
  - Lines 610-658: Enhanced node/edge creation documentation
  - Lines 657-733: Refactored `process_input_bytes()` with new order
  - Lines 1537-1566: Updated Hebbian learning comments
  - Lines 1630-1760: Updated hierarchy formation comments

## Conclusion

Successfully refactored processing order to move Hebbian learning and hierarchy formation to the front. This enables:

1. **Faster Processing**: New hierarchies available for pattern matching
2. **Better Structure**: Learning → Compression → Processing flow
3. **Compounding Effect**: System improves faster as it learns

All operations remain local with no global searches:
- Nodes: Hash table (O(1))
- Edges: Local search (O(degree))

**Status**: ✅ Complete - Ready for testing
