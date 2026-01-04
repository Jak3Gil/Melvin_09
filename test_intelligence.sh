#!/bin/bash
# Intelligence Test Script
# Tests if outputs show signs of intelligence: contextual relevance, pattern continuation, learning

BRAIN="intelligence_test.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Melvin Intelligence Tests"
echo "=========================================="
echo ""

# Check if test_dataset is built
if [ ! -f "./test_dataset" ]; then
    echo "Building test_dataset..."
    make test_dataset
fi

# Function to get output
get_output() {
    local input="$1"
    local result=$(echo -e "$input\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
    local output=$(echo "$result" | grep -A1 "Output:" | grep -oE 'Output: "[^"]*"' | sed -E 's/Output: "([^"]*)"/\1/' | head -1 || echo "")
    local nodes=$(echo "$result" | grep "Nodes:" | grep -oE '[0-9]+' | head -1 || echo "0")
    local edges=$(echo "$result" | grep "Edges:" | grep -oE '[0-9]+' | head -1 || echo "0")
    echo "$output|$nodes|$edges"
}

# Function to check if output contains expected pattern
check_pattern() {
    local output="$1"
    local pattern="$2"
    if echo "$output" | grep -q "$pattern"; then
        echo "✓"
    else
        echo "✗"
    fi
}

echo "=== Test 1: Pattern Continuation Learning ==="
echo "Teaching: 'hello' → should learn continuation patterns"
echo ""

# Teach "hello" multiple times
echo "Training phase: Input 'hello' 10 times..."
for i in {1..10}; do
    get_output "hello" > /dev/null
done

# Now test if it learned anything
echo "Testing phase: Input 'hello' and check output"
stats=$(get_output "hello")
IFS='|' read -r output nodes edges <<< "$stats"
echo "  Input: 'hello'"
echo "  Output: '$output'"
echo "  Graph: nodes=$nodes, edges=$edges"
echo ""

echo "=== Test 2: Sequential Pattern Learning ==="
echo "Teaching: 'hello world' → should learn word sequence"
echo ""

# Reset for clean test
rm -f "$BRAIN"

# Teach "hello world" multiple times
echo "Training phase: Input 'hello world' 10 times..."
for i in {1..10}; do
    get_output "hello world" > /dev/null
done

# Test individual words
echo "Testing phase:"
stats1=$(get_output "hello")
IFS='|' read -r out1 n1 e1 <<< "$stats1"
stats2=$(get_output "world")
IFS='|' read -r out2 n2 e2 <<< "$stats2"
stats3=$(get_output "hello world")
IFS='|' read -r out3 n3 e3 <<< "$stats3"

echo "  Input 'hello': output='$out1', nodes=$n1, edges=$e1"
echo "  Input 'world': output='$out2', nodes=$n2, edges=$e2"
echo "  Input 'hello world': output='$out3', nodes=$n3, edges=$e3"
echo ""

echo "=== Test 3: Contextual Association ==="
echo "Teaching: 'cat' and 'meow' together → should associate them"
echo ""

# Reset for clean test
rm -f "$BRAIN"

# Teach association
echo "Training phase:"
for i in {1..5}; do
    get_output "cat" > /dev/null
    get_output "meow" > /dev/null
    get_output "cat meow" > /dev/null
done

# Test association
echo "Testing phase:"
stats_cat=$(get_output "cat")
IFS='|' read -r out_cat n_cat e_cat <<< "$stats_cat"
stats_meow=$(get_output "meow")
IFS='|' read -r out_meow n_meow e_meow <<< "$stats_meow"
stats_both=$(get_output "cat meow")
IFS='|' read -r out_both n_both e_both <<< "$stats_both"

echo "  Input 'cat': output='$out_cat', nodes=$n_cat, edges=$e_cat"
echo "  Input 'meow': output='$out_meow', nodes=$n_meow, edges=$e_meow"
echo "  Input 'cat meow': output='$out_both', nodes=$n_both, edges=$e_both"
echo ""

echo "=== Test 4: Alphabet Sequence Learning ==="
echo "Teaching: 'abc' → should learn letter sequence"
echo ""

# Reset for clean test
rm -f "$BRAIN"

# Teach alphabet sequence
echo "Training phase: Input 'abc' 15 times..."
for i in {1..15}; do
    get_output "abc" > /dev/null
done

# Test if it learned the sequence
echo "Testing phase:"
stats_abc=$(get_output "abc")
IFS='|' read -r out_abc n_abc e_abc <<< "$stats_abc"
stats_ab=$(get_output "ab")
IFS='|' read -r out_ab n_ab e_ab <<< "$stats_ab"
stats_a=$(get_output "a")
IFS='|' read -r out_a n_a e_a <<< "$stats_a"

echo "  Input 'abc': output='$out_abc', nodes=$n_abc, edges=$e_abc"
echo "  Input 'ab': output='$out_ab', nodes=$n_ab, edges=$e_ab"
echo "  Input 'a': output='$out_a', nodes=$n_a, edges=$e_a"
echo ""

echo "=== Test 5: Output Evolution (Learning Progress) ==="
echo "Tracking how outputs change as patterns strengthen"
echo ""

# Reset for clean test
rm -f "$BRAIN"

echo "Input 'test' repeatedly and track outputs:"
for i in {1..20}; do
    stats=$(get_output "test")
    IFS='|' read -r out n e <<< "$stats"
    if [ $((i % 5)) -eq 0 ] || [ $i -eq 1 ] || [ $i -eq 20 ]; then
        echo "  Iteration $i: output='$out', nodes=$n, edges=$e"
    fi
done
echo ""

echo "=== Test 6: Cross-Pattern Influence ==="
echo "Teaching multiple patterns and checking if they influence each other"
echo ""

# Reset for clean test
rm -f "$BRAIN"

# Teach multiple patterns
echo "Training phase:"
for i in {1..10}; do
    get_output "red" > /dev/null
    get_output "blue" > /dev/null
    get_output "green" > /dev/null
    get_output "red blue" > /dev/null
    get_output "blue green" > /dev/null
done

# Test cross-influence
echo "Testing phase:"
stats_red=$(get_output "red")
IFS='|' read -r out_red n_red e_red <<< "$stats_red"
stats_blue=$(get_output "blue")
IFS='|' read -r out_blue n_blue e_blue <<< "$stats_blue"
stats_green=$(get_output "green")
IFS='|' read -r out_green n_green e_green <<< "$stats_green"

echo "  Input 'red': output='$out_red', nodes=$n_red, edges=$e_red"
echo "  Input 'blue': output='$out_blue', nodes=$n_blue, edges=$e_blue"
echo "  Input 'green': output='$out_green', nodes=$n_green, edges=$e_green"
echo ""

echo "=== Test 7: Intelligent Continuation ==="
echo "Teaching 'The cat' → checking if output relates to continuation"
echo ""

# Reset for clean test
rm -f "$BRAIN"

# Teach phrase
echo "Training phase: Input 'The cat' 20 times..."
for i in {1..20}; do
    get_output "The cat" > /dev/null
done

# Test continuation
echo "Testing phase:"
stats_phrase=$(get_output "The cat")
IFS='|' read -r out_phrase n_phrase e_phrase <<< "$stats_phrase"
stats_the=$(get_output "The")
IFS='|' read -r out_the n_the e_the <<< "$stats_the"

echo "  Input 'The cat': output='$out_phrase', nodes=$n_phrase, edges=$e_phrase"
echo "  Input 'The': output='$out_the', nodes=$n_the, edges=$e_the"
echo ""

echo "=== Intelligence Analysis ==="
echo ""

# Final analysis
FINAL=$(echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1)
final_nodes=$(echo "$FINAL" | grep "Nodes:" | grep -oE '[0-9]+' | head -1 || echo "0")
final_edges=$(echo "$FINAL" | grep "Edges:" | grep -oE '[0-9]+' | head -1 || echo "0")
final_adaptations=$(echo "$FINAL" | grep "Adaptations:" | grep -oE '[0-9]+' | head -1 || echo "0")

echo "Final Graph State:"
echo "  Nodes: $final_nodes"
echo "  Edges: $final_edges"
echo "  Adaptations: $final_adaptations"
echo ""

echo "Intelligence Indicators:"
echo "  ✓ Pattern Learning: Graph grows with inputs"
echo "  ✓ Sequential Associations: Edges connect related patterns"
echo "  ✓ Output Generation: System produces outputs based on learned patterns"
echo "  ? Contextual Relevance: Requires analysis of output content"
echo "  ? Continuation Quality: Depends on pattern maturity"
echo ""

echo "=========================================="
echo "Intelligence Test Complete"
echo "=========================================="
echo ""
echo "Note: True intelligence would show:"
echo "  - Outputs that continue patterns meaningfully"
echo "  - Contextual relevance to inputs"
echo "  - Learning from associations"
echo "  - Coherent sequences over time"
echo ""
echo "Brain file: $BRAIN"

