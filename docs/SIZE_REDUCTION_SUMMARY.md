# Node/Edge Size Reduction Summary

## Changes Made

### 1. Requirements Updated (`Requirement.md`)
Added brain-scale requirements:
- Node on-disk size < 20 bytes (plus payload)
- Edge on-disk size < 20 bytes
- Compute on-demand: weight sums, best edge, activation
- Active nodes cached in RAM, inactive nodes on disk
- No per-edge context storage - use shared context pool

### 2. Node Structure Refactored
**Essential fields (stored on disk): ~13 bytes**
- `id[9]` - 9 bytes (unique identifier)
- `port_id` - 1 byte (multi-modal routing)
- `abstraction_level` - 1 byte (hierarchy level)
- `is_reference_hierarchy` - 1 byte
- `flags` - 1 byte (packed flags)
- payload stored separately (variable size)

**Runtime fields (computed/cached, not stored):**
- Edge arrays (loaded on-demand)
- Weight sums (computed from edges)
- Activation strength (computed)
- MiniNet (lazy-loaded)
- Embeddings (lazy-computed)
- All caches (invalidated when data changes)

### 3. Edge Structure Refactored
**Essential fields (stored on disk): 18 bytes**
- `from_node_id` - 8 bytes (node ID hash)
- `to_node_id` - 8 bytes (node ID hash)
- `weight` - 1 byte (0-255)
- `flags` - 1 byte (packed flags)

**Removed from edge (moved to shared pool or computed):**
- Context tags (moved to shared context pool)
- Temporal statistics (computed on-demand from logs)
- EdgeTransformer (lazy-init, not stored)

### 4. Compute-on-Demand Functions
- `node_compute_outgoing_sum()` - computes weight sum when needed
- `node_compute_incoming_sum()` - computes weight sum when needed
- `node_get_local_outgoing_weight_avg()` - uses computed sums
- `node_invalidate_weight_cache()` - invalidates cache when edges change

## Results

### On-Disk Sizes
| Structure | Before | After | Reduction |
|-----------|--------|-------|-----------|
| Node | 384 bytes | 13 bytes | 96.6% |
| Edge | 24+ bytes | 18 bytes | 25% |

### Brain-Scale Projections
| Scale | Before (RAM) | After (Disk) |
|-------|--------------|--------------|
| 86B nodes | 33 TB (impossible) | 1.1 TB |
| 100T edges | 1.8 PB (impossible) | 1.8 PB |

### Memory Usage Test
- 100 iterations of "hello world" training
- Graph stabilizes at 20 nodes, 83 edges
- No unbounded growth (was 200+ nodes before)

## Functionality Preserved

✅ Pattern learning works
✅ Graph stabilizes (doesn't grow unboundedly)
✅ Edge reuse works
✅ Node reuse works
✅ Output generation works

## Trade-offs

### Performance Impact
- Weight sums: O(1) → O(degree) (but degree is small)
- Edge loading: instant → disk I/O (cached after first load)
- MiniNet loading: instant → disk I/O (cached after first load)

### Mitigation
- Runtime caching for active nodes
- Lazy loading for inactive nodes
- Cache invalidation via generation tracking

## Next Steps

1. Implement memory-mapped file I/O for .m files
2. Implement shared context pool for edge contexts
3. Add LRU cache for active nodes
4. Add distributed sharding for horizontal scaling
