#!/bin/bash

# Long-term growth test for Melvin
# - Generates many test inputs
# - Processes them with persistent brain
# - Shows growth patterns and outputs

set -e

echo "=========================================="
echo "Melvin Long-Term Growth Test"
echo "=========================================="
echo ""

# Clean up previous test
rm -f test_long_growth.m
rm -f test_long_input*.txt

# Create test inputs (varied patterns to observe growth)
echo "Creating test inputs..."

# Pattern 1: Repeated sequence (should strengthen edges)
echo "The quick brown fox jumps over the lazy dog" > test_long_input1.txt
echo "The quick brown fox jumps over the lazy dog" > test_long_input2.txt
echo "The quick brown fox jumps over the lazy dog" > test_long_input3.txt

# Pattern 2: Different sequence (should create new nodes/edges)
echo "A completely different pattern that creates new connections" > test_long_input4.txt
echo "A completely different pattern that creates new connections" > test_long_input5.txt

# Pattern 3: Mixed patterns (should create similarity/context edges)
echo "Hello world from Melvin" > test_long_input6.txt
echo "Hello world from Melvin" > test_long_input7.txt
echo "Hello world from Melvin" > test_long_input8.txt

# Pattern 4: More variations
echo "Machine learning is fascinating" > test_long_input9.txt
echo "Machine learning is fascinating" > test_long_input10.txt
echo "Neural networks are powerful" > test_long_input11.txt
echo "Neural networks are powerful" > test_long_input12.txt

# Pattern 5: Short patterns (should create hierarchy)
echo "cat" > test_long_input13.txt
echo "cat" > test_long_input14.txt
echo "cat" > test_long_input15.txt
echo "dog" > test_long_input16.txt
echo "dog" > test_long_input17.txt

# Pattern 6: Longer patterns
echo "The cat sat on the mat" > test_long_input18.txt
echo "The cat sat on the mat" > test_long_input19.txt
echo "The dog ran in the park" > test_long_input20.txt
echo "The dog ran in the park" > test_long_input21.txt

# Pattern 7: More complex patterns
echo "Artificial intelligence systems learn from data" > test_long_input22.txt
echo "Artificial intelligence systems learn from data" > test_long_input23.txt
echo "Deep learning models process information" > test_long_input24.txt
echo "Deep learning models process information" > test_long_input25.txt

# Compile the test runner
echo "Compiling test runner..."
gcc -Wall -Wextra -O2 melvin.c melvin_in_port.c melvin_out_port.c test_long_growth.c -o test_long_growth

# Run test
echo ""
echo "=========================================="
echo "Running long-term growth test..."
echo "=========================================="
echo ""

./test_long_growth test_long_growth.m \
    test_long_input*.txt

echo ""
echo "=========================================="
echo "Test Complete"
echo "=========================================="
echo ""
echo "Check the output above for:"
echo "- Node and edge growth over time"
echo "- Edge-to-node ratio evolution"
echo "- Output generation"
echo "- Graph structure analysis"
echo "- Self-destruct activity (if any)"

