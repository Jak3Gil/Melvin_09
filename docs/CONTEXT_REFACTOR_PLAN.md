# Context Refactor Plan: Storage → Passive Computation

## Goal
Replace explicit context storage (SparseContext, ContextTag) with passive computation from existing data.

## Current State
- 159 references to context tags/SparseContext in melvin.c
- ~13,166 lines of code
- Large refactor required

## Strategy

### Phase 1: Remove Storage Structures ✅
- Remove ContextTag from Edge structure ✅
- Remove SparseContext typedef
- Keep ActivationPattern (still needed for wave propagation)

### Phase 2: Replace Functions
- Remove: `sparse_context_create_from_nodes()` → No longer needed
- Remove: `sparse_context_match()` → No longer needed
- Remove: `sparse_context_free()` → No longer needed
- Remove: `sparse_context_clone()` → No longer needed
- Remove: `edge_add_context_tag()` → No longer needed
- Remove: `edge_prune_context_tags()` → No longer needed
- Replace: `edge_compute_context_weight()` → `compute_passive_context_signal()`

### Phase 3: Update Callers
- Find all calls to removed functions
- Replace with passive computation
- Use existing: activation_strength, sparse_embedding, wave_state

### Phase 4: Simplify Edge Selection
- Remove context_match_quality parameter
- Use passive signals directly:
  - target_activation (from wave propagation)
  - embedding_similarity (semantic context)
  - co_activation (connected nodes)

## New Approach

**Passive Context Signal:**
```c
float context_signal = compute_passive_context_signal(edge, wave_state);
// Returns: sum of (target_activation + embedding_sim + co_activation)
// All derived from existing data - zero storage
```

**Edge Selection:**
```c
float score = base_relative + context_signal;
if (node->net) {
    float refinement = mini_net_forward(node->net, inputs);
    score += refinement;
}
```

## Memory Savings

**Before:**
- Edge: 48 bytes + context_tags (100-1000 bytes)
- Total: 464-1,296 bytes per edge
- Brain scale: 46-130 petabytes

**After:**
- Edge: 24 bytes (just core structure)
- Total: 24 bytes per edge
- Brain scale: 2.4 petabytes

**Savings: 95% memory reduction**

## Implementation Status

- [x] Remove ContextTag from Edge structure
- [x] Create passive context computation function
- [ ] Remove all SparseContext/ContextTag functions
- [ ] Update all callers
- [ ] Test accuracy
- [ ] Verify memory reduction
