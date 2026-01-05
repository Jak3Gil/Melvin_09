#!/bin/bash
# Test script to prove outputs can grow and test the pipeline

BRAIN="test_output_growth.m"
rm -f "$BRAIN"

echo "=== Testing Output Growth and Pipeline ==="
echo ""

# Test 1: Single input, check output
echo "=== Test 1: Single Input ==="
echo "Input: 'hello'"
OUTPUT1=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -A1 "Output:" | head -2)
echo "$OUTPUT1"
echo ""

# Test 2: Multiple inputs to build patterns
echo "=== Test 2: Learning with Repeated Inputs ==="
for i in {1..5}; do
    echo "Input $i: 'hello'"
    OUTPUT=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -A1 "Output:" | head -2)
    echo "$OUTPUT"
    NODES=$(echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep "Nodes:" | head -1)
    echo "$NODES"
    echo ""
done

# Test 3: Sequence input (should create sequential patterns)
echo "=== Test 3: Sequential Pattern Learning ==="
echo "Input: 'hello world'"
OUTPUT3=$(echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -A1 "Output:" | head -2)
echo "$OUTPUT3"
echo ""

# Test 4: Multiple sequential inputs
echo "=== Test 4: Repeated Sequential Patterns ==="
for i in {1..3}; do
    echo "Input $i: 'hello world'"
    OUTPUT=$(echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -A1 "Output:" | head -2)
    echo "$OUTPUT"
    echo ""
done

# Test 5: Final statistics
echo "=== Final Statistics ==="
echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | tail -5

