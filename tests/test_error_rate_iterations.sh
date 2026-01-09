#!/bin/bash
# Comprehensive Error Rate Test Over Iterations
# Tests how error rate changes as the system learns with error feedback

set -e

BRAIN_FILE="error_test.m"
RESULTS_FILE="error_test_results.txt"
TEMP_DIR="/tmp/error_test"

# Create temp directory
mkdir -p "$TEMP_DIR"

# Clean start
rm -f "$BRAIN_FILE" "$RESULTS_FILE"

echo "========================================" | tee "$RESULTS_FILE"
echo "Error Rate Test Over Iterations" | tee -a "$RESULTS_FILE"
echo "========================================" | tee -a "$RESULTS_FILE"
echo "Date: $(date)" | tee -a "$RESULTS_FILE"
echo "" | tee -a "$RESULTS_FILE"

# Test patterns: (input, expected_output)
declare -a PATTERNS=(
    "hello:lo"
    "world:ld"
    "test:t"
    "learn:rn"
    "quick:ck"
)

# Function to compute error rate between two strings
compute_error_rate() {
    local output="$1"
    local expected="$2"
    
    if [ -z "$output" ] && [ -z "$expected" ]; then
        echo "0.0"
        return
    fi
    
    if [ -z "$output" ]; then
        echo "100.0"
        return
    fi
    
    if [ -z "$expected" ]; then
        echo "100.0"
        return
    fi
    
    # Calculate Levenshtein distance (simplified)
    local len_output=${#output}
    local len_expected=${#expected}
    local matches=0
    local min_len=$((len_output < len_expected ? len_output : len_expected))
    
    # Count character matches
    for ((i=0; i<min_len; i++)); do
        if [ "${output:$i:1}" = "${expected:$i:1}" ]; then
            ((matches++))
        fi
    done
    
    # Calculate error rate
    local max_len=$((len_output > len_expected ? len_output : len_expected))
    if [ $max_len -eq 0 ]; then
        echo "0.0"
        return
    fi
    
    local match_rate=$(echo "scale=2; $matches / $max_len * 100" | bc)
    local error_rate=$(echo "scale=2; 100 - $match_rate" | bc)
    echo "$error_rate"
}

# Function to get output from melvin
get_output() {
    local input_file="$1"
    local output=$(./melvin_feed "$input_file" "$BRAIN_FILE" --passes 1 2>/dev/null | grep -E "Output:|Generated:" | tail -1 | sed 's/.*Output: //; s/.*Generated: //' | cut -d' ' -f1 | tr -d '\n' || echo "")
    echo "$output"
}

# Function to get graph stats
get_stats() {
    local stats=$(./melvin_feed "$1" "$BRAIN_FILE" --passes 1 2>/dev/null 2>/dev/null | grep -E "Nodes:|Edges:" | tail -2)
    local nodes=$(echo "$stats" | grep "Nodes:" | awk '{print $2}' | head -1)
    local edges=$(echo "$stats" | grep "Edges:" | awk '{print $2}' | head -1)
    echo "$nodes $edges"
}

echo "Test Configuration:" | tee -a "$RESULTS_FILE"
echo "  Patterns: ${#PATTERNS[@]}" | tee -a "$RESULTS_FILE"
echo "  Iterations per pattern: 50" | tee -a "$RESULTS_FILE"
echo "  Total iterations: $((${#PATTERNS[@]} * 50))" | tee -a "$RESULTS_FILE"
echo "" | tee -a "$RESULTS_FILE"

# Test each pattern
for pattern in "${PATTERNS[@]}"; do
    IFS=':' read -r input expected <<< "$pattern"
    
    echo "========================================" | tee -a "$RESULTS_FILE"
    echo "Testing Pattern: '$input' -> Expected: '$expected'" | tee -a "$RESULTS_FILE"
    echo "========================================" | tee -a "$RESULTS_FILE"
    echo "" | tee -a "$RESULTS_FILE"
    
    # Create input file
    input_file="$TEMP_DIR/input_${input}.txt"
    echo -n "$input" > "$input_file"
    
    # Header
    echo "Iteration | Output  | Expected | Error % | Nodes | Edges | Avg Error %" | tee -a "$RESULTS_FILE"
    echo "----------|---------|----------|---------|-------|-------|------------" | tee -a "$RESULTS_FILE"
    
    total_error=0
    iteration_count=0
    
    # Run iterations
    for iteration in $(seq 1 50); do
        # Process input and get output
        output=$(get_output "$input_file")
        
        # Get stats
        stats=$(get_stats "$input_file")
        nodes=$(echo "$stats" | awk '{print $1}')
        edges=$(echo "$stats" | awk '{print $2}')
        
        # Calculate error
        error_rate=$(compute_error_rate "$output" "$expected")
        
        # Accumulate for average
        total_error=$(echo "scale=2; $total_error + $error_rate" | bc)
        iteration_count=$iteration
        
        # Calculate running average
        avg_error=$(echo "scale=2; $total_error / $iteration_count" | bc)
        
        # Format output for display
        output_display=$(printf "%-7s" "$output" | cut -c1-7)
        expected_display=$(printf "%-8s" "$expected" | cut -c1-8)
        
        # Print every 5th iteration and last
        if [ $((iteration % 5)) -eq 0 ] || [ $iteration -eq 50 ]; then
            printf "%9d | %-7s | %-8s | %6.2f%% | %5s | %5s | %10.2f%%\n" \
                "$iteration" "$output_display" "$expected_display" "$error_rate" \
                "$nodes" "$edges" "$avg_error" | tee -a "$RESULTS_FILE"
        fi
        
        # Clear output for next iteration
        if [ -f "$BRAIN_FILE" ]; then
            # The brain file persists between iterations, allowing learning
            true
        fi
    done
    
    echo "" | tee -a "$RESULTS_FILE"
    echo "Final Stats for '$input':" | tee -a "$RESULTS_FILE"
    echo "  Final Error Rate: ${error_rate}%" | tee -a "$RESULTS_FILE"
    echo "  Average Error Rate: ${avg_error}%" | tee -a "$RESULTS_FILE"
    echo "  Final Nodes: $nodes" | tee -a "$RESULTS_FILE"
    echo "  Final Edges: $edges" | tee -a "$RESULTS_FILE"
    echo "" | tee -a "$RESULTS_FILE"
done

echo "========================================" | tee -a "$RESULTS_FILE"
echo "Summary" | tee -a "$RESULTS_FILE"
echo "========================================" | tee -a "$RESULTS_FILE"
echo "" | tee -a "$RESULTS_FILE"

# Calculate overall statistics
echo "Overall Statistics:" | tee -a "$RESULTS_FILE"
stats=$(get_stats "$TEMP_DIR/input_hello.txt")
final_nodes=$(echo "$stats" | awk '{print $1}')
final_edges=$(echo "$stats" | awk '{print $2}')
echo "  Final Graph Size: $final_nodes nodes, $final_edges edges" | tee -a "$RESULTS_FILE"
echo "" | tee -a "$RESULTS_FILE"

echo "Test complete. Results saved to: $RESULTS_FILE" | tee -a "$RESULTS_FILE"

# Cleanup
rm -rf "$TEMP_DIR"

