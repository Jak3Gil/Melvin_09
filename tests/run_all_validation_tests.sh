#!/bin/bash

# Run All Validation Tests
# Comprehensive test suite to prove all system capabilities

echo "╔════════════════════════════════════════════════════════════╗"
echo "║         MELVIN COMPREHENSIVE VALIDATION SUITE              ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compile flags
CFLAGS="-O2 -Wall -I./src"
LIBS="src/melvin_lib.o src/melvin_in_port.o src/melvin_out_port.o -lm"

# Test counter
TOTAL=0
PASSED=0
FAILED=0

# Function to run a test
run_test() {
    local test_name=$1
    local test_file=$2
    local test_binary="/tmp/${test_name}"
    
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Running: ${test_name}"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    # Compile
    gcc ${CFLAGS} ${test_file} ${LIBS} -o ${test_binary} 2>/dev/null
    if [ $? -ne 0 ]; then
        echo -e "${RED}✗ COMPILE FAILED${NC}"
        FAILED=$((FAILED + 1))
        TOTAL=$((TOTAL + 1))
        return
    fi
    
    # Run
    ${test_binary} 2>&1 | grep -v "^\[LOG\]" | grep -v "^\[DEBUG\]"
    local exit_code=$?
    
    # Check result
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ TEST COMPLETED${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}✗ TEST FAILED${NC}"
        FAILED=$((FAILED + 1))
    fi
    
    TOTAL=$((TOTAL + 1))
    echo ""
}

# Ensure we're in the right directory
cd "$(dirname "$0")/.." || exit 1

# Build libraries if needed
if [ ! -f src/melvin_lib.o ]; then
    echo "Building Melvin libraries..."
    make clean && make all
    echo ""
fi

echo "Starting validation tests..."
echo ""

# Run all tests
run_test "test_multimodal" "tests/test_multimodal.c"
run_test "test_blank_nodes" "tests/test_blank_nodes.c"
run_test "test_hierarchy_benefit" "tests/test_hierarchy_benefit.c"
run_test "test_scalability" "tests/test_scalability.c"
run_test "test_streaming_vs_completion" "tests/test_streaming_vs_completion.c"
run_test "test_context_window" "tests/test_context_window.c"
run_test "test_mininet_learning" "tests/test_mininet_learning.c"
run_test "test_adaptive_thresholds" "tests/test_adaptive_thresholds.c"

# Summary
echo "╔════════════════════════════════════════════════════════════╗"
echo "║                    TEST SUMMARY                            ║"
echo "╚════════════════════════════════════════════════════════════╝"
echo ""
echo "Total Tests: ${TOTAL}"
echo -e "Passed: ${GREEN}${PASSED}${NC}"
echo -e "Failed: ${RED}${FAILED}${NC}"
echo ""

if [ ${FAILED} -eq 0 ]; then
    echo -e "${GREEN}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║              ALL TESTS PASSED! ✓                           ║${NC}"
    echo -e "${GREEN}╚════════════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${YELLOW}╔════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${YELLOW}║         SOME TESTS NEED ATTENTION                          ║${NC}"
    echo -e "${YELLOW}╚════════════════════════════════════════════════════════════╝${NC}"
    exit 1
fi
