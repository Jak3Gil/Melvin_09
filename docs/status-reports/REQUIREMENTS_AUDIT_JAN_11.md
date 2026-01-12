# Requirements Audit - January 11, 2026

## Requirements from Requirement.md

1. **NO O(n) searches, no global searches for anything**
2. **No hardcoded limits** - no maxes or mins
3. **No hardcoded thresholds**
4. **No Fallbacks**
5. Context is a payload of activated nodes
6. Edges are paths (only paths nodes can take)
7. Nodes make mini predictions
8. **Enable embeddings for output generation only** (not during training)
9. Use cached embeddings (already implemented)
10. **Only compute for candidate nodes** (not all nodes)

---

## VIOLATIONS FOUND

### 1. O(n) SEARCH VIOLATIONS ❌

**CRITICAL**: Multiple functions iterate over ALL nodes or ALL edges in the graph.

| Location | Line | Code | Problem |
|----------|------|------|---------|
| `graph_save_to_mfile()` | 8807 | `for (size_t i = 0; i < graph->node_count; i++)` | Iterates ALL nodes |
| `graph_save_to_mfile()` | 8832 | `for (size_t i = 0; i < graph->node_count; i++)` | Iterates ALL nodes |
| `graph_save_to_mfile()` | 8846 | `for (size_t i = 0; i < graph->node_count; i++)` | Iterates ALL nodes |
| `graph_save_to_mfile()` | 8923 | `for (size_t i = 0; i < graph->edge_count; i++)` | Iterates ALL edges |
| `spread_activation_multi_hop()` | 5830 | `for (size_t i = 0; i < graph->node_count && degree_count < 100; i++)` | Samples up to 100 nodes |
| `generate_from_pattern_internal()` | 6579 | `for (size_t gi = 0; gi < graph->node_count && sampled < 50; gi++)` | Samples up to 50 nodes |
| `hash_table_rehash()` | 5191 | `for (size_t i = 0; i < graph->hash_table_size; i++)` | Iterates entire hash table |
| `edge_pair_table_rehash()` | 4744 | `for (size_t i = 0; i < graph->edge_pair_table_size; i++)` | Iterates entire edge pair table |

**Note**: File save operations (8807, 8832, 8846, 8923) may be acceptable as they occur during explicit save, not during processing. However, the sampling loops (5830, 6579) are called during runtime.

---

### 2. HARDCODED LIMIT VIOLATIONS ❌

| Location | Line | Code | Issue |
|----------|------|------|-------|
| Output length | 6573 | `if (max_output_len < 64) max_output_len = 64;` | Hardcoded minimum 64 |
| Cycle window | 6595 | `(graph->node_count < 256 ? graph->node_count : 256) : 64` | Hardcoded max 256, fallback 64 |
| Pattern match | 7753 | `if (max_try_len > 20) max_try_len = 20;` | Hardcoded max 20 |
| Edge weight floor | 9618 | `if (edge->weight < 10) edge->weight = 10;` | Hardcoded minimum 10 |
| Stop weight cap | 9671 | `if (last_node->stop_weight > 10.0f) last_node->stop_weight = 10.0f;` | Hardcoded max 10.0 |
| Loop bounds | various | `&& i < 5`, `&& i < 10`, `&& neighbor_count < 20` | Many hardcoded iteration limits |
| Sampling | 5830 | `degree_count < 100` | Hardcoded sample size 100 |
| Sampling | 6579 | `sampled < 50` | Hardcoded sample size 50 |
| Context limits | various | `&& i < 3`, `&& i < 8` | Hardcoded context limits |

**Full list of hardcoded loop bounds:**
- `&& i < 3` (lines 1348, 9940)
- `&& i < 5` (lines 2825, 2846, 5708, 7429, 7532, 8384)
- `&& i < 10` (lines 3420, 3455, 7488, 7505, 7582, 9953)
- `&& i < 20` (lines 3429, 3465, 7597)
- `&& j < 10` (line 4903)

---

### 3. HARDCODED THRESHOLD VIOLATIONS ❌

| Location | Line | Code | Issue |
|----------|------|------|-------|
| Error threshold | 2526 | `if (error_signal > 0.5f) return;` | Hardcoded 0.5 threshold |
| Prediction accuracy | 5084 | `if (node && fabsf(node->prediction_error) < 0.1f)` | Hardcoded 0.1 threshold |
| Score ratio | 6971 | `if (score_ratio < 0.5f)` | Hardcoded 0.5 threshold |
| Confidence threshold | 7012 | `if (!edge_has_context && relative_confidence < 0.5f)` | Hardcoded 0.5 threshold |
| Confidence threshold | 7015 | `if (edge_has_context && relative_confidence < 0.3f)` | Hardcoded 0.3 threshold |
| Activation threshold | 7033 | `if (relative_activation < 0.1f && output_len > 0)` | Hardcoded 0.1 threshold |
| Stop probability | 7051 | `if (stop_prob > 0.5f)` | Hardcoded 0.5 threshold |
| Rate bounds | 5665-5666 | `if (rate < 0.05f) rate = 0.05f; if (rate > 0.95f) rate = 0.95f;` | Hardcoded min/max |
| Temperature floor | 8098 | `if (temperature < 0.01f) temperature = 0.01f;` | Hardcoded minimum |
| Tag weight | 3845 | `if (relative_weight > 0.1f)` | Hardcoded threshold |
| Activation floor | 6465 | `if (activation < 0.01f) continue;` | Hardcoded threshold |
| Exploration check | 5932 | `if (exploration_rate < 0.01f) return;` | Hardcoded threshold |
| Mini-net threshold | 5003 | `if (node->net && fabsf(node->prediction_error) > 0.01f)` | Hardcoded threshold |

