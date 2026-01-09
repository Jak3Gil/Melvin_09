# Debug Plan: Double Free Issue

## Problem
Test crashes with "Double free of object" in `melvin_m_process_input`.

## Root Cause Analysis

The crash happens because there are multiple cleanup paths that might free the same `pattern` object.

Looking at the code flow in `melvin_m_process_input`:

1. Pattern is created: `pattern = encode_input_spreading(...)`
2. Pattern is used in refine/decode phases
3. Pattern is freed at line ~6846
4. Pattern pointer is set to NULL
5. BUT there might be other code paths that also try to free it

## Investigation Needed

Need to check:
1. Is `pattern` being freed in multiple places?
2. Is there a `melvin_m_cleanup_state` function that also frees pattern?
3. Are there error paths that might double-free?

## Solution

The fix should ensure pattern is only freed once, and the pointer is set to NULL immediately after freeing to prevent double-free.

## Current Status

- Fixed one double-free (removed duplicate free at line 6873)
- Still crashing - need to find other double-free location
- Using lldb shows crash in `melvin_m_process_input` at offset +13336

## Next Steps

1. Search for ALL places where `activation_pattern_free` is called
2. Check if there's cleanup code that runs on error paths
3. Ensure pattern pointer is always NULL after free
4. Consider using a cleanup helper that checks for NULL before freeing

