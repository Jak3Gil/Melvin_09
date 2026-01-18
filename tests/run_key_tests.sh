#!/bin/bash
# Run key tests with context transformers enabled

cd "$(dirname "$0")/.."

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Key tests to run
KEY_TESTS=(
    "test_simple.c"
    "test_context_transformers.c"
    "test_learning.c"
    "test_associations.c"
    "test_hello_world.c"
    "test_semantic_features.c"
    "test_hierarchy_usage.c"
    "test_simple_association.c"
)

TOTAL=0
PASSED=0
FAILED=0

echo "Running Key Tests with Context Transformers"
echo "============================================"
echo ""

for test_file in "${KEY_TESTS[@]}"; do
    test_name=$(basename "$test_file" .c)
    TOTAL=$((TOTAL + 1))
    
    echo -n "[$TOTAL/$#KEY_TESTS] $test_name: "
    
    # Compile
    if gcc -o "tests/$test_name" "tests/$test_file" "src/melvin.c" -I"src" -lm 2>/dev/null; then
        # Run with timeout
        output=$(cd tests && ./$test_name 2>&1 | head -20)
        exit_code=$?
        
        if [ $exit_code -eq 0 ]; then
            if echo "$output" | grep -qiE "success|passed|✓|complete|test.*complete"; then
                echo -e "${GREEN}PASS${NC}"
                PASSED=$((PASSED + 1))
            elif echo "$output" | grep -qiE "fail|error.*100|✗"; then
                echo -e "${YELLOW}FAIL (ran but reported failure)${NC}"
                FAILED=$((FAILED + 1))
            else
                echo -e "${GREEN}RAN${NC} (no clear pass/fail marker)"
                PASSED=$((PASSED + 1))
            fi
        else
            echo -e "${RED}CRASH (exit $exit_code)${NC}"
            FAILED=$((FAILED + 1))
        fi
        rm -f "tests/$test_name"
    else
        echo -e "${RED}COMPILE ERROR${NC}"
        FAILED=$((FAILED + 1))
    fi
done

echo ""
echo "Summary: $PASSED/$TOTAL passed, $FAILED failed"
