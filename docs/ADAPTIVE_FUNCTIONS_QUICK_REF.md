# Adaptive Functions Quick Reference

## Brain-Inspired Adaptive System

All hardcoded constants have been replaced with adaptive functions that learn from data.

---

## Core Statistics Functions (O(1))

```c
// Welford's online algorithm - updates running mean and variance in O(1)
void update_running_stats(float *mean, float *m2, uint64_t *count, float new_value)

// Extract variance from M2 (Bessel's correction)
float get_running_variance(float m2, uint64_t count)

// Standard deviation
float get_running_stddev(float m2, uint64_t count)
```

**Usage**: Called automatically during operations to maintain running statistics.

---

## Recording Functions (Call During Operations)

```c
// Record activation for adaptive thresholds
void graph_record_activation(Graph *graph, float activation)

// Record confidence for adaptive thresholds  
void graph_record_confidence(Graph *graph, float confidence)

// Record error for adaptive thresholds
void graph_record_error(Graph *graph, float error)

// Record path length for adaptive limits
void graph_record_path_length(Graph *graph, size_t path_len)
```

**When to Call**:
- `graph_record_activation()`: During node activation
- `graph_record_confidence()`: During edge selection
- `graph_record_error()`: During error feedback
- `graph_record_path_length()`: After output generation

---

## Adaptive Threshold Functions

### 1. Generic Threshold (Percentile-Based)
```c
float compute_adaptive_threshold_from_stats(
    float mean, float m2, uint64_t count, float percentile)
```
- `percentile`: 0.0 = min, 0.5 = median, 1.0 = max
- Returns: threshold value at given percentile
- **Example**: `compute_adaptive_threshold_from_stats(mean, m2, count, 0.25f)` returns 25th percentile

### 2. Error Threshold
```c
float compute_adaptive_error_threshold(Graph *graph)
```
- Returns: mean error (errors above mean are significant)
- **Replaces**: Hardcoded `0.5f`

### 3. Confidence Threshold
```c
float compute_adaptive_confidence_threshold(Graph *graph, float percentile)
```
- `percentile`: 0.25 = lenient, 0.5 = median, 0.75 = strict
- **Replaces**: Hardcoded `0.3f`, `0.5f`

### 4. Activation Threshold
```c
float compute_adaptive_activation_threshold(Graph *graph, float percentile)
```
- `percentile`: 0.1 = keep top 90%, 0.5 = keep top 50%
- **Replaces**: Hardcoded `0.01f`, `0.1f`

---

## Adaptive Limit Functions

### 1. Neighbor Iteration Limit
```c
size_t compute_adaptive_neighbor_limit(Node *node, Graph *graph)
```
- Formula: `sqrt(degree) * (1 + abstraction_level) * graph_factor`
- **Replaces**: Hardcoded `5`, `10`, `20`
- **Brain**: More connected neurons sample more neighbors

### 2. Output Length Limit
```c
size_t compute_adaptive_output_limit(size_t input_len, Graph *graph)
```
- Formula: `input_len * 4 * (0.5 + maturity) * path_factor * graph_factor`
- **Replaces**: Hardcoded `64`, `256`
- **Brain**: Response duration scales with stimulus and capacity

### 3. Cycle Detection Window
```c
size_t compute_adaptive_cycle_window(Graph *graph)
```
- Formula: `2 * (mean_path + 2*stddev_path)`
- **Replaces**: Hardcoded `16`, `64`, `256`
- **Brain**: Working memory capacity adapts to task complexity

### 4. Pattern Match Limit
```c
size_t compute_adaptive_pattern_limit(Graph *graph)
```
- Formula: `max_hierarchy_length + 4`
- **Replaces**: Hardcoded `20`
- **Brain**: Pattern recognition scales with hierarchy depth

---

## Adaptive Bounds Functions

### 1. Weight Floor
```c
uint8_t compute_adaptive_weight_floor(Node *node)
```
- Formula: `local_avg * 0.1`, minimum 1
- **Replaces**: Hardcoded `10`
- **Brain**: Synaptic depression has minimum based on local context

### 2. Weight Ceiling
```c
float compute_adaptive_weight_ceiling(Node *node)
```
- Formula: `local_avg * 2.0`, maximum 255
- **Replaces**: Hardcoded `10.0f`
- **Brain**: Synaptic potentiation has ceiling to prevent dominance

### 3. Rate Bounds
```c
void compute_adaptive_rate_bounds(Graph *graph, float *min_rate, float *max_rate)
```
- Formula: `mean ± 2*stddev`, clamped to [0, 1]
- **Replaces**: Hardcoded `0.05f`, `0.95f`
- **Brain**: Learning rate bounded by observed variance

---

## Usage Examples

### Example 1: Adaptive Neighbor Sampling
```c
// OLD (hardcoded):
for (size_t i = 0; i < node->outgoing_count && i < 10; i++)

// NEW (adaptive):
size_t limit = compute_adaptive_neighbor_limit(node, graph);
for (size_t i = 0; i < node->outgoing_count && i < limit; i++)
```

