#!/bin/bash
# Detailed pipeline test - check output lengths and evolution

BRAIN="test_pipeline_detailed.m"
rm -f "$BRAIN"

echo "=== Detailed Pipeline Testing ==="
echo ""

# Function to extract output size
extract_output_size() {
    grep -oP 'Output:.*?\(\K[0-9]+(?= bytes)' || echo "0"
}

# Function to extract output content
extract_output() {
    grep -A1 "Output:" | grep -oP 'Output: "\K[^"]*' | head -1 || echo ""
}

echo "=== Test 1: Fresh Brain - Single Input ==="
echo "Input: 'test'"
RESULT1=$(echo -e "test\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
echo "$RESULT1" | grep -E "(Output:|Nodes:|Edges:)"
SIZE1=$(echo "$RESULT1" | extract_output_size)
OUT1=$(echo "$RESULT1" | extract_output)
echo "Output size: $SIZE1 bytes"
echo "Output content: '$OUT1'"
echo ""

echo "=== Test 2: Learning Phase - 10 Repeated Inputs ==="
for i in {1..10}; do
    RESULT=$(echo -e "test\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    SIZE=$(echo "$RESULT" | extract_output_size)
    OUT=$(echo "$RESULT" | extract_output)
    NODES=$(echo "$RESULT" | grep "Nodes:" | grep -oP '[0-9]+' | head -1)
    EDGES=$(echo "$RESULT" | grep "Edges:" | grep -oP '[0-9]+' | head -1)
    if [ $((i % 2)) -eq 0 ] || [ $i -eq 1 ] || [ $i -eq 10 ]; then
        echo "Iteration $i: Output size=$SIZE bytes, Nodes=$NODES, Edges=$EDGES"
    fi
done
echo ""

echo "=== Test 3: Pattern Building - Sequential Input ==="
echo "Input: 'hello world' (5 times)"
for i in {1..5}; do
    RESULT=$(echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    SIZE=$(echo "$RESULT" | extract_output_size)
    OUT=$(echo "$RESULT" | extract_output)
    if [ $i -eq 1 ] || [ $i -eq 5 ]; then
        echo "Iteration $i: Output size=$SIZE bytes, Output='$OUT'"
    fi
done
echo ""

echo "=== Test 4: Final State ==="
FINAL=$(echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
echo "$FINAL" | tail -5

