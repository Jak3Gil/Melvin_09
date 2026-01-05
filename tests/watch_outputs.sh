#!/bin/bash
# Watch outputs evolve - detailed view

BRAIN="watch_brain.m"
rm -f "$BRAIN"

echo "=========================================="
echo "Melvin: Watching Outputs Evolve"
echo "=========================================="
echo ""
echo "This demonstrates ML-style soft probability output generation."
echo "Watch how outputs change as patterns strengthen."
echo ""

# Function to process input and show details
process_input() {
    local input="$1"
    local label="$2"
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "$label: '$input'"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    # Process and show full output
    echo "$input" | ./test_dataset /dev/null "$BRAIN" 2>&1
    
    echo ""
    sleep 0.3
}

# Sequence showing learning progression
process_input "hello" "Input 1: First 'hello'"
process_input "hello" "Input 2: Second 'hello' (pattern strengthening)"
process_input "hello" "Input 3: Third 'hello' (edges getting stronger)"
process_input "world" "Input 4: First 'world'"
process_input "world" "Input 5: Second 'world'"
process_input "hello world" "Input 6: Novel combination 'hello world'"
process_input "hello" "Input 7: Return to 'hello' (should have stronger patterns now)"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Final Statistics"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "quit" | ./test_dataset /dev/null "$BRAIN" 2>&1 | tail -5

