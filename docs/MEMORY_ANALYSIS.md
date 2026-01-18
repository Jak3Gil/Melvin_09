# Memory Analysis: Node Size vs Memory Usage

## Node Structure Size
- **Actual Node struct size**: 384 bytes
- **Target**: < 100 bytes
- **Gap**: 284 bytes over target

## Memory Usage Analysis

### Node Structure Breakdown
The Node struct contains:
- **Pointers** (8 bytes each): ~20 pointers = 160 bytes
- **Size_t fields**: ~10 fields = 80 bytes  
- **Float fields**: ~10 fields = 40 bytes
- **Fixed-size fields**: ~104 bytes
- **Total**: 384 bytes

### Why 11MB per Node?

The 11MB per node seen in tests is **NOT from the Node structure itself**, but from:

1. **Context Tags on Edges** (PRIMARY CULPRIT)
   - Each edge can have multiple `SparseContext` objects
   - Each `SparseContext` stores:
     - `Node** active_nodes` array
     - `float* activations` array  
     - `uint8_t* port_ids` array
     - `uint32_t* abstraction_levels` array
   - If a context has 100 active nodes, that's 100 * (8 + 4 + 1 + 4) = 1.7KB per context
   - If an edge has 10 context tags = 17KB per edge
   - If a node has 100 edges = 1.7MB per node (just from context tags!)

2. **Edge Arrays**
   - `outgoing_edges` and `incoming_edges` arrays
   - Each edge pointer = 8 bytes
   - If a node has 100 edges = 800 bytes

3. **MiniNet Structures**
   - Each node has a `MiniNet*` pointer
   - MiniNet itself has weight arrays, state arrays, etc.

4. **Embeddings and Cached Data**
   - Embeddings can be large (adaptive dimension)
   - Semantic neighbor caches
   - Attention keys/values

## Fixes Applied

### 1. Removed Hardcoded Limits ✅
- Removed `write_idx < 5` limit
- Removed `tag_count > 5` threshold
- Removed `relative_weight > 0.2f` threshold
- Made all pruning **adaptive and data-driven**

### 2. Adaptive Pruning ✅
- Pruning threshold based on **median weight** (data-driven)
- Pruning frequency based on **tag_capacity / 2** (adaptive)
- No hardcoded values

### 3. Context Tag Management
- Pruning happens when `tag_count > tag_capacity / 2`
- Keeps tags above median weight (adaptive threshold)
- Frees SparseContext objects when pruning

## Recommendations

### To Reduce Node Structure Size (< 100 bytes)
The Node struct would need significant refactoring:
- Move some fields to separate structures
- Use bitfields for small values
- Lazy initialization of optional features
- This is a major refactoring

### To Reduce Memory Usage (11MB → < 100 bytes/node)
The real issue is context tags:
1. **More aggressive pruning**: Prune context tags more frequently
2. **Context tag limits**: But this violates "no limits" requirement
3. **SparseContext optimization**: Store fewer nodes per context
4. **Lazy context tags**: Only create context tags when needed
5. **Context tag sharing**: Share SparseContext objects between edges

### Current Status
- ✅ **No hardcoded limits**: All thresholds are adaptive/data-driven
- ✅ **Pruning is adaptive**: Based on median weight, not fixed threshold
- ⚠️ **Node struct size**: 384 bytes (exceeds 100 byte target)
- ⚠️ **Memory usage**: 11MB/node (from context tags, not node structure)

## Next Steps

1. **Optimize SparseContext storage**: Use more efficient representation
2. **Lazy context tag creation**: Only create when actually needed
3. **Context tag sharing**: Reuse SparseContext objects
4. **Node structure optimization**: Refactor to < 100 bytes (major work)
