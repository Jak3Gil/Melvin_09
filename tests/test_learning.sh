#!/bin/bash

BRAIN="test_learning_brain.m"
rm -f "$BRAIN"

echo "=== Learning Test: Does the system learn sequential patterns? ==="
echo ""

# Test 1: First "hello"
echo "--- Test 1: First input 'hello' ---"
echo "hello" > /tmp/test1.txt
OUTPUT1=$(./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "sampled_byte" | head -10 | sed 's/.*sampled_byte=0x\([0-9a-f]*\).*/\1/' | xargs -I {} printf "\\x{}\n" | tr -d '\n' | sed 's/\\x//g' | xxd -r -p 2>/dev/null || echo "")
echo "Output: '$OUTPUT1'"
echo ""

# Test 2: Second "hello" (should strengthen h->e->l->l->o)
echo "--- Test 2: Second input 'hello' (should strengthen edges) ---"
OUTPUT2=$(./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "sampled_byte" | head -10 | sed 's/.*sampled_byte=0x\([0-9a-f]*\).*/\1/' | xargs -I {} printf "\\x{}\n" | tr -d '\n' | sed 's/\\x//g' | xxd -r -p 2>/dev/null || echo "")
echo "Output: '$OUTPUT2'"
echo ""

# Test 3: "world"
echo "--- Test 3: New input 'world' ---"
echo "world" > /tmp/test2.txt
OUTPUT3=$(./melvin_standalone /tmp/test2.txt "$BRAIN" 2>&1 | grep -E "sampled_byte" | head -10 | sed 's/.*sampled_byte=0x\([0-9a-f]*\).*/\1/' | xargs -I {} printf "\\x{}\n" | tr -d '\n' | sed 's/\\x//g' | xxd -r -p 2>/dev/null || echo "")
echo "Output: '$OUTPUT3'"
echo ""

# Test 4: "hello world" (should use learned h->e->l->l->o and o->space->w->o->r->l->d)
echo "--- Test 4: 'hello world' (should follow learned paths) ---"
echo "hello world" > /tmp/test3.txt
OUTPUT4=$(./melvin_standalone /tmp/test3.txt "$BRAIN" 2>&1 | grep -E "sampled_byte" | head -20 | sed 's/.*sampled_byte=0x\([0-9a-f]*\).*/\1/' | xargs -I {} printf "\\x{}\n" | tr -d '\n' | sed 's/\\x//g' | xxd -r -p 2>/dev/null || echo "")
echo "Output: '$OUTPUT4'"
echo ""

# Test 5: Another "hello" (should be stronger now)
echo "--- Test 5: Third 'hello' (edges should be stronger) ---"
OUTPUT5=$(./melvin_standalone /tmp/test1.txt "$BRAIN" 2>&1 | grep -E "sampled_byte" | head -10 | sed 's/.*sampled_byte=0x\([0-9a-f]*\).*/\1/' | xargs -I {} printf "\\x{}\n" | tr -d '\n' | sed 's/\\x//g' | xxd -r -p 2>/dev/null || echo "")
echo "Output: '$OUTPUT5'"
echo ""

echo "=== Analysis ==="
echo "Test 1 output: '$OUTPUT1'"
echo "Test 2 output: '$OUTPUT2'"
echo "Test 3 output: '$OUTPUT3'"
echo "Test 4 output: '$OUTPUT4'"
echo "Test 5 output: '$OUTPUT5'"
echo ""
echo "Learning indicators:"
if [ "$OUTPUT2" != "$OUTPUT1" ]; then
    echo "  ✅ Output changed between test 1 and 2 (system adapting)"
else
    echo "  ⚠️  Output same between test 1 and 2"
fi

if [[ "$OUTPUT4" == *"hello"* ]] || [[ "$OUTPUT4" == *"world"* ]]; then
    echo "  ✅ Test 4 output contains learned patterns"
else
    echo "  ⚠️  Test 4 output doesn't show learned patterns"
fi

