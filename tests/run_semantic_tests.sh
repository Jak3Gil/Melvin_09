#!/bin/bash
# Run semantic features test suite

set -e

echo "Building semantic features test..."
cd "$(dirname "$0")/.."

# Compile test
gcc -o tests/test_semantic_features \
    tests/test_semantic_features.c \
    src/melvin.c \
    -I. -lm -Wall -Wextra -O2

echo "Running tests..."
./tests/test_semantic_features

echo ""
echo "Cleaning up test files..."
rm -f test_semantic.m
rm -f tests/test_semantic_features

echo "Done!"
