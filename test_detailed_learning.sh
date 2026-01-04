#!/bin/bash

BRAIN="test_detailed_brain.m"
rm -f "$BRAIN"

echo "=== Detailed Learning Analysis ==="
echo ""

# Test sequence
echo "hello" > /tmp/test1.txt
echo "world" > /tmp/test2.txt
echo "hello world" > /tmp/test3.txt

echo "--- Input 1: 'hello' ---"
./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "(Input-based|Node-based|sampled_byte)" | head -10
echo ""

echo "--- Input 2: 'hello' (repeat - should strengthen) ---"
./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "(Input-based|Node-based|sampled_byte|NODE_CHOICE)" | head -15
echo ""

echo "--- Input 3: 'world' ---"
./melvin_standalone /tmp/test2.txt "$BRAIN" 2>&1 | grep -E "(Input-based|Node-based|sampled_byte)" | head -10
echo ""

echo "--- Input 4: 'hello world' (should use learned paths) ---"
./melvin_standalone /tmp/test3.txt "$BRAIN" 2>&1 | grep -E "(Input-based|Node-based|sampled_byte|NODE_CHOICE)" | head -20
echo ""

echo "=== Summary ==="
echo "Check if:"
echo "1. First output comes from input node (h chooses e)"
echo "2. Subsequent outputs follow learned paths (e->l->l->o)"
echo "3. Edges strengthen with repetition"
echo "4. System follows complete sequences"

