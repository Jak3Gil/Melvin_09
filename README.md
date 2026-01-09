# Melvin: Emergent Intelligence System

## Quick Start

**Production System Only** - There is no separate test infrastructure. The production system IS the test system.

```bash
# Build
make all

# Feed any dataset (auto-detects type and routes to correct port)
./melvin_feed dataset.txt brain.m
./melvin_feed audio.wav brain.m
./melvin_feed image.jpg brain.m
./melvin_feed code.c brain.m

# Advanced usage
./melvin_feed corpus.txt brain.m --passes 3 --watch
./melvin_feed data.bin brain.m --port 0x01

# Watch brain in real-time (separate terminal)
./melvin_watch brain.m

# Test multi-modal port system
./tests/test_port_system
```

See [PRODUCTION.md](PRODUCTION.md) for complete production system documentation.
See [PORT_SYSTEM.md](docs/PORT_SYSTEM.md) for multi-modal port system documentation.
See [AI_ACCELERATION.md](docs/AI_ACCELERATION.md) for AI model integration and self-regulation.

---

## Vision

Melvin is a **universal intelligence system** that learns through biological-like mechanisms (Hebbian learning) rather than backpropagation. It aims to:

1. **Scale efficiently** through structure, not brute force compute
2. **Learn continuously** without retraining phases
3. **Make all decisions locally** (no global state needed)
4. **Adapt to any data or environment** without hardcoded limits
5. **Build intelligence through compounding** (knowledge builds on knowledge)

The system uses a graph-based architecture where **nodes** act as mini neural nets and **edges** act as mini transformers. Intelligence emerges from local interactions of many simple components following simple rules.

**Key Difference from LLMs**:
- LLMs: Dense attention (O(n²)), global backpropagation, fixed architecture, implicit knowledge
- Melvin: Sparse routing (O(degree)), local Hebbian learning, dynamic structure, explicit knowledge

---

## Core Principles

### 1. Self-Regulation Through Local Measurements Only

Every decision uses only information available to the node itself and its immediate neighbors. No global state needed.

- Nodes only know themselves and their edges
- All decisions are relative to local context
- Enables O(1) per-node computational cost
- Scales to billions of nodes efficiently
- Self-organization emerges from local interactions

### 2. No Hardcoded Limits or Thresholds

All thresholds and limits emerge from the data itself, never from programmer decisions. Values are **data-driven** (computed from model outputs), like LLMs use softmax over logits.

- No magic numbers (no `0.5f`, `0.7f`, etc. as hardcoded thresholds)
- Probabilities computed from model outputs (activation_strength, edge weights) - data-driven like LLM logits
- Thresholds computed relative to local context (from actual data distributions)
- Smooth probability functions (continuous, differentiable) but values come from data
- System scales from 1 byte to unlimited size naturally
- When no data exists: use minimal context or return 0.0f (neutral, not a threshold)

**Key Point**: Like LLMs compute probabilities from logits (data-driven), Melvin computes probabilities from activation_strength and edge weights (data-driven). The values come from the model state, not hardcoded assumptions.

### 3. Relative Adaptive Stability

Numerical stability mechanisms are adaptive and relative to data, not hardcoded.

- Epsilon scales with data range (not fixed `1e-6f`)
- Clipping bounds computed from local node/edge values
- Smoothing factors adapt to change rate
- All stability mechanisms use local context

### 4. Compounding Learning

Everything learned helps learn the next thing. Existing knowledge accelerates finding new patterns.

- Hierarchies enable matching larger patterns efficiently (10:1 compression per level)
- Blank nodes enable generalization through explicit categories
- Edge weights guide exploration to relevant patterns
- Knowledge compression: 1,000 patterns → 100 concepts → 10 meta-concepts → 1 super-concept

### 5. Adaptive Behavior

The system adapts its learning strategy based on context. Not purely linear or purely compounding - the compounding rate itself adapts.

- Exploratory behavior when in new territory (weaker connections)
- Compounding behavior when building on existing knowledge (stronger connections)
- Weight updates are direct (Hebbian learning) with decay providing natural bounds
- Pattern size limits adapt to local node sizes

### 6. Continuous Learning

The system learns on every operation. No train/test split. No retraining phase.

