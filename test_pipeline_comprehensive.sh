#!/bin/bash
# Comprehensive Pipeline Test Script
# Tests input port → processing → output port pipeline

set -e

BRAIN="pipeline_test.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Melvin Pipeline System Test"
echo "=========================================="
echo ""

# Check if test_dataset is built
if [ ! -f "./test_dataset" ]; then
    echo "Building test_dataset..."
    make test_dataset
fi

# Function to analyze output (macOS compatible)
analyze_output() {
    local output="$1"
    local test_name="$2"
    
    echo ""
    echo "=== Analysis: $test_name ==="
    
    # Extract output size (macOS compatible - no -P flag)
    local size=$(echo "$output" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
    echo "Output size: $size bytes"
    
    # Extract output content (macOS compatible)
    local content=$(echo "$output" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1 || echo "")
    if [ -n "$content" ]; then
        echo "Output content: '$content'"
        if command -v xxd >/dev/null 2>&1; then
            echo "Output hex: $(echo -n "$content" | xxd -p | tr -d '\n')"
        fi
    fi
    
    # Extract graph stats (macOS compatible)
    local nodes=$(echo "$output" | grep "Nodes:" | grep -oE '[0-9]+' | head -1 || echo "0")
    local edges=$(echo "$output" | grep "Edges:" | grep -oE '[0-9]+' | head -1 || echo "0")
    local adaptations=$(echo "$output" | grep "Adaptations:" | grep -oE '[0-9]+' | head -1 || echo "0")
    
    echo "Graph state: Nodes=$nodes, Edges=$edges, Adaptations=$adaptations"
    
    # Check if output was generated
    if [ "$size" -gt 0 ]; then
        echo "✓ Output generated successfully"
    else
        echo "⚠ No output (pure thinking mode - may be normal for new patterns)"
    fi
}

# Test 1: Single input through input port
echo "=========================================="
echo "Test 1: Single Input via Input Port"
echo "=========================================="
echo "Input: 'hello'"
RESULT1=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
echo "$RESULT1" | grep -E "(Output:|Nodes:|Edges:|Adaptations:)"
analyze_output "$RESULT1" "Single Input Test"
echo ""

# Test 2: Multiple inputs to build patterns
echo "=========================================="
echo "Test 2: Pattern Building (5 repetitions)"
echo "=========================================="
echo "Input: 'hello' (repeated 5 times)"
for i in {1..5}; do
    RESULT=$(echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    if [ $i -eq 1 ] || [ $i -eq 5 ]; then
        echo "Iteration $i:"
        analyze_output "$RESULT" "Pattern Building Iteration $i"
    fi
done
echo ""

# Test 3: Sequential pattern
echo "=========================================="
echo "Test 3: Sequential Pattern Learning"
echo "=========================================="
echo "Input: 'hello world' (3 times)"
for i in {1..3}; do
    RESULT=$(echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    if [ $i -eq 1 ] || [ $i -eq 3 ]; then
        echo "Iteration $i:"
        analyze_output "$RESULT" "Sequential Pattern Iteration $i"
    fi
done
echo ""

# Test 4: File input via input port
echo "=========================================="
echo "Test 4: File Input via Input Port"
echo "=========================================="
echo "Creating test file..."
echo "test file input" > /tmp/pipeline_test_input.txt
RESULT4=$(echo "quit" | ./test_dataset /tmp/pipeline_test_input.txt "$BRAIN" 2>&1)
echo "$RESULT4" | grep -E "(Output:|Nodes:|Edges:|Adaptations:)"
analyze_output "$RESULT4" "File Input Test"
rm -f /tmp/pipeline_test_input.txt
echo ""

# Test 5: Different input patterns
echo "=========================================="
echo "Test 5: Varied Input Patterns"
echo "=========================================="
PATTERNS=("abc" "def" "abc def" "xyz" "abc")
for pattern in "${PATTERNS[@]}"; do
    RESULT=$(echo -e "$pattern\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    echo "Input: '$pattern'"
    analyze_output "$RESULT" "Pattern: $pattern"
done
echo ""

# Test 6: Output evolution tracking
echo "=========================================="
echo "Test 6: Output Evolution Tracking"
echo "=========================================="
echo "Repeated input 'test' to track output changes..."
OUTPUTS=()
for i in {1..10}; do
    RESULT=$(echo -e "test\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    SIZE=$(echo "$RESULT" | grep -oE 'Output:.*\([0-9]+ bytes\)' | sed -E 's/.*\(([0-9]+) bytes\).*/\1/' || echo "0")
    CONTENT=$(echo "$RESULT" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1 || echo "")
    OUTPUTS+=("$SIZE:$CONTENT")
    
    if [ $((i % 3)) -eq 0 ] || [ $i -eq 1 ] || [ $i -eq 10 ]; then
        echo "Iteration $i: size=$SIZE bytes, content='$CONTENT'"
    fi
done
echo ""

# Test 7: Final statistics
echo "=========================================="
echo "Test 7: Final Graph Statistics"
echo "=========================================="
FINAL=$(echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
echo "$FINAL" | tail -10
echo ""

# Summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "✓ Input port processing: Tested"
echo "✓ Pipeline processing: Tested"
echo "✓ Output generation: Tested"
echo "✓ Pattern learning: Tested"
echo "✓ Graph growth: Tested"
echo ""
echo "Brain file: $BRAIN"
echo "Test complete!"

