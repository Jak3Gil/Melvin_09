#!/bin/bash
# Simple pipeline test - prove outputs can grow

BRAIN="test_pipeline.m"
rm -f "$BRAIN"

echo "=== Pipeline Test: Output Growth ==="
echo ""

# Test 1: Single input
echo "Test 1: Single input 'hello'"
echo "hello" > /tmp/test1.txt
RESULT1=$(./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1)
echo "$RESULT1" | grep -E "(Nodes:|Edges:)"
echo ""

# Test 2: Multiple inputs to build patterns
echo "Test 2: Repeated inputs to build patterns"
for i in {1..3}; do
    echo "hello" > /tmp/test2.txt
    RESULT=$(./melvin_standalone /tmp/test2.txt "$BRAIN" 2>&1)
    if [ $i -eq 1 ] || [ $i -eq 3 ]; then
        echo "  Iteration $i:"
        echo "$RESULT" | grep -E "(Nodes:|Edges:)"
    fi
done
echo ""

# Test 3: Interactive test to see outputs
echo "Test 3: Interactive test with output display"
echo "hello" > /tmp/test3.txt
echo "quit" | ./test_dataset /tmp/test3.txt "$BRAIN" 2>&1 | grep -E "(Output:|Nodes:|Edges:)" | head -5
echo ""

# Test 4: Sequential pattern
echo "Test 4: Sequential pattern 'hello world'"
echo "hello world" > /tmp/test4.txt
RESULT4=$(./melvin_standalone /tmp/test4.txt "$BRAIN" 2>&1)
echo "$RESULT4" | grep -E "(Nodes:|Edges:)"
echo ""

# Test 5: Multiple sequential inputs
echo "Test 5: Repeated sequential patterns"
for i in {1..3}; do
    echo "hello world" > /tmp/test5.txt
    RESULT=$(./melvin_standalone /tmp/test5.txt "$BRAIN" 2>&1)
    if [ $i -eq 1 ] || [ $i -eq 3 ]; then
        echo "  Iteration $i:"
        echo "$RESULT" | grep -E "(Nodes:|Edges:)"
    fi
done

# Cleanup
rm -f /tmp/test*.txt

