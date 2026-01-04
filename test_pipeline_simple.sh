#!/bin/bash
# Simple Pipeline Test - Shows Input → Output Flow

BRAIN="demo_pipeline.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Melvin Pipeline Demo"
echo "=========================================="
echo ""

echo "Test 1: Input 'hello' → Output"
echo "-------------------------------"
echo -e "hello\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing:|Output:|Nodes:|Edges:)"
echo ""

echo "Test 2: Input 'world' → Output"
echo "-------------------------------"
echo -e "world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing:|Output:|Nodes:|Edges:)"
echo ""

echo "Test 3: Input 'hello world' → Output"
echo "--------------------------------------"
echo -e "hello world\nquit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -E "(Processing:|Output:|Nodes:|Edges:)"
echo ""

echo "Test 4: File Input → Output"
echo "----------------------------"
echo "test file content" > /tmp/demo_input.txt
echo "quit" | ./test_dataset /tmp/demo_input.txt "$BRAIN" 2>&1 | grep -E "(Input file:|Output:|Nodes:|Edges:)"
rm -f /tmp/demo_input.txt
echo ""

echo "Final Statistics:"
echo "-----------------"
echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | tail -5
