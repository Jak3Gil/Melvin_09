#!/bin/bash
# Interactive test: Feed inputs and watch outputs evolve

BRAIN="test_ml_brain.m"

echo "=== Melvin ML-Style Output Test ==="
echo "Brain: $BRAIN"
echo ""
echo "This test shows how outputs change as the system learns."
echo "Type inputs and watch how outputs evolve over multiple inputs."
echo "Type 'quit' to exit."
echo ""

# Create or load brain
./test_dataset /dev/null "$BRAIN" <<< "quit" > /dev/null 2>&1

# Interactive loop
while true; do
    echo -n "Input: "
    read input
    
    if [ "$input" = "quit" ]; then
        break
    fi
    
    if [ -z "$input" ]; then
        continue
    fi
    
    echo "Processing: \"$input\""
    
    # Feed input and get output
    echo "$input" | ./test_dataset /dev/null "$BRAIN" 2>&1 | grep -A 5 "Processing:"
    
    echo ""
done

echo ""
echo "=== Final Statistics ==="
./test_dataset /dev/null "$BRAIN" <<< "quit" 2>&1 | tail -5
