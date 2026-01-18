# Requirement Compliance Documentation

## Purpose

This document ensures the system works **exactly** as laid out in README.md and Requirement.md. It prevents violations by documenting the correct approach for every decision.

## Core Requirements

### 1. All Values Must Be Relative (Requirement.md line 6-10)

> "All thresholds must be relative to local context (not absolute values)"

**What this means:**
- NO absolute comparisons: `if (value > 0.5f)` ❌
- YES relative comparisons: `if (value / local_avg > 1.0f)` ✅
- All edge weights compared relative to `local_avg`
- All activations compared relative to `max_activation`
- All scores compared relative to each other

**Examples:**

❌ **WRONG** (absolute):
```c
float context_weight = base_weight * context_multiplier;  // Returns absolute value
if (activation > 0.5f) { ... }  // Hardcoded threshold
```

✅ **CORRECT** (relative):
```c
float context_boost = context_multiplier;  // Returns relative multiplier [1.0, ~3.0]
float base_relative = edge->weight / local_avg;  // Relative to local average
float context_relative = base_relative * context_boost;  // Context changes edge weight
if (activation / max_activation > relative_threshold) { ... }  // Relative comparison
```

---

### 2. Context Changes Edge Weights (Requirement.md line 11)

> "context is a payload, of that activated nodes from a input and the desicions made, the current node holds the context of the last x number, that context changes the edge weights of the current node"

