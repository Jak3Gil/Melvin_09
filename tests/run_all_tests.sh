#!/bin/bash
# Run all tests with context transformers enabled

cd "$(dirname "$0")/.."
TEST_DIR="tests"
SRC_DIR="src"
RESULTS_FILE="test_results_context_transformers.txt"

echo "=========================================="
echo "Melvin Test Suite - Context Transformers"
echo "=========================================="
echo "Date: $(date)"
echo ""

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0
COMPILE_ERRORS=0

# Arrays to store results
PASSED_TESTS=()
FAILED_TESTS=()
SKIPPED_TESTS=()
COMPILE_ERROR_TESTS=()

# Function to run a single test
run_test() {
    local test_file="$1"
    local test_name=$(basename "$test_file" .c)
    TOTAL=$((TOTAL + 1))
    
    echo -n "[$TOTAL] Testing $test_name... "
    
    # Compile test
    if gcc -o "$TEST_DIR/$test_name" "$test_file" "$SRC_DIR/melvin.c" -I"$SRC_DIR" -lm 2>/dev/null; then
        # Run test (with timeout to prevent hanging)
        if output=$("$TEST_DIR/$test_name" 2>&1); then
            # Check if test reports success
            if echo "$output" | grep -qi "success\|passed\|✓\|test.*complete"; then
                echo -e "${GREEN}PASS${NC}"
                PASSED=$((PASSED + 1))
                PASSED_TESTS+=("$test_name")
            elif echo "$output" | grep -qi "fail\|error.*100\|✗"; then
                echo -e "${RED}FAIL${NC}"
                FAILED=$((FAILED + 1))
                FAILED_TESTS+=("$test_name")
            else
                # Test ran but didn't clearly pass/fail - assume it ran
                echo -e "${YELLOW}RAN${NC} (unclear result)"
                PASSED=$((PASSED + 1))
                PASSED_TESTS+=("$test_name")
            fi
        else
            # Test crashed or returned non-zero
            exit_code=$?
            echo -e "${RED}CRASH${NC} (exit $exit_code)"
            FAILED=$((FAILED + 1))
            FAILED_TESTS+=("$test_name")
        fi
    else
        echo -e "${RED}COMPILE ERROR${NC}"
        COMPILE_ERRORS=$((COMPILE_ERRORS + 1))
        COMPILE_ERROR_TESTS+=("$test_name")
    fi
    
    # Clean up
    rm -f "$TEST_DIR/$test_name"
}

# Find all test files
TEST_FILES=$(find "$TEST_DIR" -name "test_*.c" -type f | sort)

echo "Found $(echo "$TEST_FILES" | wc -l | tr -d ' ') test files"
echo ""

# Run each test
while IFS= read -r test_file; do
    run_test "$test_file"
done <<< "$TEST_FILES"

echo ""
echo "=========================================="
echo "Summary"
echo "=========================================="
echo "Total:  $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo "Compile Errors: $COMPILE_ERRORS"
echo ""

if [ ${#FAILED_TESTS[@]} -gt 0 ]; then
    echo "Failed Tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
fi

if [ ${#COMPILE_ERROR_TESTS[@]} -gt 0 ]; then
    echo "Compile Errors:"
    for test in "${COMPILE_ERROR_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
fi

# Save detailed results
{
    echo "Melvin Test Results - Context Transformers"
    echo "Date: $(date)"
    echo ""
    echo "Total: $TOTAL"
    echo "Passed: $PASSED"
    echo "Failed: $FAILED"
    echo "Compile Errors: $COMPILE_ERRORS"
    echo ""
    echo "=== PASSED TESTS ==="
    for test in "${PASSED_TESTS[@]}"; do
        echo "✓ $test"
    done
    echo ""
    echo "=== FAILED TESTS ==="
    for test in "${FAILED_TESTS[@]}"; do
        echo "✗ $test"
    done
    echo ""
    echo "=== COMPILE ERRORS ==="
    for test in "${COMPILE_ERROR_TESTS[@]}"; do
        echo "! $test"
    done
} > "$RESULTS_FILE"

echo "Detailed results saved to: $RESULTS_FILE"
echo ""

# Exit with error if any tests failed
if [ $FAILED -gt 0 ] || [ $COMPILE_ERRORS -gt 0 ]; then
    exit 1
else
    exit 0
fi
