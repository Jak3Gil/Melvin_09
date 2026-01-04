# Node Failure Fixes - Implementation Status

## Fixes Implemented

### Fix 1: Hierarchy-Aware Stop Probability ✓
**Status**: Implemented
**Location**: `wave_compute_stop_probability()` in melvin.c (lines ~1030-1100)
**Changes**:
- Added `input_nodes` and `input_count` parameters
- Builds full sequence (input + output) to check for active hierarchy
- If inside hierarchy, reduces all stop signals by 90% (multiply by 0.1)
- Updated all call sites to pass input_nodes

### Fix 2: Ensure Hierarchy Edges Exist ✓
**Status**: Implemented
**Location**: `node_compute_winning_edge_with_context()` in melvin.c (lines ~2263-2298)
**Changes**:
- If hierarchy says "output X" but no edge exists, creates it
- Uses `graph_find_or_create_pattern_node()` to find/create target node
- Creates edge with weight 2.0 (strong, because hierarchy says it's correct)
- Adds edge to graph using `graph_add_edge()`

### Fix 3: Forward Declarations ✓
**Status**: Implemented
**Location**: melvin.c (lines ~879-883)
**Changes**:
- Added forward declarations for `find_active_hierarchy()`
- Added forward declarations for `graph_find_or_create_pattern_node()`
- Added forward declarations for `graph_add_edge()`

## Test Results

### Test: test_hierarchy_debug_detailed
**Training**: 200 iterations of "hello"
- Final: 18 nodes, 71 edges
- Hierarchies ARE forming ✓

**Test 1**: Input "hel" → Expected "lo"
- Actual: "e" (1 byte)
- ✗ FAILED

**Test 2**: Input "h" → Expected "ello"
- Actual: "eee" (3 bytes)
- ✗ FAILED

**Test 3**: Input "he" → Expected "llo"
- Actual: "eeee" (4 bytes)
- ✗ FAILED

## Analysis of Current Behavior

### Observations:
1. **Hierarchies are forming** - 18 nodes from 5 unique bytes confirms this
2. **Wrong output** - System outputs 'e' repeatedly instead of continuing the pattern
3. **Stop probability still too high** - Only 1 byte for "hel" input (should output "lo" = 2 bytes)
4. **Self-loop on 'e'** - The system is stuck in an e→e loop

### Possible Issues:

#### Issue A: Hierarchy matching not finding the right hierarchy
- `find_active_hierarchy()` might not be finding "hello" hierarchy
- Or hierarchy might not exist (only 2-byte hierarchies like "he", "el", "ll", "lo")
- Need to verify what hierarchies actually exist

#### Issue B: Edge selection choosing wrong edge
- Even with hierarchy guidance, might be selecting e→e instead of l→o
- Position-aware learning might not be working
- Activation-based boosting might be boosting wrong edges

#### Issue C: Stop probability still stopping too early
- Even with 90% reduction, might still be too high
- Need to check if hierarchy is actually being found
- If hierarchy not found, no penalty applied

## Next Steps

### Debug 1: Add logging to see what hierarchies exist
- Print all nodes with abstraction_level > 0
- Show their payloads
- Verify "hello" or multi-byte hierarchies exist

### Debug 2: Add logging to hierarchy matching
- Print when `find_active_hierarchy()` is called
- Show what sequence it's matching
- Show which hierarchy (if any) is found
- Show the expected next byte

### Debug 3: Add logging to edge selection
- Print which edges are available from current node
- Show their weights and context values
- Show which edge is selected and why

### Debug 4: Add logging to stop probability
- Print the stop signals before and after hierarchy penalty
- Show if hierarchy was found
- Show final stop probability

## Hypothesis

The most likely issue is that **hierarchies are only 2-byte sequences** (like "he", "el", "ll", "lo"), not the full "hello" sequence. This means:
- Input "hel" matches "he" hierarchy (2 bytes)
- After "he", we're at position 2, which is 'l'
- But the hierarchy only covers "he", so no guidance for what comes after
- System falls back to position-aware selection
- Position-aware selection picks wrong edge (e→e instead of l→o)

If this is true, we need to:
1. Verify hierarchies are only 2-byte
2. Understand why longer hierarchies aren't forming
3. Either: fix hierarchy formation OR make position-aware selection work better

