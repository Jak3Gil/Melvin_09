#!/bin/bash
# Self-Supervised Learning Test with Iteration Tracking
# Tests error rate progression over multiple training iterations

BRAIN="brain_self_supervised.m"
TRAIN_DATA="train_data.txt"
RESULTS="self_supervised_results.csv"

# Clean start
rm -f "$BRAIN" "$RESULTS"

# Training data
echo "hello world" > "$TRAIN_DATA"

# CSV header
echo "iteration,training_error,test_error,predictions,nodes,edges,hierarchies,output" > "$RESULTS"

# Run multiple iterations
MAX_ITERATIONS=50
echo "=== Self-Supervised Learning Test ==="
echo "Training on: $(cat $TRAIN_DATA)"
echo "Iterations: $MAX_ITERATIONS"
echo ""

for iter in $(seq 1 $MAX_ITERATIONS); do
    echo -n "Iteration $iter..."
    
    # Run test (trains and queries)
    OUTPUT=$(./test_self_supervised "$BRAIN" "$TRAIN_DATA" 2>&1)
    
    # Extract metrics
    TRAIN_ERROR=$(echo "$OUTPUT" | grep "Average training error:" | awk '{print $4}')
    TEST_ERROR=$(echo "$OUTPUT" | grep "Average test error:" | awk '{print $4}')
    PREDICTIONS=$(echo "$OUTPUT" | grep "Training predictions:" | awk '{print $3}')
    NODES=$(echo "$OUTPUT" | grep "^Nodes:" | awk '{print $2}')
    EDGES=$(echo "$OUTPUT" | grep "^Edges:" | awk '{print $2}')
    HIERARCHIES=$(echo "$OUTPUT" | grep "^Hierarchies:" | awk '{print $2}')
    
    # Extract actual output (between "Output: " and next line)
    ACTUAL_OUTPUT=$(echo "$OUTPUT" | grep "^Output:" | head -1 | sed 's/Output: "\(.*\)"/\1/')
    
    # Default values if extraction failed
    TRAIN_ERROR=${TRAIN_ERROR:-1.0000}
    TEST_ERROR=${TEST_ERROR:-1.0000}
    PREDICTIONS=${PREDICTIONS:-0}
    NODES=${NODES:-0}
    EDGES=${EDGES:-0}
    HIERARCHIES=${HIERARCHIES:-0}
    ACTUAL_OUTPUT=${ACTUAL_OUTPUT:-""}
    
    # Save to CSV
    echo "$iter,$TRAIN_ERROR,$TEST_ERROR,$PREDICTIONS,$NODES,$EDGES,$HIERARCHIES,\"$ACTUAL_OUTPUT\"" >> "$RESULTS"
    
    echo " train_err=$TRAIN_ERROR test_err=$TEST_ERROR nodes=$NODES edges=$EDGES output=\"$ACTUAL_OUTPUT\""
done

echo ""
echo "=== RESULTS SUMMARY ==="
echo ""

# Calculate statistics
echo "Error Rate Progression:"
awk -F',' 'NR>1 {
    if (NR==2) {
        first_train=$2; first_test=$3;
    }
    last_train=$2; last_test=$3;
}
END {
    print "  First iteration:  train=" first_train " test=" first_test;
    print "  Last iteration:   train=" last_train " test=" last_test;
    print "  Training improvement: " (first_train - last_train);
    print "  Test improvement:     " (first_test - last_test);
}' "$RESULTS"

echo ""
echo "Graph Growth:"
awk -F',' 'NR>1 {
    if (NR==2) {
        first_nodes=$5; first_edges=$6; first_hier=$7;
    }
    last_nodes=$5; last_edges=$6; last_hier=$7;
}
END {
    print "  Nodes: " first_nodes " -> " last_nodes " (+" (last_nodes - first_nodes) ")";
    print "  Edges: " first_edges " -> " last_edges " (+" (last_edges - first_edges) ")";
    print "  Hierarchies: " first_hier " -> " last_hier " (+" (last_hier - first_hier) ")";
}' "$RESULTS"

echo ""
echo "Learning Trend Analysis:"
awk -F',' 'NR>1 {
    iter=$1; train=$2; test=$3;
    if (iter <= 10) {
        early_train += train; early_test += test; early_count++;
    } else if (iter <= 30) {
        mid_train += train; mid_test += test; mid_count++;
    } else {
        late_train += train; late_test += test; late_count++;
    }
}
END {
    print "  Early (1-10):   train=" (early_train/early_count) " test=" (early_test/early_count);
    print "  Middle (11-30): train=" (mid_train/mid_count) " test=" (mid_test/mid_count);
    print "  Late (31-50):   train=" (late_train/late_count) " test=" (late_test/late_count);
    
    if ((early_test/early_count) > (late_test/late_count)) {
        print "  ✓ Learning is occurring (error decreasing)";
    } else {
        print "  ✗ No learning detected (error not decreasing)";
    }
}' "$RESULTS"

echo ""
echo "Output Evolution:"
awk -F',' 'NR>1 {
    if (NR==2 || NR==11 || NR==31 || NR==51) {
        print "  Iter " $1 ": " $8;
    }
}' "$RESULTS"

echo ""
echo "Full results saved to: $RESULTS"
echo ""
echo "To visualize:"
echo "  cat $RESULTS | column -t -s','"

