#!/bin/bash

echo "=== INTENSIVE TESTING: ADAPTIVE MELVIN ==="
echo ""

# Test 1: Small dataset, many iterations
echo "TEST 1: Small dataset (3 sequences), 30 iterations"
echo "Expected: Should adapt with fewer hops, faster decay"
rm -f brain_small.m
./test_self_supervised train_data.txt brain_small.m 30 2>&1 | grep -E "(Average|Nodes:|Edges:|Hierarchies)" | tail -10
echo ""

# Test 2: Larger dataset, moderate iterations
echo "TEST 2: Larger dataset (10 sequences), 20 iterations"
cat > large_train.txt << 'EOFDATA'
hello world
the cat sat
good morning
hello world
the cat sat
good morning
hello there
world peace
morning sun
cat nap
EOFDATA
rm -f brain_large.m
./test_self_supervised large_train.txt brain_large.m 20 2>&1 | grep -E "(Average|Nodes:|Edges:|training_error|test_error)" | tail -10
echo ""

# Test 3: Convergence test - track error over iterations
echo "TEST 3: Convergence tracking (1, 5, 10, 20, 30 iterations)"
echo "iteration,train_error,test_error,nodes,edges"
for iters in 1 5 10 20 30; do
    rm -f brain_conv.m
    result=$(./test_self_supervised train_data.txt brain_conv.m $iters 2>&1 | grep "^training_error" -A 1 | tail -1)
    echo "$iters,$result"
done
echo ""

# Test 4: Pattern complexity test
echo "TEST 4: Simple vs Complex patterns"
echo "4a: Simple repeated pattern"
cat > simple.txt << 'EOFDATA'
aa
aa
aa
EOFDATA
rm -f brain_simple.m
./test_self_supervised simple.txt brain_simple.m 10 2>&1 | grep -E "(Average|Nodes:|Edges:)" | tail -5

echo ""
echo "4b: Complex varied patterns"
cat > complex.txt << 'EOFDATA'
abcdefgh
ijklmnop
qrstuvwx
EOFDATA
rm -f brain_complex.m
./test_self_supervised complex.txt brain_complex.m 10 2>&1 | grep -E "(Average|Nodes:|Edges:)" | tail -5
echo ""

# Test 5: Hierarchy formation tracking
echo "TEST 5: Hierarchy formation over iterations"
echo "iteration,hierarchies_formed"
for iters in 5 10 15 20 25 30; do
    rm -f brain_hier.m
    hier_count=$(./test_self_supervised train_data.txt brain_hier.m $iters 2>&1 | grep -c "\[HIERARCHY\]")
    echo "$iters,$hier_count"
done
echo ""

echo "=== TESTS COMPLETE ==="
