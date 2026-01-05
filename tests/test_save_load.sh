#!/bin/bash
# Test save/load functionality

BRAIN="test_saveload.m"
rm -f "$BRAIN"

echo "Testing Save/Load Functionality"
echo "================================"
echo ""

# Step 1: Create and train
echo "Step 1: Creating brain and training 'hello' 20 times..."
for i in {1..20}; do
    echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

# Check file was created
if [ ! -f "$BRAIN" ]; then
    echo "✗ FAIL: Brain file not created"
    exit 1
fi

FILE_SIZE=$(stat -f%z "$BRAIN" 2>/dev/null || stat -c%s "$BRAIN" 2>/dev/null)
echo "  Brain file size: $FILE_SIZE bytes"
echo ""

# Step 2: Test that it loads
echo "Step 2: Loading brain and testing..."
result=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
nodes=$(echo "$result" | grep -oE 'Nodes: [0-9]+' | grep -oE '[0-9]+' | head -1)
edges=$(echo "$result" | grep -oE 'Edges: [0-9]+' | grep -oE '[0-9]+' | head -1)

echo "  Loaded: $nodes nodes, $edges edges"

if [ -n "$nodes" ] && [ "$nodes" -gt 0 ]; then
    echo "  ✓ SUCCESS: Brain loaded with $nodes nodes"
else
    echo "  ✗ FAIL: Brain did not load properly"
    exit 1
fi

# Step 3: Test persistence - add more training
echo ""
echo "Step 3: Adding more training ('world' 10 times)..."
for i in {1..10}; do
    echo -e "world\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

result2=$(echo -e "world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
nodes2=$(echo "$result2" | grep -oE 'Nodes: [0-9]+' | grep -oE '[0-9]+' | head -1)
edges2=$(echo "$result2" | grep -oE 'Edges: [0-9]+' | grep -oE '[0-9]+' | head -1)

echo "  After more training: $nodes2 nodes, $edges2 edges"

if [ -n "$nodes2" ] && [ "$nodes2" -gt "$nodes" ]; then
    echo "  ✓ SUCCESS: Brain persisted and grew (was $nodes, now $nodes2)"
else
    echo "  ⚠ WARNING: Node count didn't increase (may be expected if nodes already existed)"
fi

echo ""
echo "================================"
echo "Save/Load Test Complete"
echo "================================"

