#!/bin/bash

echo "=== .M FILE SCALABILITY TEST ==="
echo ""

# Test 1: Tiny dataset
echo "TEST 1: TINY (10 bytes)"
cat > tiny.txt << 'DATA'
hello
DATA
rm -f brain_tiny.m
./test_self_supervised tiny.txt brain_tiny.m 10 2>&1 > /dev/null
if [ -f brain_tiny.m ]; then
    size=$(stat -f%z brain_tiny.m 2>/dev/null || stat -c%s brain_tiny.m 2>/dev/null)
    nodes=$(./melvin_standalone brain_tiny.m < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
    edges=$(./melvin_standalone brain_tiny.m < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
    echo "  Data: 10 bytes | .m file: $size bytes | Nodes: $nodes | Edges: $edges"
fi

# Test 2: Small dataset
echo ""
echo "TEST 2: SMALL (100 bytes)"
cat > small.txt << 'DATA'
hello world
the cat sat
good morning
hello there
world peace
DATA
rm -f brain_small.m
./test_self_supervised small.txt brain_small.m 10 2>&1 > /dev/null
if [ -f brain_small.m ]; then
    size=$(stat -f%z brain_small.m 2>/dev/null || stat -c%s brain_small.m 2>/dev/null)
    nodes=$(./melvin_standalone brain_small.m < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
    edges=$(./melvin_standalone brain_small.m < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
    echo "  Data: ~100 bytes | .m file: $size bytes | Nodes: $nodes | Edges: $edges"
fi

# Test 3: Medium dataset
echo ""
echo "TEST 3: MEDIUM (1KB)"
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
Blowzy red vixens fight for a quick jump.
Joaquin Phoenix was gazed by MTV for luck.
A wizard's job is to vex chumps quickly in fog.
Watch Jeopardy, Alex Trebek's fun TV quiz game.
DATA
rm -f brain_medium.m
./test_self_supervised medium.txt brain_medium.m 10 2>&1 > /dev/null
if [ -f brain_medium.m ]; then
    size=$(stat -f%z brain_medium.m 2>/dev/null || stat -c%s brain_medium.m 2>/dev/null)
    nodes=$(./melvin_standalone brain_medium.m < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
    edges=$(./melvin_standalone brain_medium.m < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
    echo "  Data: ~1KB | .m file: $size bytes | Nodes: $nodes | Edges: $edges"
fi

# Test 4: Large dataset (generated)
echo ""
echo "TEST 4: LARGE (10KB)"
python3 << 'PYSCRIPT'
import random
words = ["hello", "world", "the", "cat", "sat", "good", "morning", "quick", "brown", "fox", 
         "jumps", "over", "lazy", "dog", "pack", "box", "with", "five", "dozen", "liquor"]
with open("large.txt", "w") as f:
    for _ in range(500):
        sentence = " ".join(random.choices(words, k=5))
        f.write(sentence + "\n")
PYSCRIPT
rm -f brain_large.m
./test_self_supervised large.txt brain_large.m 10 2>&1 > /dev/null
if [ -f brain_large.m ]; then
    size=$(stat -f%z brain_large.m 2>/dev/null || stat -c%s brain_large.m 2>/dev/null)
    nodes=$(./melvin_standalone brain_large.m < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
    edges=$(./melvin_standalone brain_large.m < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
    data_size=$(wc -c < large.txt | tr -d ' ')
    echo "  Data: $data_size bytes | .m file: $size bytes | Nodes: $nodes | Edges: $edges"
fi

# Test 5: Very large dataset
echo ""
echo "TEST 5: VERY LARGE (100KB)"
python3 << 'PYSCRIPT'
import random
words = ["hello", "world", "the", "cat", "sat", "good", "morning", "quick", "brown", "fox", 
         "jumps", "over", "lazy", "dog", "pack", "box", "with", "five", "dozen", "liquor",
         "sphinx", "quartz", "judge", "vow", "waltz", "nymph", "zephyr", "vex", "jim"]
with open("xlarge.txt", "w") as f:
    for _ in range(5000):
        sentence = " ".join(random.choices(words, k=5))
        f.write(sentence + "\n")
PYSCRIPT
rm -f brain_xlarge.m
echo "  Processing 100KB dataset (this may take a moment)..."
./test_self_supervised xlarge.txt brain_xlarge.m 5 2>&1 > /dev/null
if [ -f brain_xlarge.m ]; then
    size=$(stat -f%z brain_xlarge.m 2>/dev/null || stat -c%s brain_xlarge.m 2>/dev/null)
    nodes=$(./melvin_standalone brain_xlarge.m < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
    edges=$(./melvin_standalone brain_xlarge.m < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
    data_size=$(wc -c < xlarge.txt | tr -d ' ')
    echo "  Data: $data_size bytes | .m file: $size bytes | Nodes: $nodes | Edges: $edges"
fi

echo ""
echo "=== COMPRESSION ANALYSIS ==="
echo "data_size,file_size,nodes,edges,compression_ratio"
for brain in brain_tiny.m brain_small.m brain_medium.m brain_large.m brain_xlarge.m; do
    if [ -f "$brain" ]; then
        size=$(stat -f%z "$brain" 2>/dev/null || stat -c%s "$brain" 2>/dev/null)
        nodes=$(./melvin_standalone "$brain" < /dev/null 2>&1 | grep "Nodes:" | awk '{print $2}')
        edges=$(./melvin_standalone "$brain" < /dev/null 2>&1 | grep "Edges:" | awk '{print $2}')
        
        # Estimate original data size
        case "$brain" in
            brain_tiny.m) data=10 ;;
            brain_small.m) data=100 ;;
            brain_medium.m) data=1000 ;;
            brain_large.m) data=$(wc -c < large.txt | tr -d ' ') ;;
            brain_xlarge.m) data=$(wc -c < xlarge.txt | tr -d ' ') ;;
        esac
        
        if [ "$size" -gt 0 ]; then
            ratio=$(echo "scale=2; $data / $size" | bc)
            echo "$data,$size,$nodes,$edges,$ratio"
        fi
    fi
done

echo ""
echo "=== SCALABILITY METRICS ==="
echo ""
echo "Compression Ratio = Original Data Size / .m File Size"
echo "Higher ratio = Better compression"
echo ""
echo "Edge/Node Ratio = Edges / Nodes"
echo "~2.0 = Optimal sparse graph"
echo ""

