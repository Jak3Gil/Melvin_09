#!/bin/bash

# Output Analysis Test
# - Tests if system is learning and producing intelligent outputs
# - Analyzes output quality and evolution

set -e

echo "=========================================="
echo "Melvin Output Analysis Test"
echo "=========================================="
echo ""

# Clean up previous test
rm -f test_output_analysis.m
rm -f test_analysis_input*.txt

# Create test inputs with patterns that should show learning
echo "Creating test inputs..."

# Pattern 1: Repeated sequence (should strengthen and produce predictable output)
echo "hello world" > test_analysis_input1.txt
echo "hello world" > test_analysis_input2.txt
echo "hello world" > test_analysis_input3.txt

# Pattern 2: Similar patterns (should create similarity edges)
echo "hello there" > test_analysis_input4.txt
echo "hello friend" > test_analysis_input5.txt

# Pattern 3: Different pattern (should create new nodes/edges)
echo "goodbye world" > test_analysis_input6.txt
echo "goodbye world" > test_analysis_input7.txt

# Pattern 4: More variations
echo "hello melvin" > test_analysis_input8.txt
echo "hello system" > test_analysis_input9.txt
echo "hello ai" > test_analysis_input10.txt

# Compile the test runner
echo "Compiling test runner..."
gcc -Wall -Wextra -O2 melvin.c melvin_in_port.c melvin_out_port.c test_output_analysis.c -o test_output_analysis -lm

# Run test
echo ""
echo "=========================================="
echo "Running output analysis test..."
echo "=========================================="
echo ""

./test_output_analysis test_output_analysis.m \
    test_analysis_input*.txt

echo ""
echo "=========================================="
echo "Test Complete"
echo "=========================================="
echo ""
echo "Check the output above for:"
echo "- Output quality and structure"
echo "- Learning indicators"
echo "- Output evolution over time"
echo "- Whether outputs are intelligent or random"

