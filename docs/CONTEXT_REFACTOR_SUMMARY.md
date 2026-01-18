# Context Refactor Summary: Storage → Passive Computation

## Goal
Replace explicit context storage (SparseContext copies in edges) with passive computation from existing data.

## Implementation

### What Changed:
1. **Edge structure**: context_tags fields kept but deprecated (always NULL, tag_count = 0)
2. **Context computation**: Now passive - uses target_activation + embedding_similarity
3. **Context tag additions**: Removed - no longer adding context tags during training
4. **Memory**: 90-96% reduction (48 bytes vs 464-1,296 bytes per edge)

### What Stayed:
1. **SparseContext structure**: Still exists for temporary computation
2. **sparse_context functions**: Still exist but not used for edge storage
3. **Edge selection**: Still uses same algorithm, just with passive signals

---

## Results

### Memory: ✅ Success
- Edge size: 48 bytes (vs 464-1,296 bytes before)
- Savings: 90-96% reduction
- Brain-scale compatible: 4.8 petabytes (vs 46-130 petabytes before)

### Accuracy: ❌ Failure
- test_associations: Gibberish outputs
- test_simple: Garbage output
- Error rate: ~100%

---

## Why Passive Context Doesn't Work

### The Core Issue:
**Passive context requires target nodes to be pre-activated**, but they're not:
- During generation, we're at node 'h', choosing next edge
- Target 'e' hasn't been reached yet, so `activation_strength = 0.0`
- No context signal to help choose!

### The Math:
```
score = base_relative + target_activation + embedding_sim
      = 1.0 + 0.0 + 0.3
      = 1.3

All edges score similarly (target_activation = 0 for all)
Selection is random
```

### What's Missing:
1. **Training memory**: Edges don't remember "I was trained in context X"
2. **Pre-activation**: Target nodes not activated before selection
3. **Discrimination**: Embeddings not different enough for single bytes

---

## The Trade-Off

**Storage vs Accuracy:**
- Storage: 90-96% reduction ✅
- Accuracy: 0% (complete failure) ❌

**Brain vs Implementation:**
- Brain: Passive context works because neurons pre-activate through spreading activation
- Melvin: Wave propagation exists but doesn't pre-activate targets enough

---

## Possible Solutions

### 1. Stronger Wave Propagation
- Activate target nodes BEFORE edge selection
- Requires: Multiple wave passes before generation
- Cost: More compute, but no storage

### 2. Minimal Context Memory
- Store just node IDs (not full SparseContext)
- 8 bytes per association vs 100-400 bytes
- 90% reduction vs current, but some storage

### 3. Hybrid Approach
- Use passive context when targets are activated
- Fall back to edge weights when not
- Best of both worlds?

### 4. Accept Context Storage
- Keep context tags (current approach before this refactor)
- Accept 464-1,296 bytes per edge
- Prioritize accuracy over memory

---

## Recommendation

**Passive context is theoretically correct but practically doesn't work** because:
1. Target nodes aren't pre-activated during generation
2. Embeddings aren't discriminating enough
3. No training context memory

**Options:**
1. Fix wave propagation to pre-activate targets (complex)
2. Use minimal context memory (8 bytes per association)
3. Revert to context tags (accept memory cost for accuracy)

**User's choice**: Which trade-off is acceptable?
- Memory efficiency (passive, 48 bytes, 0% accuracy)
- Accuracy (context tags, 464-1,296 bytes, ~20% accuracy)
- Middle ground (minimal memory, 8 bytes per association, unknown accuracy)
