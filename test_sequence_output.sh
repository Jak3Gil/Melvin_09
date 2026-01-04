#!/bin/bash

# Test script to verify output generation continues after first byte
# Should generate full sequences like "hello" → h→e→l→l→o

set -e

BRAIN_FILE="test_sequence_brain.m"
INPUT_FILE="test_sequence_input.txt"

# Clean up from previous runs
rm -f "$BRAIN_FILE" "$INPUT_FILE"

# Create test input
echo -n "hello" > "$INPUT_FILE"

echo "=== Testing Output Generation ==="
echo "Input: hello"
echo ""

# Process input and check output
./melvin_standalone "$INPUT_FILE" "$BRAIN_FILE" 2>&1 | tee test_sequence_debug.log

echo ""
echo "=== Checking Output ==="

# Check if output was generated
if [ -f "$BRAIN_FILE" ]; then
    echo "Brain file exists"
    
    # Try to read output (this would require a utility, but for now just check file size)
    FILE_SIZE=$(stat -f%z "$BRAIN_FILE" 2>/dev/null || stat -c%s "$BRAIN_FILE" 2>/dev/null || echo "0")
    echo "Brain file size: $FILE_SIZE bytes"
    
    # Check debug log for output information
    if grep -q "OUTPUT.*sampled_byte" test_sequence_debug.log; then
        echo ""
        echo "=== Output Bytes Generated ==="
        grep "OUTPUT.*sampled_byte" test_sequence_debug.log | head -10
        
        OUTPUT_COUNT=$(grep -c "OUTPUT.*sampled_byte" test_sequence_debug.log || echo "0")
        echo ""
        echo "Total output bytes: $OUTPUT_COUNT"
        
        if [ "$OUTPUT_COUNT" -gt 2 ]; then
            echo "✅ SUCCESS: System generated more than 2 bytes!"
        else
            echo "⚠️  WARNING: Only $OUTPUT_COUNT bytes generated (expected more)"
        fi
    else
        echo "⚠️  No output bytes found in debug log"
    fi
    
    # Check stop reason
    if grep -q "Generation complete" test_sequence_debug.log; then
        echo ""
        echo "=== Stop Reason ==="
        grep "Generation complete" test_sequence_debug.log | tail -1
    fi
else
    echo "❌ Brain file not created"
fi

echo ""
echo "=== Test Complete ==="