**Many magic float constants throughout:**
- `0.001f` - epsilon values
- `0.01f` - various thresholds
- `0.1f` - various thresholds
- `0.3f` - confidence thresholds
- `0.5f` - used extensively as "neutral" value
- `0.7f` - context matching thresholds
- `0.9f`, `0.95f` - rate caps
- `2.0f`, `3.0f`, `4.0f` - multipliers

---

### 4. FALLBACK VIOLATIONS ❌

| Location | Line | Code | Issue |
|----------|------|------|-------|
| Terminal lookup | 4641 | `return best ? best : current->terminal_nodes[0];` | Fallback to first if no context |
| Raw node estimate | 5060 | `// Fallback: estimate from total - blanks - hierarchies` | Estimation fallback |
| Last resort | 6553 | `// Last resort fallback` | Explicit fallback comment |
| Hierarchy lookup | 7774 | `// Fallback: use hierarchy index (O(hierarchies) not O(all_nodes))` | Fallback mechanism |
| Byte node creation | 7814 | `// Fallback: create byte node` | Node creation fallback |
| Data-driven computation | 8038 | `// Fallback: data-driven computation` | Computation fallback |
| Temperature | 8090 | `// Fallback: entropy-based temperature` | Temperature fallback |

---

### 5. EMBEDDING USAGE VIOLATIONS ⚠️

**Requirement**: "Enable embeddings for output generation only (not during training)"

**Current Implementation**:
- Embeddings ARE computed lazily (on first access) ✓
- Embeddings ARE cached ✓
- `node_invalidate_embedding()` is called during weight updates, edge creation, hierarchy formation

**Potential Issues**:
- Line 3264: `node_invalidate_embedding(node)` called during `node_activate()` (training path)
- Lines 3601-3603: `node_invalidate_embedding()` called during edge creation (training path)
- Lines 3993-3995: `node_invalidate_embedding()` called during edge weight update (training path)
- Line 8401: `node_invalidate_embedding()` called during hierarchy formation (training path)

**Analysis**: Invalidation during training is fine (doesn't compute, just marks for later). The `node_get_embedding()` function is primarily called from `compute_embedding_similarity()` which is used in output generation. **This appears COMPLIANT** - embeddings are lazily computed only when needed for output.

---

### 6. OTHER CONCERNS

**Mini-net weight initialization:**
```c
// Line 963
net->weights[i] = 0.0f;  // Neutral start, data-driven learning
```
Uses zero initialization - acceptable as it's neutral, not a hardcoded value.

**Adaptive epsilon calculations:**
```c
// Line 559
float epsilon = (range > 0.0f) ? (range * 0.001f) : 0.001f;
```
Falls back to hardcoded 0.001f when no data available.

**Decay values:**
```c
// Line 2548
float decay = 0.7f;  // How much of previous context to retain
```
Hardcoded decay factor.

---

## SUMMARY TABLE

| Requirement | Status | Count | Severity |
|-------------|--------|-------|----------|
| No O(n) searches | ❌ VIOLATED | 8+ locations | HIGH |
| No hardcoded limits | ❌ VIOLATED | 20+ locations | HIGH |
| No hardcoded thresholds | ❌ VIOLATED | 25+ locations | HIGH |
| No fallbacks | ❌ VIOLATED | 7 locations | MEDIUM |
| Embeddings output-only | ✅ COMPLIANT | - | - |
| Cached embeddings | ✅ COMPLIANT | - | - |
| Compute for candidates only | ⚠️ PARTIAL | See note | MEDIUM |

**Note on "Compute for candidates only"**: The sampling loops (5830, 6579) iterate through nodes but with limits. This may technically violate the requirement depending on interpretation.

---

## RECOMMENDATIONS

### Priority 1: Remove O(n) Iterations in Processing
1. Replace global sampling in `spread_activation_multi_hop()` with index-based access
2. Replace global sampling in `generate_from_pattern_internal()` with index-based access

### Priority 2: Remove Hardcoded Limits
1. Make loop bounds adaptive based on local node degree/connectivity
2. Remove fixed sample sizes (50, 100)
3. Make output length limits data-driven

### Priority 3: Remove Hardcoded Thresholds
1. All `0.5f` thresholds should be computed from local statistics
2. Rate bounds (0.05f, 0.95f) should emerge from data
3. Confidence thresholds should be relative to local averages

### Priority 4: Remove Fallbacks
1. Return NULL/empty instead of falling back to first element
2. Remove estimation fallbacks - track exact counts
3. Make all code paths explicit (no "last resort" logic)

---

## FILES REQUIRING CHANGES

Only `src/melvin.c` requires changes. Specific functions:
- `spread_activation_multi_hop()`
- `generate_from_pattern_internal()`
- `graph_save_to_mfile()`
- Multiple prediction/scoring functions
- Various edge/node update functions
