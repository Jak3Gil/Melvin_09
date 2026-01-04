#!/bin/bash
# Demonstration: Watch ML-style outputs evolve over multiple inputs

BRAIN="demo_ml_brain.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Melvin ML-Style Output Demonstration"
echo "=========================================="
echo ""
echo "This shows how outputs change as the system learns."
echo "We'll feed the same input multiple times and watch outputs evolve."
echo ""

# Create brain
MelvinMFile *mfile = melvin_m_create("$BRAIN");

echo "=== Test Sequence: Learning 'hello' ==="
echo ""

# Feed "hello" multiple times
for i in 1 2 3 4 5; do
    echo "--- Input #$i: 'hello' ---"
    echo "hello" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing|Output|Nodes|Edges)" | head -4
    echo ""
    sleep 0.5
done

echo "=== Test Sequence: Learning 'world' ==="
echo ""

# Feed "world" multiple times
for i in 1 2 3; do
    echo "--- Input #$i: 'world' ---"
    echo "world" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing|Output|Nodes|Edges)" | head -4
    echo ""
    sleep 0.5
done

echo "=== Test Sequence: Novel Input 'hello world' ==="
echo ""

# Feed novel combination
for i in 1 2; do
    echo "--- Input #$i: 'hello world' ---"
    echo "hello world" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing|Output|Nodes|Edges)" | head -4
    echo ""
    sleep 0.5
done

echo "=== Final Statistics ==="
echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | tail -5

