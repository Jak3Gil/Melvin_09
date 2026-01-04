#!/bin/bash

# Test script to verify the system learns and generates longer sequences
# Feed "hello" multiple times, then check if it generates the full sequence

set -e

BRAIN_FILE="test_learning_brain.m"
INPUT_FILE="test_learning_input.txt"

# Clean up from previous runs
rm -f "$BRAIN_FILE" "$INPUT_FILE"

echo "=== Testing Learning and Sequence Generation ==="
echo ""

# Feed "hello" 3 times to build the pattern
for i in 1 2 3; do
    echo "=== Input $i: hello ==="
    echo -n "hello" > "$INPUT_FILE"
    ./melvin_standalone "$INPUT_FILE" "$BRAIN_FILE" 2>&1 | grep -E "(OUTPUT.*sampled_byte|Generation complete|Nodes:|Edges:)" | tail -5
    echo ""
done

echo "=== Final Test: What does the system output now? ==="
echo -n "h" > "$INPUT_FILE"
./melvin_standalone "$INPUT_FILE" "$BRAIN_FILE" 2>&1 | grep -E "(OUTPUT.*sampled_byte|Generation complete|stop_reason)" | head -10

echo ""
echo "=== Summary ==="
if [ -f "$BRAIN_FILE" ]; then
    FILE_SIZE=$(stat -f%z "$BRAIN_FILE" 2>/dev/null || stat -c%s "$BRAIN_FILE" 2>/dev/null || echo "0")
    echo "Brain file size: $FILE_SIZE bytes"
fi