- Edge weights update immediately during wave propagation
- Node weights update immediately during wave propagation
- Unused edges/nodes decay naturally (intelligent forgetting)
- System improves over time automatically
- Can learn from live data streams

### 7. Emergent Intelligence

Intelligence emerges from interactions between nodes. No explicit algorithms for intelligence - it emerges naturally.

- Structure emerges from experience, not design
- No fixed architecture - grows organically
- Intelligence emerges from local interactions
- Self-organization through local rules

### 8. Explicit Hierarchical Abstraction

Abstraction levels are tracked explicitly. Hierarchies are concrete nodes, not implicit concepts.

- Hierarchies are inspectable nodes in the graph
- Abstraction levels tracked explicitly
- Enables compression through abstraction
- Transparent and debuggable

---

## System Workflow: Start to Finish

### Phase 1: Input Processing

1. **Receive Input**: Raw bytes arrive (text, device input, etc.)
2. **Pattern Matching & Node Creation**:
   - Process input byte-by-byte
   - Try to match existing nodes (hierarchy-first: larger patterns first)
   - If no match: create new node with byte/pattern as payload
   - Create sequential edges between consecutive nodes
   - Example: "hello" → creates edges: h→e, e→l, l→l, l→o

**Result**: Graph contains nodes representing the input sequence, connected by sequential edges.

### Phase 2: Three-Phase Intelligence (Encode → Refine → Decode)

This is the core "thinking" phase. Unlike traditional wave propagation that only collects input nodes, this three-phase architecture finds **continuations** (like LLMs predict next tokens).

**PHASE 2A: ENCODE (Input → Activation Pattern)**

Like LLM: input tokens → embeddings → hidden state
Like Brain: sensory input → neural activation pattern

1. **Direct Activation**: Input nodes activate with position-weighted strength
   - Later positions get higher activation (recency bias)
   - Example: "hello" → h(0.6), e(0.7), l(0.8), l(0.9), o(1.0)

2. **Spreading Activation**: Activation spreads through learned edges
   - **KEY INSIGHT**: This discovers continuation nodes like " world" after "hello"
   - Multi-hop spreading with decay: 50%, 25%, 12.5%...
   - Last input node's outgoing edges get strongest boost
   - Result: Pattern includes both input AND continuation candidates

3. **Context Vector**: Build weighted sum of all activations
   - Like LLM hidden state - encodes "meaning" of context
   - Used for context-aware edge selection

**Result**: ActivationPattern with input nodes AND spread continuation nodes.

**PHASE 2B: REFINE (Activation → Refined Activation)**

Like LLM: Transformer layers (attention + feedforward)
Like Brain: Recurrent cortical processing

1. **Recurrent Dynamics** (3 iterations):
   - Self-activation (momentum): nodes retain 60% activation
   - Neighbor input: connected nodes prime each other through spreading activation
   - Context fit: nodes matching context receive stronger activation (priming)
   
2. **Emergent Attention**: 
   - Nodes connected to active context strengthen
   - Disconnected nodes decay naturally
   - No explicit attention mechanism - it EMERGES

**Result**: Refined pattern where continuation nodes are ranked by relevance.

### Priming (Brain-Inspired)

Like the human brain, activation spreads unconditionally to all connected nodes:
- Strong connections receive large activation
- Weak connections receive small activation (priming)
- No filtering - all paths get a chance to compete
- Priming prevents catastrophic forgetting by keeping weak memories alive

**PHASE 2C: DECODE (Activation → Output)**

Like LLM: Sample from softmax distribution
Like Brain: Motor output from motor cortex

1. **Candidate Selection**: 
   - Exclude pure input nodes (we want continuations, not echoes)
   - Filter by activation threshold
   - Result: Candidates are nodes discovered via spreading

2. **Autoregressive Generation**:
   - First byte: highest activation candidate
   - Subsequent bytes: context-aware edge selection
   - Pattern matching: prefer edges that match training sequences
   - Loop detection: stop on repeating patterns

**Result**: Output bytes representing the learned continuation.

### Phase 3: Hebbian Learning

**Edge Strengthening** (during decode):
- Edges used during generation get strengthened
- `edge->weight += activation` (direct Hebbian learning)
- "Neurons that fire together, wire together"

