#!/bin/bash

echo "=== .M FILE SCALABILITY TEST ==="
echo ""
echo "dataset,input_bytes,m_file_bytes,nodes,edges,compression_ratio,edge_per_node"
echo "------------------------------------------------------------------------"

# Test 1: Tiny
cat > tiny.txt << 'DATA'
hello
DATA
rm -f brain_tiny.m
result=$(./test_self_supervised tiny.txt brain_tiny.m 10 2>&1)
if [ -f brain_tiny.m ]; then
    input_size=$(wc -c < tiny.txt | tr -d ' ')
    file_size=$(stat -f%z brain_tiny.m 2>/dev/null || stat -c%s brain_tiny.m 2>/dev/null)
    nodes=$(echo "$result" | grep "^Nodes:" | awk '{print $2}')
    edges=$(echo "$result" | grep "^Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$file_size" -gt 0 ]; then
        ratio=$(echo "scale=2; $input_size / $file_size" | bc)
        epn=$(echo "scale=2; $edges / $nodes" | bc)
        echo "tiny,$input_size,$file_size,$nodes,$edges,$ratio,$epn"
    fi
fi

# Test 2: Small
cat > small.txt << 'DATA'
hello world
the cat sat
good morning
hello there
world peace
DATA
rm -f brain_small.m
result=$(./test_self_supervised small.txt brain_small.m 10 2>&1)
if [ -f brain_small.m ]; then
    input_size=$(wc -c < small.txt | tr -d ' ')
    file_size=$(stat -f%z brain_small.m 2>/dev/null || stat -c%s brain_small.m 2>/dev/null)
    nodes=$(echo "$result" | grep "^Nodes:" | awk '{print $2}')
    edges=$(echo "$result" | grep "^Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$file_size" -gt 0 ]; then
        ratio=$(echo "scale=2; $input_size / $file_size" | bc)
        epn=$(echo "scale=2; $edges / $nodes" | bc)
        echo "small,$input_size,$file_size,$nodes,$edges,$ratio,$epn"
    fi
fi

# Test 3: Medium
cat > medium.txt << 'DATA'
The quick brown fox jumps over the lazy dog.
Pack my box with five dozen liquor jugs.
How vexingly quick daft zebras jump!
The five boxing wizards jump quickly.
Sphinx of black quartz, judge my vow.
Waltz, bad nymph, for quick jigs vex.
Quick zephyrs blow, vexing daft Jim.
Two driven jocks help fax my big quiz.
Five quacking zephyrs jolt my wax bed.
The jay, pig, fox, zebra and my wolves quack!
DATA
rm -f brain_medium.m
result=$(./test_self_supervised medium.txt brain_medium.m 10 2>&1)
if [ -f brain_medium.m ]; then
    input_size=$(wc -c < medium.txt | tr -d ' ')
    file_size=$(stat -f%z brain_medium.m 2>/dev/null || stat -c%s brain_medium.m 2>/dev/null)
    nodes=$(echo "$result" | grep "^Nodes:" | awk '{print $2}')
    edges=$(echo "$result" | grep "^Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$file_size" -gt 0 ]; then
        ratio=$(echo "scale=2; $input_size / $file_size" | bc)
        epn=$(echo "scale=2; $edges / $nodes" | bc)
        echo "medium,$input_size,$file_size,$nodes,$edges,$ratio,$epn"
    fi
fi

# Test 4: Large (10KB)
python3 << 'PYSCRIPT'
import random
words = ["hello", "world", "the", "cat", "sat", "good", "morning", "quick", "brown", "fox"]
with open("large.txt", "w") as f:
    for _ in range(500):
        sentence = " ".join(random.choices(words, k=5))
        f.write(sentence + "\n")
PYSCRIPT
rm -f brain_large.m
result=$(./test_self_supervised large.txt brain_large.m 5 2>&1)
if [ -f brain_large.m ]; then
    input_size=$(wc -c < large.txt | tr -d ' ')
    file_size=$(stat -f%z brain_large.m 2>/dev/null || stat -c%s brain_large.m 2>/dev/null)
    nodes=$(echo "$result" | grep "^Nodes:" | awk '{print $2}')
    edges=$(echo "$result" | grep "^Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$file_size" -gt 0 ]; then
        ratio=$(echo "scale=2; $input_size / $file_size" | bc)
        epn=$(echo "scale=2; $edges / $nodes" | bc)
        echo "large,$input_size,$file_size,$nodes,$edges,$ratio,$epn"
    fi
fi

# Test 5: XLarge (50KB)
python3 << 'PYSCRIPT'
import random
words = ["hello", "world", "the", "cat", "sat", "good", "morning", "quick", "brown", "fox"]
with open("xlarge.txt", "w") as f:
    for _ in range(2500):
        sentence = " ".join(random.choices(words, k=5))
        f.write(sentence + "\n")
PYSCRIPT
echo "Processing 50KB dataset..."
rm -f brain_xlarge.m
result=$(./test_self_supervised xlarge.txt brain_xlarge.m 3 2>&1)
if [ -f brain_xlarge.m ]; then
    input_size=$(wc -c < xlarge.txt | tr -d ' ')
    file_size=$(stat -f%z brain_xlarge.m 2>/dev/null || stat -c%s brain_xlarge.m 2>/dev/null)
    nodes=$(echo "$result" | grep "^Nodes:" | awk '{print $2}')
    edges=$(echo "$result" | grep "^Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$file_size" -gt 0 ]; then
        ratio=$(echo "scale=2; $input_size / $file_size" | bc)
        epn=$(echo "scale=2; $edges / $nodes" | bc)
        echo "xlarge,$input_size,$file_size,$nodes,$edges,$ratio,$epn"
    fi
fi

echo ""
echo "=== ANALYSIS ==="
echo "Compression Ratio = Input Size / File Size (higher = better compression)"
echo "Edge/Node Ratio = Edges / Nodes (~2.0 = optimal sparse graph)"

