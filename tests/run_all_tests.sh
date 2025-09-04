#!/bin/bash

# BASIC Interpreter Test Runner

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# The interpreter should be built by 'make' before running this script.
# The executable is expected to be in the src directory.
ALTAIR_BASIC_EXEC="src/altair_basic"

if [ ! -f "$ALTAIR_BASIC_EXEC" ]; then
    echo -e "${RED}❌ EXECUTABLE NOT FOUND at $ALTAIR_BASIC_EXEC${NC}"
    echo "Please run 'make' first."
    exit 1
fi

CASES_DIR="tests/cases"
EXPECTED_DIR="tests/expected"
TEMP_OUTPUT_DIR=$(mktemp -d)

# Ensure temp directory is cleaned up on exit
trap 'rm -rf -- "$TEMP_OUTPUT_DIR"' EXIT

passed_tests=0
failed_tests=0
total_tests=0

for test_file in "$CASES_DIR"/*.bas; do
    total_tests=$((total_tests + 1))
    base_name=$(basename "$test_file")
    expected_file="$EXPECTED_DIR/$base_name.expected"
    actual_output_file="$TEMP_OUTPUT_DIR/$base_name.actual"

    # Run the test
    (
        echo "NEW"
        cat "$test_file"
        echo "RUN"
    ) | $ALTAIR_BASIC_EXEC > "$actual_output_file" 2>&1

    # Compare the output, ignoring trailing whitespace
    if diff -ub --strip-trailing-cr "$expected_file" "$actual_output_file" > /dev/null; then
        echo -e "${GREEN}✅ PASS:${NC} $base_name"
        passed_tests=$((passed_tests + 1))
    else
        echo -e "${RED}❌ FAIL:${NC} $base_name"
        failed_tests=$((failed_tests + 1))
        echo "--------------------------------------------------"
        echo "Diff for $base_name:"
        diff -ub --strip-trailing-cr "$expected_file" "$actual_output_file"
        echo "--------------------------------------------------"
    fi
done

echo ""
echo "======================"
echo "    TEST SUMMARY"
echo "======================"
echo "Total tests: $total_tests"
echo -e "${GREEN}Passed: $passed_tests${NC}"
echo -e "${RED}Failed: $failed_tests${NC}"
echo "======================"
echo ""

if [ "$failed_tests" -ne 0 ]; then
    exit 1
fi