**What this means:**
- Context temporarily modifies edge weights (doesn't replace them)
- Start with base relative weight: `edge->weight / local_avg`
- Context applies multiplier: `base_relative * context_boost`
- Result is still relative (not absolute)

**Implementation:**

✅ **CORRECT**:
```c
// 1. Compute base relative weight
float base_weight = (float)edge->weight / 255.0f;
float local_avg = node_get_local_outgoing_weight_avg(node);
float base_relative = base_weight / local_avg;  // Relative [0.5, 2.0, etc.]

// 2. Context changes it (multiplier)
float context_boost = edge_compute_context_weight(edge, context);  // [1.0, ~3.0]
float context_relative = base_relative * context_boost;  // Context changes edge weight

// 3. Use context-modified relative weight
score = context_relative + other_factors;
```

❌ **WRONG** (violates requirement):
```c
// Returns absolute weight, not relative boost
float context_weight = edge_compute_context_weight(edge, context);  // Returns absolute
float score = context_weight + base_weight;  // Mixing absolute values
```

---

### 3. No Hardcoded Thresholds (Requirement.md line 4)

> "No hardcoded thresholds"

**What this means:**
- NO: `if (value > 0.5f)` ❌
- YES: `if (value > compute_adaptive_threshold())` ✅
- All thresholds computed from data
- Thresholds adapt to local context

**Examples:**

❌ **WRONG**:
```c
float threshold = 0.5f;  // Hardcoded
if (formation_prob > threshold) { ... }
```

✅ **CORRECT**:
```c
// Threshold from data (variance, local avg, etc.)
float variance_norm = variance / (max_value * max_value + 0.001f);
float threshold = 0.1f + (variance_norm * 0.3f);  // Adapts to data
if (relative_strength > threshold) { ... }
```

---

### 4. Edges Compete Relative to Local Average (Requirement.md line 8)

> "Edge selection: edges compete relative to local average, not absolute threshold"

**What this means:**
- Compute `local_avg = sum(edge_weights) / edge_count`
- Each edge's strength: `edge->weight / local_avg`
- Edges compete relative to each other
- Winner is strongest relative to local average

**Implementation:**

✅ **CORRECT**:
```c
float local_avg = node_get_local_outgoing_weight_avg(node);
float base_relative = edge->weight / local_avg;  // Relative strength
float context_relative = base_relative * context_boost;  // Context modifies it

// All edges compete with relative values
// Winner = highest context_relative
```

❌ **WRONG**:
```c
float context_weight = base_weight * context_multiplier;  // Absolute
float score = context_weight + base_weight;  // Absolute values
// Edges compete with absolute values (violates requirement)
```

---

### 5. Mini-Nets Receive Only Relative Values

**What this means:**
- Mini-nets learn patterns in relative space
- Absolute values confuse learning (different scales)
- All inputs must be relative or normalized

**Implementation:**

✅ **CORRECT**:
```c
float inputs[] = {
    context_relative,   // ✅ RELATIVE (base * context boost)
    base_relative,      // ✅ RELATIVE (edge / local_avg)
    activation,         // ✅ RELATIVE (from pattern)
    context_boost,      // ✅ RELATIVE (multiplier)
    local_avg,          // ✅ CONTEXT (for reference)
    context_count,      // ✅ CONTEXT
    context_level       // ✅ CONTEXT
};
```

❌ **WRONG**:
```c
float inputs[] = {
    context_weight,     // ❌ ABSOLUTE (base * multiplier)
    base_weight,        // ❌ ABSOLUTE
    relative_strength,  // ✅ RELATIVE
    // Mixing absolute and relative confuses mini-net!
};
```

---

## Function Reference

### `edge_compute_context_weight(Edge *edge, SparseContext *context)`

**Purpose**: Compute how much context boosts this edge

**Returns**: Relative boost factor [1.0, ~3.0]
- 1.0 = no boost (context doesn't match)
- 2.0 = double strength (context matches well)
- 3.0 = triple strength (perfect match + hierarchies + attention)

**NOT**: Absolute weight (violates requirements)

**Implementation**:
```c
// Softmax over context tags → probability
// Combine with level boost, adaptive mult, attention
// Return: context_multiplier (1.0 + context_boost)
return context_multiplier;  // ✅ RELATIVE
```

---

### Edge Selection Pattern

**Correct Pattern** (use everywhere):
```c
// 1. Get relative base strength
float base_weight = (float)edge->weight / 255.0f;
float local_avg = node_get_local_outgoing_weight_avg(node);
float epsilon = compute_adaptive_epsilon(local_avg);
float base_relative = (local_avg > epsilon) ? (base_weight / local_avg) : base_weight;

// 2. Get context boost (relative multiplier)
float context_boost = edge_compute_context_weight(edge, context);  // [1.0, ~3.0]

// 3. Context changes edge weight (requirement line 11)
float context_relative = base_relative * context_boost;

// 4. Add other relative factors
float activation = ...;  // Relative from pattern
float score = context_relative + activation;

// 5. All values are relative - edges compete fairly!
```

---

## Checklist for New Code

Before adding any new code, verify:

- [ ] All comparisons are relative (no absolute thresholds)
- [ ] Edge weights compared to `local_avg` (not absolute)
- [ ] Activations compared to `max_activation` (not absolute)
- [ ] `edge_compute_context_weight()` returns relative boost, not absolute weight
- [ ] Mini-net inputs are all relative or normalized
- [ ] No hardcoded thresholds (0.5f, 0.7f, etc.)
- [ ] Context modifies edge weights (doesn't replace them)
- [ ] All values scale with data (not fixed)

---

## Common Violations to Avoid

### Violation 1: Returning Absolute Weight
```c
// ❌ WRONG
return base_weight * context_multiplier;  // Absolute value
```

**Fix**: Return relative boost factor
```c
// ✅ CORRECT
return context_multiplier;  // Relative [1.0, ~3.0]
```

### Violation 2: Mixing Absolute and Relative
```c
// ❌ WRONG
float inputs[] = {
    context_weight,      // Absolute
    relative_strength,   // Relative
    // Mixed scales!
};
```

**Fix**: Use only relative
```c
// ✅ CORRECT
float inputs[] = {
    context_relative,    // Relative
    base_relative,       // Relative
    // All same scale!
};
```

### Violation 3: Hardcoded Thresholds
```c
// ❌ WRONG
if (value > 0.5f) { ... }  // Hardcoded
```

**Fix**: Compute from data
```c
// ✅ CORRECT
float threshold = compute_adaptive_threshold(node);  // From data
if (value > threshold) { ... }
```

### Violation 4: Absolute Comparisons
```c
// ❌ WRONG
if (activation > 0.01f) { ... }  // Absolute threshold
```

**Fix**: Relative comparison
```c
// ✅ CORRECT
if (activation / max_activation > relative_threshold) { ... }
```

---

## Testing Compliance

When testing, verify:

1. **Relative Values**: All edge scores are relative to local_avg
2. **Context Effect**: Context actually changes edge selection
3. **No Hardcoded Limits**: System works with 1 node or 1 billion nodes
4. **Data-Driven**: Thresholds adapt to data distribution

---

## Summary

**Golden Rule**: Everything is relative to local context.

- Edge weights → relative to local_avg
- Activations → relative to max_activation
- Scores → relative to each other
- Context → modifies relative weights (doesn't replace them)

**Never**:
- Return absolute values from relative functions
- Mix absolute and relative in same computation
- Use hardcoded thresholds
- Compare to fixed values

**Always**:
- Compute relative to local context
- Use data-driven thresholds
- Scale with data
- Follow Requirement.md exactly
