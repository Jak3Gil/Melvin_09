# SIGABRT Crash Fix Summary

## Problem
The system was crashing with SIGABRT (exit code 134) immediately during the first iteration of processing input, with the error message:
```
malloc: Double free of object 0x...
```

## Root Cause Analysis

### Investigation Process
1. Added comprehensive debug instrumentation to track memory allocations and frees
2. Used stderr logging with fflush() to ensure output before crash
3. Traced the execution flow through `melvin_m_process_input()`
4. Identified the double-free location

### The Bug
In `src/melvin.c`, function `melvin_m_process_input()`:

**Line 6770**: `pattern_nodes` array allocated
```c
pattern_nodes = malloc(data_size * sizeof(Node*));
```

**Line 6789**: `initial_nodes` created as an **alias** (not a copy) to `pattern_nodes`
```c
Node **initial_nodes = pattern_nodes;  // Just a pointer alias!
```

**Line 6941**: `initial_nodes` freed
```c
free(initial_nodes);  // Frees the memory
```

**Line 6961**: `pattern_nodes` freed **again**
```c
free(pattern_nodes);  // DOUBLE FREE! Same memory already freed
```

Since `initial_nodes` and `pattern_nodes` point to the **same memory**, freeing both causes a double-free crash.

## The Fix

**File**: `src/melvin.c`  
**Line**: 6941  
**Change**: Removed the `free(initial_nodes)` call

```c
// Before:
free(initial_nodes);

// After:
// NOTE: initial_nodes is just an alias to pattern_nodes, so don't free it here
// It will be freed later when we free pattern_nodes
// free(initial_nodes);  // REMOVED: This was causing double-free!
```

## Verification

After the fix:
- ✅ Test runs to completion without crashing
- ✅ No SIGABRT errors
- ✅ System processes 200 iterations successfully
- ✅ Hierarchies are created correctly
- ⚠️  Learning performance needs improvement (90.9% error rate)

## Test Output (Post-Fix)
```
Iteration | Nodes | Edges | Output Len | Error Rate | Status
----------|-------|-------|------------|------------|--------
       20 |     8 |    10 |          0 |     100.0% | = Same
       40 |     8 |    10 |          0 |     100.0% | = Same
      100 |    18 |    35 |         18 |      90.9% | ✓ IMPROVED
      200 |    18 |    35 |        218 |      90.9% | = Same

=== Results ===
Initial error rate: 100.0%
Best error rate:    90.9%
Improvements:       1 times
Error reduction:    9.1%

=== Verdict ===
⚠️  PARTIAL: Some improvement (9.1% reduction)
   System is learning but slowly
```

## Lessons Learned

1. **Pointer Aliasing**: When creating pointer aliases (not copies), only free the memory once
2. **Debug Instrumentation**: stderr with fflush() is critical for debugging crashes
3. **Memory Ownership**: Clear ownership of dynamically allocated memory prevents double-frees
4. **Code Review**: Look for patterns where pointers are assigned (not allocated) and then freed

## Next Steps

1. ✅ **COMPLETED**: Fix SIGABRT crash
2. 🔄 **IN PROGRESS**: Improve learning performance (currently 90.9% error)
3. ⏳ **PENDING**: Fix hierarchy matching (hierarchies created but not used for output)

