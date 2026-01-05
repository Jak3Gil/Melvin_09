#!/bin/bash

# Simple output test - just show what outputs look like
set -e

echo "=========================================="
echo "Melvin Simple Output Test"
echo "=========================================="
echo ""

rm -f test_simple.m
rm -f test_simple_input*.txt

# Create simple test inputs
echo "hello" > test_simple_input1.txt
echo "hello" > test_simple_input2.txt
echo "world" > test_simple_input3.txt
echo "world" > test_simple_input4.txt

# Compile
gcc -Wall -Wextra -O2 melvin.c melvin_in_port.c melvin_out_port.c test_production_persistent.c -o test_simple

# Run with limited inputs to avoid crash
echo "Processing inputs..."
./test_simple test_simple.m test_simple_input1.txt test_simple_input2.txt test_simple_input3.txt 2>&1 | grep -v "OUTPUT\]" | tail -30

echo ""
echo "=========================================="
echo "Output Analysis"
echo "=========================================="
echo ""
echo "Check if outputs are:"
echo "1. Random/meaningless (system not learning)"
echo "2. Related to inputs (system learning patterns)"
echo "3. Evolving over time (system improving)"
echo "4. Repetitive (system stuck)"

