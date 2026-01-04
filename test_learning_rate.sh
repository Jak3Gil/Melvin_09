#!/bin/bash
# Test Learning Rate: Measure error rate vs training iterations

TRAIN_FILE="train_learning.txt"
QUERY_FILE="query_learning.txt"
BRAIN="test_learning.m"
RESULTS="learning_results.csv"

# Expected output for query "hello"
EXPECTED=" world"

echo "=== Melvin Learning Rate Test ==="
echo "Training: 'hello world'"
echo "Query: 'hello'"
echo "Expected: '$EXPECTED'"
echo ""
echo "Creating results file: $RESULTS"
echo "iterations,output,correct,error_rate,edge_count,node_count,hierarchy_count" > "$RESULTS"

# Test different training iterations
for iterations in 1 2 3 5 10 15 20 25 30 40 50 75 100; do
    echo "Testing with $iterations training iterations..."
    
    # Clean brain
    rm -f "$BRAIN"
    
    # Train
    echo "hello world" > "$TRAIN_FILE"
    for i in $(seq 1 $iterations); do
        ./melvin_standalone "$TRAIN_FILE" "$BRAIN" > /dev/null 2>&1
    done
    
    # Query
    echo "hello" > "$QUERY_FILE"
    OUTPUT_RAW=$(./melvin_standalone "$QUERY_FILE" "$BRAIN" 2>&1)
    OUTPUT=$(echo "$OUTPUT_RAW" | grep "Output:" | sed 's/Output: "\(.*\)".*/\1/')
    
    # Get stats
    NODES=$(echo "$OUTPUT_RAW" | grep "Nodes:" | awk '{print $2}')
    EDGES=$(echo "$OUTPUT_RAW" | grep "Edges:" | awk '{print $2}')
    
    # Count hierarchies (from stderr or need to parse differently)
    # For now, estimate from node count growth
    HIERARCHIES=0
    
    # Compute error rate using edit distance
    if [ -z "$OUTPUT" ]; then
        OUTPUT="(empty)"
        CORRECT="NO"
        ERROR_RATE="100.0"
    elif [ "$OUTPUT" = "$EXPECTED" ]; then
        CORRECT="YES"
        ERROR_RATE="0.0"
    else
        CORRECT="NO"
        # Compute character-level error rate
        # Count matching characters at each position
        MATCHES=0
        MAX_LEN=$((${#OUTPUT} > ${#EXPECTED} ? ${#OUTPUT} : ${#EXPECTED}))
        MIN_LEN=$((${#OUTPUT} < ${#EXPECTED} ? ${#OUTPUT} : ${#EXPECTED}))
        
        # Count exact matches
        for ((i=0; i<$MIN_LEN; i++)); do
            if [ "${OUTPUT:$i:1}" = "${EXPECTED:$i:1}" ]; then
                MATCHES=$((MATCHES + 1))
            fi
        done
        
        # Error = (total_chars - matches) / max_length
        if [ $MAX_LEN -gt 0 ]; then
            DIFF=$((MAX_LEN - MATCHES))
            ERROR_RATE=$(echo "scale=1; ($DIFF * 100) / $MAX_LEN" | bc)
        else
            ERROR_RATE="100.0"
        fi
    fi
    
    # Record results
    echo "$iterations,\"$OUTPUT\",$CORRECT,$ERROR_RATE,$EDGES,$NODES,$HIERARCHIES" >> "$RESULTS"
    
    printf "  Iterations: %3d | Output: %-20s | Correct: %-3s | Error: %5s%% | Nodes: %3s | Edges: %3s\n" \
           "$iterations" "$OUTPUT" "$CORRECT" "$ERROR_RATE" "$NODES" "$EDGES"
done

echo ""
echo "Results saved to: $RESULTS"
echo ""
echo "=== Summary Statistics ==="
echo "Average error rate by training range:"
awk -F',' 'NR>1 {
    if ($1 <= 5) { early+=$4; early_count++ }
    else if ($1 <= 20) { mid+=$4; mid_count++ }
    else { late+=$4; late_count++ }
}
END {
    if (early_count > 0) printf "  1-5 iterations:   %.1f%% (n=%d)\n", early/early_count, early_count
    if (mid_count > 0) printf "  6-20 iterations:  %.1f%% (n=%d)\n", mid/mid_count, mid_count
    if (late_count > 0) printf "  21-100 iterations: %.1f%% (n=%d)\n", late/late_count, late_count
}' "$RESULTS"

echo ""
echo "=== Learning Trend Analysis ==="
awk -F',' 'NR>1 {
    print $1, $4
}' "$RESULTS" | awk '{
    if (NR == 1) { first=$2; last=$2 }
    last=$2
    count++
    sum+=$2
}
END {
    if (count > 0) {
        avg = sum/count
        trend = last - first
        printf "  Initial error: %.1f%%\n", first
        printf "  Final error:   %.1f%%\n", last
        printf "  Average error: %.1f%%\n", avg
        printf "  Trend:         %.1f%% (%s)\n", trend, (trend < 0 ? "IMPROVING" : (trend > 0 ? "DEGRADING" : "STABLE"))
    }
}'

