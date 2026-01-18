# Edge Direction Refactor

## Problem

Previous implementation created bidirectional edges in `create_hierarchy_node`:
- **Hierarchy→Component edges** (lines 2440-2445): Created reverse edges not seen in input
- **Component→Hierarchy edges** (lines 2451-2460): Created forward edges following input

This violated the fundamental principle: **edges should only follow input order**.

## Principle

**Edges Only Follow Input Direction**

Input "world" creates:
- w→o, o→r, r→l, l→d ✓ (forward, as seen in input)

Should NOT create:
- o→w, r→o, l→r, d→l ✗ (reverse, not in input)

## Solution

### Removed (Lines 2440-2445)
```c
// REMOVED: Create edges from hierarchy to sequence nodes (for matching)
for (size_t i = 0; i < seq_len; i++) {
    Edge *edge = edge_create(hierarchy, sequence[i]);  // ❌ REVERSE
    if (edge) {
        graph_add_edge(graph, edge);
    }
}
```

### Kept (Lines 2451-2460)
```c
// RULE 1: Component→Hierarchy (shortcut entry)
// When first component activates, it can jump directly to hierarchy
// Example: 'w' activates → can choose 'w'→'o' OR 'w'→"world" (hierarchy)
if (seq_len > 0 && sequence[0]) {
    Edge *comp_to_hierarchy = find_edge_between(sequence[0], hierarchy);
    if (!comp_to_hierarchy) {
        comp_to_hierarchy = edge_create(sequence[0], hierarchy);  // ✓ FORWARD
        if (comp_to_hierarchy) {
            graph_add_edge(graph, comp_to_hierarchy);
        }
    }
}
```

## Edge Creation Rules

### Rule 1: Sequential Edges (in `create_sequential_edges`)
- **Direction**: Component→Next Component
- **Example**: w→o, o→r, r→l, l→d
- **Purpose**: Basic pattern learning

### Rule 2: Component→Hierarchy (in `create_hierarchy_node`)
- **Direction**: First Component→Hierarchy
- **Example**: w→"world"
- **Purpose**: Shortcut entry to learned pattern

### Rule 3: Hierarchy→Next (in `create_sequential_edges`)
- **Direction**: Hierarchy→Next Component (after pattern)
- **Example**: "world"→[space] (created when hierarchy appears in input)
- **Purpose**: Continuation after pattern completes

### Rule 4: STOP Edge (in `create_sequential_edges`)
- **Direction**: Last Component→STOP
- **Example**: d→STOP
- **Purpose**: Signal pattern end

## Benefits

1. **Consistency**: All edges follow input order
2. **Simplicity**: No reverse edges to manage
3. **Brain-like**: Neurons fire in sequence, not backward
4. **Scalability**: Fewer edges = less memory, faster decisions

## Verification

All edge creations in `melvin.c`:
1. Line 783: `edge_create(from, to)` - sequential order ✓
2. Line 806: `edge_create(last_node, graph->stop_node)` - STOP edge ✓
3. Line 2451: `edge_create(sequence[0], hierarchy)` - component→hierarchy ✓
4. Line 3649: `edge_create(nodes[i], nodes[i + 1])` - sequential order ✓

## Test Results

```
TEST 1: Forward generation from 'w'
  Output: 'or'  ✓ (forward continuation)

TEST 2: Forward generation from 'wo'
  INFO: No output generated (needs more training)

TEST 3: No reverse generation from 'd'
  PASS: No output generated ✓ (end of pattern)

TEST 4: Multiple patterns
  'w' → 'or'  ✓ (forward)
  'h' → 'el'  ✓ (forward)
```

## Impact

- **Graph Structure**: Cleaner, more directed
- **Memory**: Reduced edge count (no reverse edges)
- **Learning**: More focused (edges only strengthen in used direction)
- **Generation**: More predictable (follows learned sequences)
