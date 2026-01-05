# Output Failure Analysis

## Debug Output Results

```
[DEBUG OUTPUT] Calling melvin_generate_output_from_state with 13 activated nodes
[DEBUG OUTPUT] melvin_generate_output_from_state called: input_count=18, all_activated_count=13
[DEBUG OUTPUT] After filtering: candidate_count=12 (from 13 activated nodes)
[DEBUG OUTPUT] First byte: start_node=0x11e606d30, outgoing_count=0
[DEBUG OUTPUT] Start node has no outgoing edges
[DEBUG OUTPUT] After output generation, output_size=1
```

## Root Cause

**The problem**: Output generation tries to start from the LAST input node (line 4400), but:
- The last input node is the last character in the sequence (e.g., 'o' in "hello")
- The last character has NO outgoing edges (by definition - it's the end!)
- So output generation stops immediately with "input_no_outgoing_edges"
- The echo fallback then outputs 1 byte (first input node's payload)

## Why This Is Wrong

1. **Wrong starting point**: Starting from the last input node doesn't make sense for output generation
2. **Candidates are ignored**: We collected 12 candidates from wave propagation, but they're not used for the first byte
3. **Contradicts README**: The system should use "local relative decisions" - use the candidates that were activated, not try to continue from the last input byte

## What Should Happen

According to README principles:
- "All decisions are relative to local context"
- Output should sample from the activated candidates (like LLM softmax over vocabulary)
- The first byte should come from the candidate distribution, not from trying to continue the input sequence

## The Fix Needed

The first byte generation (line 4398-4420) should:
1. **NOT** try to continue from the last input node
2. **Instead**: Sample from the candidate distribution (the 12 candidates collected)
3. Use the logits/probabilities computed from activation_strength and edge transforms
4. This matches LLM behavior: compute probabilities over all candidates, then sample

The current code path at lines 4444+ (the "else" branch) seems to handle candidate-based selection, but it's not being used for the first byte.

