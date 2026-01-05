#!/bin/bash

echo "=== DETAILED ADAPTIVE BEHAVIOR ANALYSIS ==="
echo ""

# Test: Track actual outputs to see learning progression
echo "TEST: Output Quality Over Iterations"
echo "Training on 'hello world', 'the cat sat', 'good morning'"
echo ""

for iters in 1 3 5 10 20 30; do
    echo "=== ITERATION $iters ==="
    rm -f brain_detail.m
    ./test_self_supervised train_data.txt brain_detail.m $iters 2>&1 | grep -A 3 "Query: \"hello\"" | head -4
    echo ""
done

echo ""
echo "=== GRAPH GROWTH ANALYSIS ==="
echo "iteration,nodes,edges,edge_per_node"
for iters in 1 2 5 10 15 20 25 30; do
    rm -f brain_growth.m
    stats=$(./test_self_supervised train_data.txt brain_growth.m $iters 2>&1 | grep -E "^Nodes:|^Edges:")
    nodes=$(echo "$stats" | grep "Nodes:" | awk '{print $2}')
    edges=$(echo "$stats" | grep "Edges:" | awk '{print $2}')
    if [ ! -z "$nodes" ] && [ ! -z "$edges" ] && [ "$nodes" != "0" ]; then
        ratio=$(echo "scale=2; $edges / $nodes" | bc)
        echo "$iters,$nodes,$edges,$ratio"
    fi
done

echo ""
echo "=== ADAPTIVE PARAMETER OBSERVATION ==="
echo "Testing with different graph densities..."

# Sparse graph test
echo ""
echo "Sparse graph (few connections):"
cat > sparse.txt << 'EOFDATA'
a
b
c
EOFDATA
rm -f brain_sparse.m
./test_self_supervised sparse.txt brain_sparse.m 5 2>&1 | grep -E "(Nodes:|Edges:|Average)" | tail -5

# Dense graph test
echo ""
echo "Dense graph (many connections):"
cat > dense.txt << 'EOFDATA'
abcabc
abcabc
abcabc
EOFDATA
rm -f brain_dense.m
./test_self_supervised dense.txt brain_dense.m 5 2>&1 | grep -E "(Nodes:|Edges:|Average)" | tail -5

echo ""
echo "=== TESTS COMPLETE ==="
