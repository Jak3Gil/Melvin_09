#!/bin/bash
# Quick test for multi-byte output

BRAIN="quick_test.m"
rm -f "$BRAIN"

echo "Quick Multi-Byte Test"
echo "===================="

# Train quickly
echo "Training 'abc' 30 times..."
for i in {1..30}; do
    echo -e "abc\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

# Test
echo "Testing output..."
result=$(echo -e "abc\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
size=$(echo "$result" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
content=$(echo "$result" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1)

echo "Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "✓ SUCCESS: Multi-byte output!"
else
    echo "✗ Still single-byte"
fi

# Show stop reason
echo "$result" | grep "\[OUTPUT\] Stop:" | head -1

