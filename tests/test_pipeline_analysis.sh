#!/bin/bash

# Test pipeline: Feed same input multiple times and analyze outputs
BRAIN="test_pipeline_analysis.m"
INPUT="test_all_input1.txt"

rm -f "$BRAIN"

echo "=== Pipeline Test: Feeding 'hello' 15 times ==="
echo ""

for i in {1..15}; do
    echo "--- Input $i ---"
    
    # Process input and capture output
    OUTPUT=$(./melvin_standalone "$INPUT" "$BRAIN" 2>&1)
    
    # Extract output bytes
    OUTPUT_BYTES=$(echo "$OUTPUT" | grep "sampled_byte" | sed 's/.*byte=0x\([0-9a-f][0-9a-f]\)/\\x\1/' | tr -d '\n')
    
    # Get node/edge counts
    NODES=$(echo "$OUTPUT" | grep "Nodes:" | awk '{print $2}')
    EDGES=$(echo "$OUTPUT" | grep "Edges:" | awk '{print $2}')
    
    # Get generation info
    GEN_INFO=$(echo "$OUTPUT" | grep "Generation complete" | head -1)
    
    # Print summary
    if [ -n "$OUTPUT_BYTES" ]; then
        echo "  Output: $OUTPUT_BYTES"
        echo "  Output length: $(echo "$OUTPUT_BYTES" | wc -c)"
    else
        echo "  Output: (none)"
    fi
    echo "  Nodes: $NODES, Edges: $EDGES"
    echo "  $GEN_INFO"
    echo ""
done

echo "=== Final Brain State ==="
./melvin_standalone "$INPUT" "$BRAIN" 2>&1 | grep -E "(Nodes:|Edges:|Statistics:)" | head -5
