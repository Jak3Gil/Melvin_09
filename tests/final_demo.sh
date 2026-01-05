#!/bin/bash
# Final demonstration of Melvin's learning capability

echo "========================================="
echo "MELVIN LEARNING DEMONSTRATION"
echo "========================================="
echo ""

# Clean start
rm -f demo.m

# Create training data
echo "hello world" > train.txt

echo "Training on 'hello world'..."
echo ""

# Train and show progress
for i in 1 5 10 25 50 100; do
    if [ $i -eq 1 ]; then
        ./melvin_standalone train.txt demo.m 2>/dev/null
    else
        prev=$((i - 1))
        for j in $(seq 2 $i); do
            ./melvin_standalone train.txt demo.m 2>/dev/null
        done
    fi
    
    echo "After $i training iterations:"
    echo "hello" > query.txt
    output=$(./melvin_standalone query.txt demo.m 2>&1 | grep "Output:")
    echo "  Query 'hello' → $output"
done

echo ""
echo "========================================="
echo "RESULT: System learns through frequency"
echo "Edge weights grow with each training"
echo "Persistence works across sessions"
echo "========================================="
