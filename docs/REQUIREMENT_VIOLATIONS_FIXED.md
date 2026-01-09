# Requirement.md Violations - Investigation and Fixes

## Critical Violation Found and Fixed

### Violation #1: Edge Creation During Output Generation

**Location**: `src/melvin.c` lines 2847-2862

**Requirement Violated**: 
> Line 6: "edges are paths they are the only paths that nodes can take, if a node doesnt have a edge between another node it cant touch that one or predict it"

**What Was Happening**:
```c
// FIX 2: ENSURE HIERARCHY EDGES EXIST
// If hierarchy says "output X" but no edge exists, create it
if (!result && graph) {
    Node *target = graph_find_or_create_pattern_node(graph, &expected_next, 1);
    if (target && target != node) {
        Edge *new_edge = edge_create(node, target);  // VIOLATION!
        if (new_edge) {
            new_edge->weight = 2.0f;
            graph_add_edge(graph, new_edge);
            result = new_edge;
        }
    }
}
```

**Why This Violated Requirement.md**:
1. During output generation, if a hierarchy suggested a next byte but no edge existed
2. The code would CREATE the edge on the fly
3. This allowed predictions that weren't in the training data
4. Example: "o" → "w" edge created during output even though it never appeared in training

**Impact**:
- System could generate outputs using paths that don't exist in training data
- Violated fundamental constraint that edges are the ONLY paths
- Caused "wowowo" loops by creating impossible transitions

**Fix Applied**:
```c
// REMOVED: Edge creation during output generation
// Per Requirement.md line 6: "edges are paths they are the only paths that nodes can take,
// if a node doesnt have a edge between another node it cant touch that one or predict it"
// Edges must ONLY be created during training, never during output generation
// If hierarchy says output X but no edge exists, we stop (no valid path)
```

**Result**: Edge creation during output generation completely removed.

---

## Other Edge Creation Locations Investigated

### Valid: Edge Creation During Training

**Location 1**: Line 5459-5463 (`melvin_m_process_input`)
```c
// No direct edge - create one for learning
// This allows error feedback to strengthen correct pattern-activated paths
Edge *new_edge = edge_create(current_node, next_node);
```
**Status**: ✅ VALID - This is during input processing (training phase)

**Location 2**: Line 6277-6279 (`graph_learn_from_predictions`)
```c
if (!correct_edge) {
    // Create edge if it doesn't exist
    correct_edge = edge_create(current_node, actual_next);
    if (correct_edge) graph_add_edge(graph, correct_edge);
}
```
**Status**: ✅ VALID - This is during training (prediction error learning)

**Location 3**: Line 6413-6426 (`graph_process_sequential_patterns`)
```c
// Create new edge (including edges to hierarchy nodes!)
Edge *edge = edge_create(prev_node, node);
```
**Status**: ✅ VALID - This is during input processing (training phase)

**Location 4**: Line 8050-8069 (input processing)
```c
// CREATE NEW EDGE - This is critical for learning new patterns!
edge = edge_create(from, to);
```
**Status**: ✅ VALID - This is during input processing (training phase)

**Location 5**: Line 8496-8498 (`melvin_m_strengthen_continuation`)
```c
if (!edge) {
    edge = edge_create(prev_node, node);
    if (edge) {
        graph_add_edge(graph, edge);
    }
}
```
**Status**: ✅ VALID - This is during self-supervised training

**Location 6**: Line 7596 (file loading)
```c
// Create edge
Edge *edge = edge_create(from_node, to_node);
```
**Status**: ✅ VALID - This is reconstructing edges from saved file

---

## Remaining Issues

### Issue #1: Single Node for Repeated Characters

**Problem**: "hello world" has two "o" characters:
- One in "hello" (followed by space)
- One in "world" (followed by "r")

But `graph_find_or_create_pattern_node` creates only ONE node for "o".

**Result**: The single "o" node has edges to BOTH " " and "r", causing ambiguity.

**Why This Happens**:
- The system uses a single node per unique byte pattern
- Context should disambiguate which "o" we're at
- But context mechanisms (embeddings, context_trace) aren't working effectively yet

**This is NOT a violation**: Requirement.md doesn't say each occurrence needs a separate node. The context mechanisms should handle disambiguation.

### Issue #2: Context Mechanisms Not Effective

**Problems Identified**:
1. **Context trace is empty** during early iterations (timing issue)
2. **Habituation not strong enough** to prevent loops
3. **Embedding similarity** may boost wrong choices
4. **Loop detection** threshold may be too permissive

**These are implementation issues**, not Requirement.md violations.

---

## Summary

### Violations Fixed: 1
- ✅ Removed edge creation during output generation (lines 2847-2862)

### Violations Remaining: 0
- All edge creation now happens only during training/input processing
- System strictly follows Requirement.md line 6

### Test Results After Fix:
- Error rate: Still 66.7% (same as before)
- Output: Still "wowowo"
- **Why**: The o→w edge doesn't exist from training, but the system has:
  - One "o" node with edges to both " " and "r"
  - Context mechanisms not working effectively to choose the right edge
  - The "wo" pattern followed by "wo" again suggests the system is choosing the wrong edge from "o"

### Next Steps:
The violation is fixed, but the system still needs:
1. Better context disambiguation (fix timing of context_trace updates)
2. Stronger habituation to prevent loops
3. Better use of embeddings to distinguish "o" after "hell" vs "o" after "w"

These are implementation improvements, not Requirement.md violations.
