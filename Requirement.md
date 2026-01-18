Melvin.c is required to...
-NO O(n) searchs, no global searchs for anything.
-No hardcoded limits we dont want to add maxes or mins
-No hardcoded thresholds
-No Fallbacks
-All thresholds must be relative to local context (not absolute values)
  - Output readiness: nodes fire relative to strongest signal in current context
  - Edge selection: edges compete relative to local average, not absolute threshold
  - Hierarchy formation: edges form hierarchies relative to local average strength
  - All comparisons: activation/weight/strength compared to local max/mean, not hardcoded values

NODE-LEVEL DECISION MAKING (CRITICAL):
-NODES make predictions, NOT the system. Each node locally decides which edge to follow.
-Node prediction uses: embedding context + semantic features + edge weights as memory/log
-Edge weights are MEMORY/LOG ONLY: they record "this path was used before" but DO NOT drive decisions
-Context-driven decisions: nodes evaluate their embedding context to predict next edge
-Local evaluation: each node examines its outgoing edges using its own context
-System provides tools (embeddings, semantic features), nodes make decisions

-context is a payload, of that activated nodes from a input and the decisions made, the current node holds the context of the last x number, that context changes the node's prediction
-edges are paths they are the only paths that nodes can take, if a node doesnt have a edge between another node it cant touch that one or predict it
-nodes make mini prediction: given my embedding context and semantic features, which of MY edges should I follow?
  - Node evaluates: embedding similarity, semantic fit, disambiguation, analogical reasoning
  - Edge weight informs (memory/log) but doesn't decide
  - Node returns: "I predict edge X leads to the right continuation" 
-Enable embeddings for output generation only (not during training)
-Use cached embeddings (already implemented)
-Only compute for candidate nodes (not all nodes)

HIERARCHY FORMATION (Brain-Like Synaptic Consolidation):
-hierarchies form naturally from repeated patterns, not hardcoded "if-then" rules
-pure competition: edges stronger than local average (relative_strength > 1.0) automatically form hierarchies
-no hardcoded thresholds: more repetitions → stronger edge → hierarchy forms automatically
-mini nets learn from hierarchy outcomes (reinforcement) but don't block formation
-self-growing: patterns that repeat often naturally consolidate into hierarchies
-recursive: hierarchies can combine into higher-level hierarchies (level 1 → level 2 → level 3...)
-data-driven: all formation decisions based on edge strength relative to local context
-ORGANIC MULTI-LEVEL: Two-phase process - Phase 1 processes input sequence (2-byte), Phase 2 processes activation pattern (3+ byte)
-COMPUTE SAVINGS: Larger hierarchies = fewer node traversals = faster generation organically
-No fallbacks: Pure competition and co-activation drive hierarchy formation

BRAIN-SCALE REQUIREMENTS (86B nodes, 100T edges):
-Node on-disk size < 20 bytes (plus payload) - must scale to 86 billion nodes
-Edge on-disk size < 20 bytes - must scale to 100 trillion edges
-Compute on-demand: weight sums, best edge, activation - don't store what can be computed
-Active nodes cached in RAM, inactive nodes on disk (memory-mapped)
-No per-edge context storage - use shared context pool with IDs
-Runtime caches invalidated when data changes (generation tracking)

ADAPTIVE COMPUTE (Context Through Computation):
-Context is not stored, it is computed through wave propagation
-Multi-pass wave propagation: adaptive (1-10 passes) based on confidence, not fixed
-Mini-transformers: attention-based edge transformation during wave propagation
-Mini-nets evaluate confidence: request more compute when ambiguous
-Adaptive thresholds: change with context complexity, ambiguity, and local state
-Everything relative: activation, thresholds, confidence all relative to local context
-Graph IS the memory: edge weights + embeddings + mini-nets encode all context
-Trade-off: 90-96% memory reduction for 3-10x adaptive compute (acceptable)
-