### Example 2: Adaptive Confidence Threshold
```c
// OLD (hardcoded):
if (confidence < 0.5f) break;

// NEW (adaptive):
graph_record_confidence(graph, confidence);
float threshold = compute_adaptive_confidence_threshold(graph, 0.5f);
if (confidence < threshold) break;
```

### Example 3: Adaptive Output Limit
```c
// OLD (hardcoded):
size_t max_len = 256;

// NEW (adaptive):
size_t max_len = compute_adaptive_output_limit(input_len, graph);
```

### Example 4: Adaptive Weight Bounds
```c
// OLD (hardcoded):
if (edge->weight < 10) edge->weight = 10;

// NEW (adaptive):
uint8_t floor = compute_adaptive_weight_floor(edge->from_node);
if (edge->weight < floor) edge->weight = floor;
```

---

## Bootstrap Behavior

All functions have sensible bootstrap behavior when no data is available:

| Function | Bootstrap Value | Reason |
|----------|----------------|---------|
| Error threshold | 0.5 | Neutral (50/50) |
| Confidence threshold | 0.5 | Median assumption |
| Activation threshold | 0.01 | Low (inclusive) |
| Neighbor limit | 1 | Minimum safe |
| Output limit | input_len * 4 | Reasonable expansion |
| Cycle window | 4 | Detect simple cycles |
| Weight floor | 1 | Absolute minimum |
| Weight ceiling | 255.0 | Maximum possible |
| Rate bounds | [0.0, 1.0] | Full range |

**After ~10-100 samples**, all functions adapt to actual data distribution.

---

## Performance Characteristics

| Function | Time Complexity | Space Complexity |
|----------|----------------|------------------|
| `update_running_stats()` | O(1) | O(1) |
| `compute_adaptive_threshold_*()` | O(1) | O(1) |
| `compute_adaptive_neighbor_limit()` | O(1) | O(1) |
| `compute_adaptive_output_limit()` | O(1) | O(1) |
| `compute_adaptive_cycle_window()` | O(1) | O(1) |
| `compute_adaptive_weight_*()` | O(1) | O(1) |
| `compute_adaptive_rate_bounds()` | O(1) | O(1) |
| `compute_adaptive_pattern_limit()` | O(levels * samples) | O(1) |

**Note**: `compute_adaptive_pattern_limit()` samples a few nodes per hierarchy level (not O(n)).

---

## Memory Overhead

Per `Graph` struct:
- 4 running means (4 floats = 16 bytes)
- 4 running M2 values (4 floats = 16 bytes)
- 4 sample counts (4 uint64_t = 32 bytes)
- 2 adaptive factors (2 floats = 8 bytes)

**Total**: 72 bytes per graph (negligible)

---

## Debugging Tips

### 1. Print Statistics
```c
printf("Activation: mean=%.3f stddev=%.3f samples=%lu\n",
       graph->running_activation_mean,
       get_running_stddev(graph->running_activation_m2, 
                         graph->activation_sample_count),
       graph->activation_sample_count);
```

### 2. Check Bootstrap Phase
```c
if (graph->activation_sample_count < 10) {
    printf("WARNING: Still in bootstrap phase (< 10 samples)\n");
}
```

### 3. Verify Adaptive Limits
```c
size_t limit = compute_adaptive_neighbor_limit(node, graph);
printf("Node degree=%zu, adaptive_limit=%zu\n", 
       node->outgoing_count, limit);
```

---

## Migration Guide

### Step 1: Identify Hardcoded Value
```c
if (value < 0.5f) { ... }  // Hardcoded threshold
```

### Step 2: Find Appropriate Adaptive Function
- Threshold? → `compute_adaptive_*_threshold()`
- Limit? → `compute_adaptive_*_limit()`
- Bound? → `compute_adaptive_*_bounds()`

### Step 3: Add Recording Call
```c
graph_record_confidence(graph, value);  // Record for statistics
```

### Step 4: Replace with Adaptive Call
```c
float threshold = compute_adaptive_confidence_threshold(graph, 0.5f);
if (value < threshold) { ... }
```

---

## Key Principles

1. **No Hardcoded Constants**: Every threshold/limit emerges from data
2. **O(1) Operations**: All adaptive functions use cached statistics
3. **Local Computation**: Decisions based on local context, not global state
4. **Graceful Bootstrap**: Sensible defaults when no data available
5. **Continuous Adaptation**: Statistics update incrementally during operation

---

## Neuroscience Parallels

| Biological Mechanism | Implementation |
|---------------------|----------------|
| Homeostatic plasticity | Running statistics maintain stable behavior |
| Synaptic scaling | Weight bounds relative to local activity |
| Adaptive thresholding | Firing threshold varies with recent activity |
| Working memory capacity | Cycle window adapts to task complexity |
| Dendritic arbor size | Neighbor limit scales with connectivity |

**Result**: System behaves like biological neural network - adapts to its environment without hardcoded parameters.
