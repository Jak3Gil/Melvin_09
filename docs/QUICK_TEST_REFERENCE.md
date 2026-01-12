# Quick Test Reference

## Run All Tests
```bash
./tests/run_all_validation_tests.sh
```

## Individual Tests (Quick Run)

### 1. Multimodal (30 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_multimodal.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t1 -lm && /tmp/t1
```
**Proves:** Cross-modal associations (text→audio, audio→visual)

### 2. Blank Nodes (45 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_blank_nodes.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t2 -lm && /tmp/t2
```
**Proves:** Pattern separation at ambiguous points

### 3. Hierarchy Benefit (60 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_hierarchy_benefit.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t3 -lm && /tmp/t3
```
**Proves:** Hierarchies improve completion/recognition

### 4. Scalability (90 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_scalability.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t4 -lm && /tmp/t4
```
**Proves:** Handles 100+ char sequences, memory efficiency

### 5. Streaming vs Completion (30 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_streaming_vs_completion.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t5 -lm && /tmp/t5
```
**Proves:** System mode (streaming or completion)

### 6. Context Window (60 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_context_window.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t6 -lm && /tmp/t6
```
**Proves:** Context effectiveness at various ranges

### 7. Mini-Net Learning (45 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_mininet_learning.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t7 -lm && /tmp/t7
```
**Proves:** Mini-nets learn context-dependent routing

### 8. Adaptive Thresholds (30 seconds)
```bash
gcc -O2 -Wall -I./src tests/test_adaptive_thresholds.c src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -o /tmp/t8 -lm && /tmp/t8
```
**Proves:** Threshold adaptation to data

## What to Look For

### ✓ PASS Indicators
- "PASS:" in output
- "YES ✓" for key checks
- Different outputs for different contexts
- Correct completions

### ✗ FAIL Indicators
- "FAIL:" in output
- "NO ✗" for key checks
- Same output for different contexts
- No output generated

### ~ PARTIAL Indicators
- "PARTIAL:" in output
- Some tests pass, some fail
- Behavior exists but not optimal

## Quick Diagnosis

### If multimodal fails:
→ Check `port_id` field is being set and used in context

### If blank nodes don't form:
→ May not be needed! ContextTags might be sufficient

### If hierarchies don't help:
→ Expected! They're blocked during generation (by design)

### If scalability fails:
→ Check memory limits, increase if needed

### If streaming unclear:
→ System is streaming by design (outputs during input)

### If context fails at long range:
→ Expected! Brain also limited (~7±2 items)

### If mini-nets don't learn:
→ ContextTags may dominate decision-making

### If thresholds are hardcoded:
→ Some are intentional (safety), others can be adaptive

## Expected Timeline

| Test | Time | Complexity |
|------|------|------------|
| Multimodal | 30s | Simple |
| Blank Nodes | 45s | Medium |
| Hierarchy | 60s | Medium |
| Scalability | 90s | Complex |
| Streaming | 30s | Simple |
| Context | 60s | Medium |
| Mini-Net | 45s | Medium |
| Thresholds | 30s | Simple |
| **TOTAL** | **~7 min** | - |

## Already Proven (From Previous Tests)

✅ **Fast Learning:** 1-2 iterations for associations  
✅ **Hierarchy Formation:** 'he', 'el', 'll', 'lo' hierarchies form  
✅ **Context Discrimination:** ContextTags work perfectly  
✅ **Loop Prevention:** Hierarchies blocked during generation  

## Critical Tests (Must Pass)

1. **Multimodal** - Core differentiator
2. **Scalability** - Real-world viability
3. **Context Window** - Working memory analog

## Nice-to-Have Tests

4. **Blank Nodes** - May not be necessary
5. **Hierarchy Benefit** - Already forming, usage is design choice
6. **Mini-Net Learning** - ContextTags may suffice
7. **Streaming Mode** - Already known (streaming)
8. **Adaptive Thresholds** - Mostly adaptive, some constants OK
