#!/bin/bash
rm -f test_evo.m
echo "=== Training Evolution: 'hello' ==="
echo ""
for i in {1..200}; do
    echo -e "hello\nquit" | ./test_dataset /dev/null test_evo.m > /tmp/evo_$i.txt 2>&1
    if [ $((i % 20)) -eq 0 ] || [ $i -eq 1 ] || [ $i -eq 5 ] || [ $i -eq 10 ]; then
        output=$(grep -E "^Output:" /tmp/evo_$i.txt | head -1)
        nodes=$(grep "Nodes:" /tmp/evo_$i.txt | tail -1)
        edges=$(grep "Edges:" /tmp/evo_$i.txt | tail -1)
        echo "Iteration $i: $output ($nodes, $edges)"
    fi
done
