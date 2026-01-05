# Testing Guide: Pure Data-Driven Output Generation

## Overview

With the new pure data-driven output generation, testing focuses on **adaptive behavior** rather than fixed outputs. The system's behavior evolves with the data it learns.

## Key Testing Principles

### 1. **No Fixed Outputs**
- ❌ Don't expect: Same input → same output
- ✅ Expect: Same input → behavior adapts based on graph maturity
- ✅ Expect: More training → longer, more coherent outputs

### 2. **Natural Convergence**
- Output length varies based on:
  - Pattern maturity (new patterns → shorter outputs)
  - Energy levels (high activation → longer outputs)
  - Graph connectivity (more edges → more paths)

### 3. **Data-Driven Growth**
- Everything scales with data:
  - Buffer sizes grow from usage
  - Temperature adapts to variance
  - Stopping happens naturally

## Test Scenarios

### Test 1: Fresh Graph (No Training)

**What to test:**
- Output generation works (no crashes)
- Short outputs are expected (graph is new)
- System learns patterns

**Command:**
```bash
rm -f test.m
./test_simple
```

**Expected:**
```
Input: 'hello'
Output size: 1-5 bytes  # Short output, graph is new
Output: [some bytes]     # May not be readable text yet
```

**What to verify:**
- ✅ No crashes or errors
- ✅ Output is generated (even if short)
- ✅ Graph grows (nodes/edges increase)

---

### Test 2: Progressive Learning

**What to test:**
- Output evolves as graph learns
- Pattern maturity affects output length
- System adapts over time

**Command:**
```bash
rm -f brain.m
./test_dataset
# Then type multiple times:
hello
hello
hello world
quit
```

**Expected progression:**
```
Input 1: "hello"
  Nodes: 12, Edges: 15
  Output: 1-2 bytes (new pattern, low maturity)

Input 2: "hello"  # Repeated pattern
  Nodes: 13, Edges: 16
  Output: 1-3 bytes (pattern stronger, more connections)

Input 3: "hello"
  Nodes: 14, Edges: 17
  Output: 2-4 bytes (pattern maturing)

Input 4: "hello world"
  Nodes: 21, Edges: 28
  Output: 3-10 bytes (longer input, more patterns)
```

**What to verify:**
- ✅ Output length varies (data-driven)
- ✅ Graph grows with each input
- ✅ Repeated patterns strengthen connections
- ✅ Longer inputs create more complex graphs

---

### Test 3: Adaptive Buffer Growth

**What to test:**
- Buffers grow from actual usage (not hardcoded)
- No memory errors
- Growth adapts to data

**How to verify:**
1. Process a long text file
2. Check memory usage doesn't explode
3. Verify buffers grow smoothly

**Command:**
```bash
rm -f brain.m
./test_dataset test_input.txt brain.m
```

**What to verify:**
- ✅ No buffer overflows
- ✅ Memory usage grows gradually
- ✅ System handles variable-length inputs

---

### Test 4: Temperature Adaptation

**What to test:**
- Temperature varies with data variance
- High variance → higher temperature (more exploration)
- Low variance → lower temperature (more focused)

**How to test:**
- Process diverse inputs → expect more variation
- Process repetitive inputs → expect more focused outputs

**Expected:**
```
Diverse inputs: "hello", "world", "test", "data"
  → Higher temperature (more exploration)
  → More varied outputs

Repetitive inputs: "hello", "hello", "hello"
  → Lower temperature (more focused)
  → More consistent outputs
```

---

### Test 5: Natural Convergence

**What to test:**
- Output stops naturally (energy dissipation)
- Not limited by hardcoded max bytes
- Convergence adapts to context

**How to verify:**
1. Feed a long input that creates many paths
2. Observe where output naturally stops
3. Repeat with different inputs
4. Verify stopping point varies (data-driven)

**Expected:**
- Output length varies by input complexity
- Stops when energy naturally dissipates
- No artificial truncation

---

### Test 6: Repetition Penalty

**What to test:**
- System avoids repeating same bytes too often
- Penalty is data-driven (from recent probabilities)
- Time decay based on position

**How to test:**
```bash
# Create a file with repetitive pattern
echo "aaa aaa aaa aaa aaa" > repetitive.txt
./test_dataset repetitive.txt brain.m
```

**Expected:**
- System generates output
- Repetition is reduced (but not eliminated - probabilistic)
- Penalty adapts based on recent output

---

### Test 7: Energy Dissipation

**What to test:**
- Output continues until energy naturally dissipates
- Convergence ratio computed from actual energy
- No hardcoded stopping thresholds

**How to verify:**
- Monitor output generation steps
- Check convergence happens naturally
- Verify energy ratios are data-driven

---

## Metrics to Monitor

### 1. **Output Length Distribution**
```
What: Length of outputs over time
Why: Shows if system is learning (longer outputs = mature patterns)
```

### 2. **Graph Growth Rate**
```
What: Nodes and edges per input
Why: Verifies learning and pattern formation
```

### 3. **Energy Levels**
```
What: Initial vs final energy per generation
Why: Shows natural convergence in action
```

### 4. **Temperature Variation**
```
What: Temperature values during generation
Why: Confirms temperature adapts to data variance
```

### 5. **Buffer Growth Patterns**
```
What: How buffers grow over time
Why: Verifies growth is data-driven, not fixed
```

## Expected Behaviors (vs Bugs)

### ✅ **Expected (Normal)**
- Short outputs for new graphs
- Variable output length (data-driven)
- Different outputs for same input (probabilistic)
- Outputs evolve as graph matures
- Natural stopping (no artificial limits)

### ❌ **Bug (Report)**
- Crashes or segfaults
- Infinite loops (never stops generating)
- Output always exactly 1 byte (might indicate issue)
- Memory leaks (continuous growth without data)
- Fixed output length (hardcoded limit still present)

## Debugging Tips

### If output is always 1 byte:
1. Check graph is growing (nodes/edges increasing)
2. Verify patterns are being learned
3. May need more training data
4. Check energy dissipation logic

### If output never stops:
1. Check convergence ratio computation
2. Verify energy dissipation threshold
3. Look for infinite loops in generation

### If temperature seems fixed:
1. Check activation variance computation
2. Verify temperature calculation uses data
3. Check for hardcoded clamping

## Test Scripts

### Quick Test
```bash
make clean && make all
./test_simple
```

### Learning Test
```bash
./watch_learning
```

### Interactive Test
```bash
./test_dataset
```

### File Test
```bash
./test_dataset test_input.txt brain.m
```

## Current Status

**What works:**
- ✅ Pure data-driven generation (no hardcoded values)
- ✅ Natural convergence (energy dissipation)
- ✅ Adaptive growth (buffers, temperature)
- ✅ Progressive learning (outputs evolve)

**What to expect:**
- Short outputs initially (new graph)
- Outputs grow as graph matures
- Variable behavior (probabilistic, data-driven)
- Natural stopping (no hard limits)

**Next steps:**
- Train with more data to see longer outputs
- Test with larger datasets
- Monitor learning progression over time

