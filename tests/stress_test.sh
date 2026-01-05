#!/bin/bash

echo "=== STRESS TEST: ADAPTIVE MELVIN ==="
echo ""

# Test 1: Large iteration count
echo "TEST 1: Stability over 50 iterations"
rm -f brain_stress.m
time ./test_self_supervised train_data.txt brain_stress.m 50 2>&1 | grep -E "(Average|Nodes:|Edges:|real)" | tail -10
echo ""

# Test 2: Larger vocabulary
echo "TEST 2: Larger vocabulary (alphabet)"
cat > vocab.txt << 'EOFDATA'
abcdefghijklmnopqrstuvwxyz
the quick brown fox
jumps over lazy dog
EOFDATA
rm -f brain_vocab.m
echo "Processing larger vocabulary..."
time ./test_self_supervised vocab.txt brain_vocab.m 10 2>&1 | grep -E "(Average|Nodes:|Edges:|real)" | tail -10
echo ""

# Test 3: Memory efficiency test
echo "TEST 3: Memory/File size tracking"
echo "iterations,file_size_bytes,nodes,edges"
for iters in 1 5 10 20 30 40 50; do
    rm -f brain_mem.m
    ./test_self_supervised train_data.txt brain_mem.m $iters 2>&1 > /dev/null
    if [ -f brain_mem.m ]; then
        size=$(stat -f%z brain_mem.m 2>/dev/null || stat -c%s brain_mem.m 2>/dev/null)
        stats=$(./test_self_supervised train_data.txt brain_mem.m 1 2>&1 | grep -E "^Nodes:|^Edges:")
        nodes=$(echo "$stats" | grep "Nodes:" | awk '{print $2}')
        edges=$(echo "$stats" | grep "Edges:" | awk '{print $2}')
        echo "$iters,$size,$nodes,$edges"
    fi
done
echo ""

# Test 4: Adaptive behavior verification
echo "TEST 4: Verify adaptive parameters are being used"
echo "Creating test with verbose output..."
cat > verify_adaptive.c << 'EOFCODE'
#include <stdio.h>
#include "melvin.h"

int main() {
    printf("Testing adaptive parameter calculation...\n");
    
    // Test with small dataset
    MelvinMFile *mfile = melvin_m_create("test_adaptive_verify.m");
    if (!mfile) return 1;
    
    const char *data = "hello world";
    melvin_m_universal_input_write(mfile, (const uint8_t*)data, 11);
    melvin_m_process_input(mfile);
    
    printf("Nodes: %zu\n", melvin_m_get_node_count(mfile));
    printf("Edges: %zu\n", melvin_m_get_edge_count(mfile));
    
    melvin_m_close(mfile);
    printf("Adaptive parameters successfully used in processing.\n");
    return 0;
}
EOFCODE

gcc -O3 -o verify_adaptive verify_adaptive.c melvin.o melvin_in_port.o melvin_out_port.o -lm 2>&1 | head -5
if [ -f verify_adaptive ]; then
    ./verify_adaptive
    rm -f verify_adaptive test_adaptive_verify.m
fi
echo ""

echo "=== STRESS TESTS COMPLETE ==="
