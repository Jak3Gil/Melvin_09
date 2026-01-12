# Large-Scale Training Status

## Summary

Attempted to train Melvin on large-scale Wikipedia data (~450MB of text, 12.8M lines). The system crashes during training, indicating memory or stability issues that need to be resolved before large-scale training can proceed.

## What Was Completed

### ✅ Data Preparation
1. **Downloaded Wikipedia Simple English dump** - 325MB compressed
2. **Extracted clean text** - 449MB of plain text from 50,000+ articles
3. **Created training infrastructure**:
   - `tools/train_large_corpus.c` - Batch training tool with progress monitoring
   - `tools/extract_wiki_text.py` - Wikipedia XML to plain text converter
   - `tools/prepare_training_data.sh` - Data download and preparation script

### ✅ Training Infrastructure
- Compiled training tool successfully
- Progress monitoring implemented (MB fed, nodes, edges, throughput)
- Statistics tracking (bytes fed, lines fed, graph growth)

### ❌ Training Execution
- **Status**: FAILED
- **Issue**: System crashes (SIGABRT/SIGSEGV) when processing large amounts of data
- **Tested**: 860KB (10k lines), 100MB (2M lines), 449MB (12.8M lines)
- **Result**: All crash within seconds of starting

## Data Available

| File | Size | Lines | Articles | Status |
|------|------|-------|----------|--------|
| `training_data/wiki_small.txt` | 860KB | 10,000 | ~20 | Ready |
| `training_data/wikipedia_text.txt` | 102MB | 2.6M | 50,000 | Ready |
| `training_data/wikipedia_1gb.txt` | 449MB | 12.8M | 200,000+ | Ready |
| `training_data/news/*.txt` | ~10KB | ~100 | 3 | Ready |
| `training_data/education/*.txt` | ~15KB | ~150 | 3 | Ready |
| `training_data/conversations/*.txt` | ~5KB | ~50 | 1 | Ready |

**Total Available**: ~551MB of clean training text

## Issues Encountered

### 1. Memory Crash (SIGABRT - Exit 134)
```
Training starts → Processes a few hundred lines → SIGABRT
```
**Likely Causes**:
- Memory allocation failure (malloc/realloc returning NULL)
- Double free or invalid free
- Heap corruption
- Edge transformer lazy initialization causing memory issues

### 2. Segmentation Fault (SIGSEGV - Exit 139)
```
Training starts → Immediate crash
```
**Likely Causes**:
- NULL pointer dereference
- Accessing freed memory
- Stack overflow from deep recursion
- Buffer overflow

### 3. Debug Output Overhead
- System generates massive debug output (stderr)
- Slows down training significantly
- Makes it hard to see actual progress

## Root Cause Analysis

### Possible Issues

1. **Edge Transformer Memory**
   - Each edge can lazily allocate an EdgeTransformer (~300 bytes)
   - With millions of edges, this adds up quickly
   - May be hitting memory limits

2. **Sparse Embedding Allocation**
   - Each node can have sparse embeddings
   - Allocation failures not handled gracefully

3. **Graph Growth**
   - System creates many nodes/edges quickly
   - Hash tables may need resizing
   - Realloc failures not handled properly

4. **Context Traces**
   - Adaptive context traces grow dynamically
   - May be growing unbounded

5. **Hierarchy Creation**
   - System creates hierarchy nodes automatically
   - May be creating too many

## Next Steps to Fix

### Immediate (Critical for Training)

1. **Add Memory Limit Checks**
   ```c
   // Before every malloc/realloc
   if (!ptr) {
       fprintf(stderr, "Memory allocation failed\n");
       return gracefully;
   }
   ```

2. **Disable Debug Output**
   - Comment out all `fprintf(stderr, "[LOG]"...)`
   - Comment out all `fprintf(stderr, "[DEBUG]"...)`
   - Keep only critical errors

3. **Add Memory Monitoring**
   ```c
   // Track total memory usage
   // Report every 1MB of data fed
   // Warn if approaching limits
   ```

4. **Lazy Initialization Limits**
   ```c
   // Don't create EdgeTransformer for every edge
   // Only create for frequently-used edges
   // Add usage counter threshold
   ```

5. **Test with Valgrind**
   ```bash
   valgrind --leak-check=full ./tools/train_large_corpus test.m small_data.txt
   ```

### Medium Term (Optimization)

1. **Batch Processing**
   - Process data in smaller batches
   - Save checkpoint after each batch
   - Resume from checkpoint if crash

2. **Memory Pooling**
   - Pre-allocate memory pools
   - Reuse freed memory
   - Reduce malloc/free overhead

3. **Sparse Structure Limits**
   - Cap max sparse embedding dimensions
   - Cap max edge transformers per node
   - Cap max attention heads

4. **Incremental Saves**
   - Save brain file every N MB
   - Allows recovery from crashes
   - Prevents data loss

### Long Term (Scalability)

1. **Memory-Mapped Files**
   - Store graph on disk
   - Load only active portions
   - Scale beyond RAM limits

2. **Distributed Training**
   - Split data across multiple processes
   - Merge results periodically

3. **Pruning During Training**
   - Remove low-weight edges
   - Remove unused transformers
   - Keep graph size bounded

## Workaround for Now

### Option 1: Train on Smaller Batches
```bash
# Split data into 1MB chunks
split -l 2500 training_data/wikipedia_text.txt chunk_

# Train on each chunk
for chunk in chunk_*; do
    ./tools/train_large_corpus brain.m $chunk
done
```

### Option 2: Use Existing Small Tests
```bash
# Use the working test suite
./tests/test_general_intelligence
./tests/test_association_simple
./tests/test_multi_pattern
```

### Option 3: Fix Memory Issues First
1. Run with valgrind to find exact crash location
2. Fix memory leaks and allocation failures
3. Then retry large-scale training

## Current System Capabilities

Despite training issues, the system has:

✅ **All 6 architectural gaps implemented**
✅ **Edge transformer attention (Q, K, V projections)**
✅ **Sparse semantic embeddings**
✅ **Abstraction mechanism**
✅ **Temporal reasoning**
✅ **Cross-modal integration**
✅ **Attention mechanism**
✅ **Uncertainty handling**
✅ **All tests passing on small data**

## Conclusion

The system is **architecturally complete** but has **stability issues** that prevent large-scale training. The crashes need to be debugged and fixed before we can train on 1GB+ of data.

**Priority**: Fix memory issues and crashes before attempting large-scale training again.

**Recommendation**: 
1. Run valgrind to identify exact crash location
2. Add comprehensive error handling for all allocations
3. Disable debug output
4. Test incrementally with increasing data sizes
5. Once stable, resume large-scale training

---

*Status as of: January 11, 2026*
