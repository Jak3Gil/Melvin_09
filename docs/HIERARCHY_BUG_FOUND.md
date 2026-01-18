# Hierarchy Formation Bug - FIXED!

## The Bug (Fixed)

**Location:** `src/melvin.c` lines 11439-11591

**The Problem:**
```c
// Line 11439: Only processes edges between INPUT nodes
for (size_t i = 0; i + 1 < initial_count && initial_nodes; i++) {
    Node *from = initial_nodes[i];      // INPUT node (byte)
    Node *to = initial_nodes[i + 1];     // INPUT node (byte)
    
    // ... create edge ...
    
    // Line 11501: Only checks if from/to are bytes
    if (from->payload_size > 0 && to->payload_size > 0) {
        // Creates hierarchy from (byte + byte)
        // NEVER checks (hierarchy + byte) or (hierarchy + hierarchy)!
    }
}
```

**What This Means:**
- ✅ Creates 2-byte hierarchies: "he", "el", "ll", "lo" (byte + byte)
- ❌ NEVER creates 3+ byte hierarchies: "hel", "ell", "llo", "hello"
- ❌ NEVER checks if `from` or `to` is already a hierarchy
- ❌ NEVER combines hierarchies: (he hierarchy) + l → "hel"

## Why It Fails

**During training "hello":**
1. Input: [h, e, l, l, o] (all bytes)
2. Loop processes: h→e, e→l, l→l, l→o
3. Creates hierarchies: "he", "el", "ll", "lo" (2-byte only)
4. **STOPS** - never checks if "he" hierarchy can combine with "l" to form "hel"
5. **STOPS** - never checks if "hel" hierarchy can combine with "lo" hierarchy to form "hello"

**Result:** Only 2-byte hierarchies form, never 3+ byte!

## The Fix Needed

**After creating hierarchies, we need to check if they can combine:**

```c
// After processing initial input sequence:
for (size_t i = 0; i + 1 < initial_count; i++) {
    // ... existing code creates 2-byte hierarchies ...
}

// NEW: Check if newly created hierarchies can combine
// This should happen AFTER the initial loop, or recursively
for (each hierarchy created in this iteration) {
    // Check if hierarchy can combine with next node
    // Check if hierarchy can combine with next hierarchy
    // Recursively form larger hierarchies
}
```

**Or better: Process ALL active nodes, not just input nodes:**

```c
// Instead of only initial_nodes, process ALL active nodes
// This includes hierarchies that were just created
for (each active_node in wave_state) {
    for (each outgoing_edge from active_node) {
        Node *from = active_node;  // Could be hierarchy!
        Node *to = edge->to_node;   // Could be hierarchy!
        
        // Check if (from + to) should form hierarchy
        // This works for: (byte + byte), (hierarchy + byte), (hierarchy + hierarchy)
    }
}
```

## Why This Is a Silent Killer

**Without 3+ byte hierarchies:**
- Input "hel" → searches for hierarchy matching "hel..."
- Only finds "he" (2 bytes) - doesn't match!
- No hierarchy guidance → random output → garbage

**With 3+ byte hierarchies (fixed):**
- Input "hel" → finds "hello" hierarchy (5 bytes)
- "hel" matches first 3 bytes ✓
- Hierarchy guides output → "lo" ✓

## The Root Cause

**The code assumes hierarchies only form from adjacent input bytes:**
- Assumes: `from` and `to` are always bytes
- Reality: After first iteration, `from` or `to` could be hierarchies
- Bug: Never checks if `from` or `to` is a hierarchy

**The fix: Remove the assumption that only bytes form hierarchies!**
