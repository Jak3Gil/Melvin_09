# Debugging Status

## Current Issue: Double Free in Test

The system crashes with "Double free of object" when running error rate tests.

## What We Know

1. **Crash location**: `melvin_m_process_input` at offset +13336
2. **Error**: "Double free of object 0x..."
3. **Already fixed**: Removed duplicate `activation_pattern_free()` call
4. **Still crashing**: There's another double-free somewhere

## Potential Causes

1. **Hash table cleanup**: The `activation_pattern_free()` function frees hash table buckets
2. **Uninitialized pointers**: Some hash buckets might not be properly initialized
3. **Multiple cleanup paths**: Error handling might free the same memory twice

## Fixes Applied

1. ✅ Initialized graph mini nets (refine_net, decode_net, hierarchy_net)
2. ✅ Added proper cleanup for graph mini nets in `melvin_m_close()`
3. ✅ Removed duplicate `activation_pattern_free()` call
4. ✅ Added NULL checks in `activation_pattern_create()`

## Current Hypothesis

The hash table in `ActivationPattern` might have uninitialized buckets that are being freed.

When we do:
```c
pattern->node_hash = calloc(pattern->hash_size, sizeof(Node**));
```

This creates an array of NULL pointers. Then when we free:
```c
for (size_t i = 0; i < pattern->hash_size; i++) {
    free(pattern->node_hash[i]);  // This should be safe for NULL
}
```

This should be safe because `free(NULL)` is a no-op. But maybe there's corruption elsewhere.

## Alternative Theory

The double-free might be happening because:
1. Pattern is created
2. Some operation partially initializes it
3. An error occurs
4. Cleanup tries to free partially-initialized memory
5. Later, another cleanup tries to free the same memory

## Next Steps

1. Add more robust NULL checking in allocation
2. Consider using a cleanup helper that sets pointers to NULL after freeing
3. Run with Address Sanitizer to get better error information
4. Simplify the test to isolate the issue

## Status

**In Progress** - Still debugging the double-free issue.

