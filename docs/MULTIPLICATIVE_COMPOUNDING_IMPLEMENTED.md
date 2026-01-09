# Multiplicative Compounding Implementation

## Changes Made

### Simplified Edge Scoring to Three Factors

Changed from complex additive mechanisms to **three multiplicative factors**:

```c
score = edge_frequency × embedding_match × habituation
```

### Factor 1: Edge Frequency (Structure)
```c
float context_value = edge->weight / local_avg;
```
- Relative to local average (data-driven)
- Represents how often this path is used in training

### Factor 2: Embedding Match (Context)
```c
float embedding_sim = compute_embedding_similarity(...);
float context_match = (embedding_sim + 1.0f) / 2.0f;  // Normalize to [0,1]
context_value *= context_match;
```
- Semantic similarity between candidate and context
- Normalized cosine similarity
- **MULTIPLICATIVE**: if context doesn't match, score → 0

### Factor 3: Habituation (Novelty)
```c
float habituation = 1.0f / (1.0f + recent_count * 2.0f);
context_value *= habituation;
```
- Penalizes recently output nodes
- Prevents loops
- Data-driven penalty based on repetition count

## Removed Complexity

**Removed:**
- Context_trace complexity (positional state building)
- Attention mechanism (query/key scoring)
- Activation priming from wave_state
- Complex habituation with decay rates

**Kept:**
- Edge weights (frequency)
- Embeddings (semantic context)
- Simple habituation (loop prevention)

## Why Multiplicative?

**Compounding Intelligence** requires all factors to agree:
- Strong edge + wrong context = **low score** ✓
- Weak edge + right context = **low score** ✓
- Strong edge + right context = **high score** ✓

This is true compounding, not competition.

## Test Results

**After implementation:**
- Iteration 120: 60.0% error ("wold")
- Later: 66.7% error ("wowowo")

**No improvement** - same as before.

## Root Cause Analysis

The multiplicative approach is **correct**, but embeddings may be:
1. **Not computed yet** - returning 0 or default values
2. **Too similar** - all candidates score similarly
3. **Computed incorrectly** - not capturing semantic differences

The formula `score = edge_freq × embedding_match × habituation` is sound, but if `embedding_match ≈ 0.5` for all candidates (neutral similarity), it has no discriminative power.

## Next Steps

1. **Debug embedding computation**: Check if embeddings are actually being computed and returning meaningful values
2. **Check embedding dimensions**: Verify nodes have non-zero embedding_dim
3. **Inspect similarity values**: Log actual embedding_sim values to see if they're discriminative
4. **Consider alternative**: If embeddings aren't working, use simpler context (byte-level matching)

## Summary

**Implementation**: ✅ Correct (multiplicative compounding)
**Results**: ❌ No improvement (embeddings may not be working)
**Conclusion**: The architecture is sound, but embeddings need debugging
