#!/bin/bash
# Final test for multi-byte output generation

BRAIN="final_test.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Multi-Byte Output Generation - Final Test"
echo "=========================================="
echo ""

# Test 1: Basic training
echo "Test 1: Training 'hello' 50 times..."
for i in {1..50}; do
    echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

result=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
size=$(echo "$result" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
content=$(echo "$result" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1)
stop_reason=$(echo "$result" | grep "\[OUTPUT\] Stop:" | head -1)

echo "  Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "  ✓ PASS: Multi-byte output generated"
else
    echo "  ✗ FAIL: Still single-byte"
fi
echo "  Stop reason: $stop_reason"
echo ""

# Test 2: Sequence training
echo "Test 2: Training 'hello world' 100 times..."
for i in {1..100}; do
    echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

result=$(echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
size=$(echo "$result" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
content=$(echo "$result" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1)

echo "  Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "  ✓ PASS: Multi-byte output for sequence"
else
    echo "  ✗ FAIL: Still single-byte"
fi
echo ""

# Test 3: Multiple runs to check consistency
echo "Test 3: Testing 10 runs for consistency..."
multi_count=0
for i in {1..10}; do
    result=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    size=$(echo "$result" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
    if [ "$size" -gt 1 ]; then
        multi_count=$((multi_count + 1))
    fi
done

echo "  Multi-byte outputs: $multi_count / 10"
if [ "$multi_count" -gt 0 ]; then
    echo "  ✓ PASS: System generates multi-byte outputs"
else
    echo "  ✗ FAIL: All outputs single-byte"
fi
echo ""

echo "=========================================="
echo "Test Complete"
echo "=========================================="

