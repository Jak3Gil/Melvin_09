# Weight Contribution Fix Results

## Problem Identified

The `edge_compute_context_weight()` function was not using `weight_contribution` from context tags, violating:
- **Requirement.md line 6**: "context changes the edge weights" - weight_contribution represents how often a context was trained
- **README Principle 1**: "All decisions are relative" - weight_contribution is relative (training frequency)
- **README Principle 2**: "Data-driven" - weight_contribution comes from training, not hardcoded

## Fix Implemented

Changed from finding only `best_match` to using winner-take-all based on `(match_strength * weight_contribution)`:

1. **Winner-take-all selection**: Find the context tag with highest `(match^2 * weight_contribution)`
   - This ensures more frequently trained contexts win (data-driven, relative)
   - Prevents confusion when multiple tags match equally

2. **Context boost calculation**: 
   - `context_boost = best_weighted_score * adaptive_mult * level_boost`
   - `best_weighted_score = (match^2) * weight_contribution`
   - More training (higher weight_contribution) + strong match = stronger signal

## Test Results

### test_context_specificity.c
- **Test 1**: ✅ PASSED - Output matches one of the learned patterns ("hello" → " world")
- **Test 3**: ⚠️ Accuracy degraded after learning more patterns ("hello" → " l" instead of " world" or " there")

### test_association_scaling.c
- **Association Tests**: 11/20 passed (55.0%)
- **Average accuracy**: 10.3% (improved from previous runs)
- **Loops detected**: 0 ✅
- **Interference Tests**: 0/4 passed (0.0%)

## Analysis

The fix correctly implements `weight_contribution` usage, but accuracy issues persist. The root cause appears to be:

1. **Context ambiguity**: When querying "hello", the context is just ["hello"], which matches both "hello world" and "hello there" equally well. If both were trained the same number of times, they have the same `weight_contribution`, so there's no tie-breaker.

2. **Context tag merging**: When training multiple patterns that share common nodes, `edge_add_context_tag` finds the best matching tag and reinforces it, potentially causing tags to merge even when they represent different patterns.

3. **Context updates during generation**: The context should update as we generate output (e.g., "hello" → "hello world"), but by the time we've generated enough to distinguish patterns, we may have already chosen the wrong edge.

## Next Steps

1. Investigate context tag creation/merging logic in `edge_add_context_tag`
2. Consider including "future context" (what comes after the edge) in context tags to better distinguish patterns
3. Verify that context is being updated correctly during generation as output is produced
