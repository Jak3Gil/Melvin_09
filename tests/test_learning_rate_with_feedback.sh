#!/bin/bash
# Test Learning Rate WITH Error Feedback: Measure error rate vs training iterations

TRAIN_FILE="train_learning.txt"
QUERY_FILE="query_learning.txt"
BRAIN="test_learning_feedback.m"
RESULTS="learning_results_feedback.csv"
TEST_PROG="./test_learning_with_feedback"

# Expected output for query "hello"
EXPECTED=" world"

echo "=== Melvin Learning Rate Test (WITH Error Feedback) ==="
echo "Training: 'hello world'"
echo "Query: 'hello'"
echo "Expected: '$EXPECTED'"
echo "Error Feedback: ENABLED"
echo ""

# Build test program if needed
if [ ! -f "$TEST_PROG" ]; then
    echo "Building test program..."
    gcc -O2 -Wall -Wextra test_learning_with_feedback.c melvin.o melvin_in_port.o melvin_out_port.o -o "$TEST_PROG" -lm
    if [ $? -ne 0 ]; then
        echo "Error: Failed to build test program"
        exit 1
    fi
fi

echo "Creating results file: $RESULTS"
echo "iterations,output,error_signal,error_rate,edge_count,node_count,hierarchy_count" > "$RESULTS"

# Test different training iterations
for iterations in 1 2 3 5 10 15 20 25 30 40 50 75 100; do
    echo "Testing with $iterations training iterations..."
    
    # Clean brain
    rm -f "$BRAIN"
    
    # Train with error feedback after each iteration
    echo "hello world" > "$TRAIN_FILE"
    echo "hello" > "$QUERY_FILE"
    
    # Train for N iterations, applying error feedback after each training+query cycle
    for i in $(seq 1 $iterations); do
        # Training: process input (frequency-based Hebbian learning)
        ./melvin_standalone "$TRAIN_FILE" "$BRAIN" > /dev/null 2>&1
        
        # Query and apply error feedback (error-based learning)
        # This applies error feedback after each training iteration
        "$TEST_PROG" "$TRAIN_FILE" "$QUERY_FILE" "$BRAIN" "$EXPECTED" > /dev/null 2>&1
    done
    
    # Final query to measure performance (without applying feedback, just measuring)
    OUTPUT_RAW=$("$TEST_PROG" "$TRAIN_FILE" "$QUERY_FILE" "$BRAIN" "$EXPECTED" "no_feedback" 2>&1)
    
    # Parse output
    OUTPUT=$(echo "$OUTPUT_RAW" | grep "OUTPUT: " | sed 's/OUTPUT: //')
    ERROR_SIGNAL=$(echo "$OUTPUT_RAW" | grep "ERROR_SIGNAL: " | awk '{print $2}')
    NODES=$(echo "$OUTPUT_RAW" | grep "NODES: " | awk '{print $2}')
    EDGES=$(echo "$OUTPUT_RAW" | grep "EDGES: " | awk '{print $2}')
    HIERARCHIES=$(echo "$OUTPUT_RAW" | grep "HIERARCHIES: " | awk '{print $2}')
    
    # Compute error rate from error signal (1.0 = perfect, 0.0 = wrong)
    # Error rate percentage = (1.0 - error_signal) * 100
    if [ -z "$ERROR_SIGNAL" ]; then
        ERROR_RATE="100.0"
    else
        ERROR_RATE=$(echo "scale=1; (1.0 - $ERROR_SIGNAL) * 100" | bc)
    fi
    
    # Clean up output string (remove escape sequences for CSV)
    OUTPUT_CLEAN=$(echo "$OUTPUT" | sed 's/\\x[0-9a-f][0-9a-f]/?/g')
    
    # Determine if correct
    if [ -z "$OUTPUT_CLEAN" ]; then
        OUTPUT_CLEAN="(empty)"
        CORRECT="NO"
    elif [ "$OUTPUT_CLEAN" = "$EXPECTED" ]; then
        CORRECT="YES"
    else
        CORRECT="NO"
    fi
    
    # Record results
    echo "$iterations,\"$OUTPUT_CLEAN\",$ERROR_SIGNAL,$ERROR_RATE,$EDGES,$NODES,$HIERARCHIES" >> "$RESULTS"
    
    printf "  Iterations: %3d | Output: %-20s | Error Signal: %5.3f | Error Rate: %5s%% | Nodes: %3s | Edges: %3s\n" \
           "$iterations" "$OUTPUT_CLEAN" "$ERROR_SIGNAL" "$ERROR_RATE" "$NODES" "$EDGES"
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

echo ""
echo "=== Error Signal Analysis ==="
awk -F',' 'NR>1 {
    print $1, $3
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
        printf "  Initial error signal: %.3f (1.0 = perfect)\n", first
        printf "  Final error signal:   %.3f (1.0 = perfect)\n", last
        printf "  Average error signal: %.3f (1.0 = perfect)\n", avg
        printf "  Trend:                %+.3f (%s)\n", trend, (trend > 0 ? "IMPROVING" : (trend < 0 ? "DEGRADING" : "STABLE"))
    }
}'

