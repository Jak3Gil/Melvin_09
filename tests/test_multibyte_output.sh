#!/bin/bash
# Test script for multi-byte output generation
# Verifies that the patch fixes single-byte output limitation

BRAIN="multibyte_test.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Multi-Byte Output Generation Test"
echo "=========================================="
echo ""

# Check if test_dataset is built
if [ ! -f "./test_dataset" ]; then
    echo "Building test_dataset..."
    make test_dataset
fi

# Function to get output size
get_output_size() {
    local input="$1"
    local result=$(echo -e "$input\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    local size=$(echo "$result" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
    local content=$(echo "$result" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1 || echo "")
    echo "$size|$content"
}

# Function to check stop reason
check_stop_reason() {
    local input="$1"
    local result=$(echo -e "$input\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    echo "$result" | grep "\[OUTPUT\] Stop:" | head -1 || echo "no_stop_reason"
}

echo "=== Test 1: Basic Training and Output ==="
echo "Training 'hello' 50 times..."
for i in {1..50}; do
    echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

echo "Testing output length..."
stats=$(get_output_size "hello")
IFS='|' read -r size content <<< "$stats"
echo "  Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "  ✓ PASS: Multi-byte output generated ($size bytes)"
else
    echo "  ✗ FAIL: Still single-byte output ($size bytes)"
fi

stop_reason=$(check_stop_reason "hello")
echo "  Stop reason: $stop_reason"
echo ""

echo "=== Test 2: Sequence Training ==="
echo "Training 'hello world' 100 times..."
for i in {1..100}; do
    echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

echo "Testing sequence output..."
stats=$(get_output_size "hello world")
IFS='|' read -r size content <<< "$stats"
echo "  Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "  ✓ PASS: Multi-byte output for sequence ($size bytes)"
else
    echo "  ✗ FAIL: Still single-byte ($size bytes)"
fi

stop_reason=$(check_stop_reason "hello world")
echo "  Stop reason: $stop_reason"
echo ""

echo "=== Test 3: Heavy Training ==="
echo "Training 'test pattern' 200 times..."
for i in {1..200}; do
    echo -e "test pattern\nquit" | ./test_dataset /dev/null "$BRAIN" > /dev/null 2>&1
done

echo "Testing heavily trained pattern..."
stats=$(get_output_size "test pattern")
IFS='|' read -r size content <<< "$stats"
echo "  Output: '$content' ($size bytes)"
if [ "$size" -gt 1 ]; then
    echo "  ✓ PASS: Multi-byte output after heavy training ($size bytes)"
else
    echo "  ✗ FAIL: Still single-byte after heavy training ($size bytes)"
fi

stop_reason=$(check_stop_reason "test pattern")
echo "  Stop reason: $stop_reason"
echo ""

echo "=== Test 4: Multiple Test Runs ==="
echo "Testing 'hello' 10 times to check consistency..."
multi_byte_count=0
for i in {1..10}; do
    stats=$(get_output_size "hello")
    IFS='|' read -r size content <<< "$stats"
    if [ "$size" -gt 1 ]; then
        multi_byte_count=$((multi_byte_count + 1))
    fi
    if [ $((i % 3)) -eq 0 ]; then
        echo "  Run $i: $size bytes"
    fi
done
echo "  Multi-byte outputs: $multi_byte_count / 10"
if [ "$multi_byte_count" -gt 0 ]; then
    echo "  ✓ PASS: System can generate multi-byte outputs"
else
    echo "  ✗ FAIL: All outputs are single-byte"
fi
echo ""

echo "=== Test 5: Debug Output Verification ==="
echo "Checking for debug stop reason logs..."
result=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
if echo "$result" | grep -q "\[OUTPUT\]"; then
    echo "  ✓ PASS: Debug logs present"
    echo "$result" | grep "\[OUTPUT\]" | head -5
else
    echo "  ✗ FAIL: No debug logs found"
fi
echo ""

echo "=== Test 6: Output Length Distribution ==="
echo "Collecting 20 outputs to analyze length distribution..."
sizes=()
for i in {1..20}; do
    stats=$(get_output_size "hello")
    IFS='|' read -r size content <<< "$stats"
    sizes+=("$size")
done

max_size=$(printf '%s\n' "${sizes[@]}" | sort -n | tail -1)
min_size=$(printf '%s\n' "${sizes[@]}" | sort -n | head -1)
avg_size=$(printf '%s\n' "${sizes[@]}" | awk '{sum+=$1; count++} END {if(count>0) print int(sum/count); else print 0}')

echo "  Min output size: $min_size bytes"
echo "  Max output size: $max_size bytes"
echo "  Average output size: $avg_size bytes"

if [ "$max_size" -gt 1 ]; then
    echo "  ✓ PASS: System generates multi-byte outputs (max: $max_size bytes)"
else
    echo "  ✗ FAIL: All outputs are single-byte"
fi
echo ""

echo "=========================================="
echo "Test Summary"
echo "=========================================="
if [ "$max_size" -gt 1 ]; then
    echo "✓✓✓ SUCCESS: Multi-byte output generation is working!"
    echo "  The patch successfully removed entropy-based early stopping."
else
    echo "✗✗✗ FAILURE: Still generating single-byte outputs"
    echo "  May need more training or additional fixes."
fi
echo ""

