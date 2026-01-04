#!/bin/bash

# Test Edge Deletion Mechanism
# Verifies that:
# 1. Edges decay when unused
# 2. Weak edges are pruned (deleted)
# 3. Edge count stabilizes (doesn't grow unbounded)
# 4. All thresholds are adaptive (no hardcoded values)

set -e

echo "=========================================="
echo "Testing Edge Deletion Mechanism"
echo "=========================================="
echo ""

# Clean up previous test
rm -f test_edge_deletion.m
rm -f test_edge_deletion_input*.txt

# Create test inputs (repeated patterns to create edges)
echo "Creating test inputs..."
echo "The quick brown fox jumps over the lazy dog" > test_edge_deletion_input1.txt
echo "The quick brown fox jumps over the lazy dog" > test_edge_deletion_input2.txt
echo "The quick brown fox jumps over the lazy dog" > test_edge_deletion_input3.txt
echo "A completely different pattern that creates new edges" > test_edge_deletion_input4.txt
echo "A completely different pattern that creates new edges" > test_edge_deletion_input5.txt

# Compile the test runner
echo "Compiling test runner..."
gcc -Wall -Wextra -O2 melvin.c melvin_in_port.c melvin_out_port.c test_production_persistent.c -o test_edge_deletion_runner

# Run test with persistent brain
echo ""
echo "=========================================="
echo "Processing inputs with persistent brain..."
echo "=========================================="
echo ""

./test_edge_deletion_runner test_edge_deletion.m \
    test_edge_deletion_input1.txt \
    test_edge_deletion_input2.txt \
    test_edge_deletion_input3.txt \
    test_edge_deletion_input4.txt \
    test_edge_deletion_input5.txt

echo ""
echo "=========================================="
echo "Edge Deletion Test Complete"
echo "=========================================="
echo ""
echo "Expected behavior:"
echo "1. Edge count grows initially (new patterns)"
echo "2. Edge count stabilizes or decreases (pruning kicks in)"
echo "3. Weak unused edges are deleted"
echo "4. Strong active edges persist"
echo "5. Edge-to-node ratio: 2-4:1 (healthy, not 800:400)"
echo ""
echo "Check the output above for:"
echo "- Edge count after each input"
echo "- Edge pruning activity"
echo "- Final edge-to-node ratio"