**Structure Formation**:
- Strong co-occurring edges may form hierarchy nodes
- Hierarchy = combined payload (e.g., "he" + "llo" → "hello")
- **Compounding Effect**: Match larger patterns in O(1)

### Phase 4: Output & Persistence

1. **Write Output**: Generated bytes to output buffer
2. **Auto-Save**: Graph state persisted to `.m` file
3. **Cleanup**: Clear input buffer, free temporary state

### Phase 5: Persistence & Cleanup

1. **Auto-Save**: Graph state saved to `.m` file
2. **Cleanup**: Clear input buffer, free temporary state

---

## Key Mechanisms

### Biological Learning (Not Backpropagation)

- Edges strengthen when activated: `edge->weight += activation` (direct Hebbian learning)
- Nodes strengthen when activated: `node->weight += activation` (direct Hebbian learning)
- Happens immediately during wave propagation
- No gradients, no error signals, no backpropagation
- No learning rate - decay and competition provide natural bounds
- "Neurons that fire together, wire together"

### Local-Only Operations

- Every decision uses only node's properties + immediate neighbors
- No global state needed
- Enables O(1) per-node complexity
- Scales efficiently

### Adaptive Everything

- Weight updates are direct and immediate (pure Hebbian learning)
- Thresholds adapt from local context
- Pattern sizes adapt from local node sizes
- No hardcoded values

### Compounding Learning

- Hierarchies enable matching larger patterns faster
- Blank nodes enable generalization
- Each thing learned helps learn the next thing
- Knowledge compression through abstraction

---

## Examples

### Example 1: Learning "Hello"

**Input**: "hello"

**What Happens**:
1. Creates nodes: 'h', 'e', 'l', 'l', 'o'
2. Creates sequential edges: h→e, e→l, l→l, l→o
3. Wave propagation explores graph, updates weights
4. On second "hello" input: edges strengthen (co-activation)
5. After many repetitions: creates hierarchy node "hello"
6. Can now match "hello" in 1 step instead of 5 steps

**Compounding Effect**:
- First time: 5 nodes created, 5 edges created
- Tenth time: 0 new nodes, edges strengthened, hierarchy created
- System gets faster at recognizing "hello"

### Example 2: Learning Categories

**Inputs**: "cat", "dog", "bird"

**What Happens**:
1. Creates nodes for each word: c-a-t, d-o-g, b-i-r-d
2. Wave propagation finds patterns are similar
3. Creates blank node as category bridge (no payload)
4. Connects "cat" and "dog" to blank node
5. Blank node learns category through connections
6. When "bird" arrives: wave finds blank node, connects to it
7. Blank node now represents "3-letter animal word" category

**Generalization**:
- Blank node matches through connections to patterns
- Similar patterns connect to same blank
- Category knowledge compounds through connections

---

## Comparison to Neural Networks

| LLMs | Melvin |
|------|--------|
| Dense attention (O(n²)) | Sparse routing (O(degree)) |
| Global backpropagation | Local Hebbian learning |
| Fixed architecture | Dynamic structure (grows organically) |
| Implicit knowledge (in weights) | Explicit knowledge (nodes/edges) |
| Requires retraining | Continuous learning |
| Brute-force scaling | Structural scaling (hierarchies, abstractions) |

---

## Computational Complexity

### Time Complexity

- **Pattern Matching**: O(m) where m = pattern length
- **Wave Propagation**: O(m × d) where m = edges activated, d = average degree
- **Node Operations**: O(degree) per node
- **Edge Operations**: O(1) per edge
- **Overall**: O(m) operations per input (linear in graph size for sparse graphs)

### Space Complexity

- **Nodes**: O(n) where n = number of nodes
- **Edges**: O(m) where m = number of edges
- **Graph**: O(n + m), linear in graph size
- **Sparse graphs**: m ≈ O(n), so O(n) total space

---

## Implementation Status

The current implementation in `melvin.c` follows all core principles:

- ✅ Local-only operations (O(degree) per node)
- ✅ Adaptive capacities and thresholds
- ✅ Smooth probability-based functions
- ✅ Wave propagation with biological learning
- ✅ Hierarchy formation
- ✅ Continuous learning
- ✅ Pattern matching and edge formation
- ✅ File persistence (.m file format)

For detailed implementation specifications, see code comments in `melvin.c`.
