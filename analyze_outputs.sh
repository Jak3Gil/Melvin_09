#!/bin/bash

# Analyze outputs from multiple inputs
BRAIN="test_analysis_brain.m"
rm -f "$BRAIN"

echo "=== Output Analysis: Testing Node Edge Collection ==="
echo ""

# Test 1: First "hello"
echo "--- Input 1: 'hello' ---"
echo "hello" > /tmp/test1.txt
./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "(sampled_byte|Generation complete|Stop:)" | tail -5
echo ""

# Test 2: Second "hello" (should strengthen edges)
echo "--- Input 2: 'hello' (repeat) ---"
./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "(sampled_byte|Generation complete|Stop:)" | tail -5
echo ""

# Test 3: "world"
echo "--- Input 3: 'world' ---"
echo "world" > /tmp/test2.txt
./melvin_standalone /tmp/test2.txt "$BRAIN" 2>&1 | grep -E "(sampled_byte|Generation complete|Stop:)" | tail -5
echo ""

# Test 4: "hello world" (should follow learned paths)
echo "--- Input 4: 'hello world' (should use learned h->e->l->l->o) ---"
echo "hello world" > /tmp/test3.txt
./melvin_standalone /tmp/test3.txt "$BRAIN" 2>&1 | grep -E "(sampled_byte|Node-based decision|NODE_CHOICE|Generation complete|Stop:)" | head -25
echo ""

echo "=== Analysis Complete ==="

