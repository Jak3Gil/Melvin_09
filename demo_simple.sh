#!/bin/bash
# Simple demonstration: Watch outputs evolve

BRAIN="demo_simple.m"
rm -f "$BRAIN"

echo "=== Melvin ML-Style Output Evolution ==="
echo ""

# Sequence of inputs showing learning
inputs=(
    "hello"
    "hello"
    "hello"
    "world"
    "world"
    "hello world"
)

for i in "${!inputs[@]}"; do
    input="${inputs[$i]}"
    echo "[Input $((i+1))] '$input'"
    echo "$input" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Output:|Nodes:|Edges:)" | head -3
    echo ""
done

echo "=== Final Stats ==="
echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | tail -3

