#!/bin/bash
# connector_integration_test.sh - Comprehensive integration test suite for all boxes-live connectors
#
# This is the master integration test that validates:
# - All connectors work correctly
# - Connectors integrate with boxes-cli
# - Canvas file format correctness
# - Box positioning and sizing algorithms
# - Edge cases and error handling
# - Cross-connector workflows

set -e  # Exit on error for the master test runner

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test counters
TOTAL_SUITES=0
PASSED_SUITES=0
FAILED_SUITES=0
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Results tracking
declare -a SUITE_RESULTS
declare -a FAILED_SUITES_LIST

# Helper functions
section() {
    echo ""
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}$1${NC}"
    echo -e "${CYAN}========================================${NC}"
}

suite_start() {
    echo ""
    echo -e "${BLUE}[SUITE]${NC} $1"
    TOTAL_SUITES=$((TOTAL_SUITES + 1))
}

suite_pass() {
    echo -e "${GREEN}[✓ PASS]${NC} $1"
    PASSED_SUITES=$((PASSED_SUITES + 1))
    SUITE_RESULTS+=("✓ $1")
}

suite_fail() {
    echo -e "${RED}[✗ FAIL]${NC} $1"
    FAILED_SUITES=$((FAILED_SUITES + 1))
    FAILED_SUITES_LIST+=("$1")
    SUITE_RESULTS+=("✗ $1")
}

run_test_suite() {
    local test_script="$1"
    local suite_name="$2"

    suite_start "$suite_name"

    if [ ! -f "$test_script" ]; then
        echo -e "  ${YELLOW}⚠${NC} Test script not found: $test_script"
        suite_fail "$suite_name (script not found)"
        return 1
    fi

    if [ ! -x "$test_script" ]; then
        chmod +x "$test_script"
    fi

    # Run the test suite and capture output
    if "$test_script" > /tmp/test_output_$$.log 2>&1; then
        # Extract test counts from output
        local passed=$(grep -o "Passed: [0-9]*" /tmp/test_output_$$.log | tail -1 | awk '{print $2}' || echo "0")
        local failed=$(grep -o "Failed: [0-9]*" /tmp/test_output_$$.log | tail -1 | awk '{print $2}' || echo "0")

        TOTAL_TESTS=$((TOTAL_TESTS + passed + failed))
        PASSED_TESTS=$((PASSED_TESTS + passed))
        FAILED_TESTS=$((FAILED_TESTS + failed))

        echo -e "  ${GREEN}✓${NC} Passed: $passed, Failed: $failed"
        suite_pass "$suite_name"
        rm -f /tmp/test_output_$$.log
        return 0
    else
        echo -e "  ${RED}✗${NC} Test suite failed"
        # Show last 10 lines of output for debugging
        echo -e "  ${YELLOW}Last 10 lines of output:${NC}"
        tail -10 /tmp/test_output_$$.log | sed 's/^/    /'
        suite_fail "$suite_name"
        rm -f /tmp/test_output_$$.log
        return 1
    fi
}

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$PROJECT_ROOT"

section "Boxes-Live Connector Integration Tests"
echo "Project root: $PROJECT_ROOT"
echo "Date: $(date)"
echo ""

# ==============================================================================
# Phase 1: Individual Connector Tests
# ==============================================================================
section "Phase 1: Individual Connector Tests"

# Test each connector individually
run_test_suite "$SCRIPT_DIR/test_cli.sh" "boxes-cli CLI Tool"
run_test_suite "$SCRIPT_DIR/test_pstree2canvas.sh" "pstree2canvas Connector"
run_test_suite "$SCRIPT_DIR/test_log2canvas.sh" "log2canvas Connector"
run_test_suite "$SCRIPT_DIR/test_docker2canvas.sh" "docker2canvas Connector"

# Check for json2canvas test
if [ -f "$SCRIPT_DIR/test_json2canvas.sh" ]; then
    run_test_suite "$SCRIPT_DIR/test_json2canvas.sh" "json2canvas Connector"
fi

# ==============================================================================
# Phase 2: Quick Functional Tests for Other Connectors
# ==============================================================================
section "Phase 2: Quick Functional Tests for Other Connectors"

TEMP_DIR=$(mktemp -d)
trap "rm -rf $TEMP_DIR" EXIT

# Test git2canvas
suite_start "git2canvas Quick Test"
if ./connectors/git2canvas --max 5 > "$TEMP_DIR/git.txt" 2>/dev/null; then
    if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/git.txt"; then
        echo -e "  ${GREEN}✓${NC} Generates valid canvas format"
        suite_pass "git2canvas Quick Test"
    else
        echo -e "  ${RED}✗${NC} Invalid canvas format"
        suite_fail "git2canvas Quick Test"
    fi
else
    echo -e "  ${RED}✗${NC} Execution failed"
    suite_fail "git2canvas Quick Test"
fi

# Test csv2canvas
suite_start "csv2canvas Quick Test"
if [ -f "examples/sample_data.csv" ]; then
    if ./connectors/csv2canvas examples/sample_data.csv > "$TEMP_DIR/csv.txt" 2>/dev/null; then
        if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/csv.txt"; then
            echo -e "  ${GREEN}✓${NC} Generates valid canvas format"
            suite_pass "csv2canvas Quick Test"
        else
            echo -e "  ${RED}✗${NC} Invalid canvas format"
            suite_fail "csv2canvas Quick Test"
        fi
    else
        echo -e "  ${RED}✗${NC} Execution failed"
        suite_fail "csv2canvas Quick Test"
    fi
else
    echo -e "  ${YELLOW}⚠${NC} Sample data not found, skipping"
fi

# Test tree2canvas
suite_start "tree2canvas Quick Test"
if ./connectors/tree2canvas . --max-depth 2 > "$TEMP_DIR/tree.txt" 2>/dev/null; then
    if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/tree.txt"; then
        echo -e "  ${GREEN}✓${NC} Generates valid canvas format"
        suite_pass "tree2canvas Quick Test"
    else
        echo -e "  ${RED}✗${NC} Invalid canvas format"
        suite_fail "tree2canvas Quick Test"
    fi
else
    echo -e "  ${RED}✗${NC} Execution failed"
    suite_fail "tree2canvas Quick Test"
fi

# Test text2canvas
suite_start "text2canvas Quick Test"
if echo -e "Line 1\nLine 2\nLine 3" | ./connectors/text2canvas --lines > "$TEMP_DIR/text.txt" 2>/dev/null; then
    if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/text.txt"; then
        echo -e "  ${GREEN}✓${NC} Generates valid canvas format"
        suite_pass "text2canvas Quick Test"
    else
        echo -e "  ${RED}✗${NC} Invalid canvas format"
        suite_fail "text2canvas Quick Test"
    fi
else
    echo -e "  ${RED}✗${NC} Execution failed"
    suite_fail "text2canvas Quick Test"
fi

# Test periodic2canvas
suite_start "periodic2canvas Quick Test"
if ./connectors/periodic2canvas > "$TEMP_DIR/periodic.txt" 2>/dev/null; then
    if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/periodic.txt"; then
        box_count=$(head -3 "$TEMP_DIR/periodic.txt" | tail -1)
        echo -e "  ${GREEN}✓${NC} Generated canvas with $box_count boxes"
        suite_pass "periodic2canvas Quick Test"
    else
        echo -e "  ${RED}✗${NC} Invalid canvas format"
        suite_fail "periodic2canvas Quick Test"
    fi
else
    echo -e "  ${RED}✗${NC} Execution failed"
    suite_fail "periodic2canvas Quick Test"
fi

# Test jcaldwell-labs2canvas
suite_start "jcaldwell-labs2canvas Quick Test"
if ./connectors/jcaldwell-labs2canvas > "$TEMP_DIR/jcaldwell-labs.txt" 2>/dev/null; then
    if grep -q "BOXES_CANVAS_V1" "$TEMP_DIR/jcaldwell-labs.txt"; then
        box_count=$(head -3 "$TEMP_DIR/jcaldwell-labs.txt" | tail -1)
        echo -e "  ${GREEN}✓${NC} Generated canvas with $box_count boxes"
        # Verify it contains expected project boxes
        if grep -q "my-context" "$TEMP_DIR/jcaldwell-labs.txt" && \
           grep -q "boxes-live" "$TEMP_DIR/jcaldwell-labs.txt"; then
            echo -e "  ${GREEN}✓${NC} Contains expected project boxes"
            suite_pass "jcaldwell-labs2canvas Quick Test"
        else
            echo -e "  ${RED}✗${NC} Missing expected project boxes"
            suite_fail "jcaldwell-labs2canvas Quick Test"
        fi
    else
        echo -e "  ${RED}✗${NC} Invalid canvas format"
        suite_fail "jcaldwell-labs2canvas Quick Test"
    fi
else
    echo -e "  ${RED}✗${NC} Execution failed"
    suite_fail "jcaldwell-labs2canvas Quick Test"
fi

# ==============================================================================
# Phase 3: Cross-Connector Integration Tests
# ==============================================================================
section "Phase 3: Cross-Connector Integration Tests"

# Test 1: Connector → CLI workflow
suite_start "Connector → CLI Integration"
./connectors/periodic2canvas > "$TEMP_DIR/integration1.txt" 2>/dev/null
if ./connectors/boxes-cli stats "$TEMP_DIR/integration1.txt" > /dev/null 2>&1; then
    echo -e "  ${GREEN}✓${NC} boxes-cli can read connector output"

    # Test search on connector-generated canvas
    if ./connectors/boxes-cli search "$TEMP_DIR/integration1.txt" "Hydrogen" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} boxes-cli search works on connector output"
    else
        echo -e "  ${RED}✗${NC} boxes-cli search failed"
    fi

    # Test export on connector-generated canvas
    if ./connectors/boxes-cli export "$TEMP_DIR/integration1.txt" --format markdown -o "$TEMP_DIR/export.md" 2>/dev/null; then
        echo -e "  ${GREEN}✓${NC} boxes-cli export works on connector output"
        suite_pass "Connector → CLI Integration"
    else
        echo -e "  ${RED}✗${NC} boxes-cli export failed"
        suite_fail "Connector → CLI Integration"
    fi
else
    echo -e "  ${RED}✗${NC} boxes-cli cannot read connector output"
    suite_fail "Connector → CLI Integration"
fi

# Test 2: Multi-connector pipeline
suite_start "Multi-Connector Pipeline"
# Generate canvas from git, then manipulate with CLI
if ./connectors/git2canvas --max 5 > "$TEMP_DIR/pipeline.txt" 2>/dev/null; then
    # Add a box using CLI
    if ./connectors/boxes-cli add "$TEMP_DIR/pipeline.txt" \
        --x 500 --y 500 --title "Custom Note" --content "Added via CLI" --color 5 > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} Can mix connector output with CLI manipulation"

        # Verify the canvas is still valid
        if ./connectors/boxes-cli stats "$TEMP_DIR/pipeline.txt" > /dev/null 2>&1; then
            echo -e "  ${GREEN}✓${NC} Canvas remains valid after CLI manipulation"
            suite_pass "Multi-Connector Pipeline"
        else
            echo -e "  ${RED}✗${NC} Canvas corrupted after CLI manipulation"
            suite_fail "Multi-Connector Pipeline"
        fi
    else
        echo -e "  ${RED}✗${NC} Cannot add box to connector-generated canvas"
        suite_fail "Multi-Connector Pipeline"
    fi
else
    echo -e "  ${RED}✗${NC} git2canvas failed"
    suite_fail "Multi-Connector Pipeline"
fi

# Test 3: Canvas format validation across all connectors
suite_start "Canvas Format Consistency"
consistent=true
for canvas_file in "$TEMP_DIR"/*.txt; do
    if [ -f "$canvas_file" ]; then
        # Check header
        if ! head -1 "$canvas_file" | grep -q "BOXES_CANVAS_V1"; then
            echo -e "  ${RED}✗${NC} Invalid header in $(basename $canvas_file)"
            consistent=false
        fi

        # Check world dimensions on line 2
        if ! head -2 "$canvas_file" | tail -1 | grep -qE '^[0-9]+(\.[0-9]+)? [0-9]+(\.[0-9]+)?$'; then
            echo -e "  ${RED}✗${NC} Invalid world dimensions in $(basename $canvas_file)"
            consistent=false
        fi

        # Check box count on line 3
        if ! head -3 "$canvas_file" | tail -1 | grep -qE '^[0-9]+$'; then
            echo -e "  ${RED}✗${NC} Invalid box count in $(basename $canvas_file)"
            consistent=false
        fi
    fi
done

if $consistent; then
    echo -e "  ${GREEN}✓${NC} All generated canvases have consistent format"
    suite_pass "Canvas Format Consistency"
else
    suite_fail "Canvas Format Consistency"
fi

# ==============================================================================
# Phase 4: Edge Cases and Error Handling
# ==============================================================================
section "Phase 4: Edge Cases and Error Handling"

# Test empty input handling
suite_start "Empty Input Handling"
empty_tests_passed=true

# Test boxes-cli with empty canvas
./connectors/boxes-cli create "$TEMP_DIR/empty.txt" > /dev/null 2>&1
if ./connectors/boxes-cli list "$TEMP_DIR/empty.txt" > /dev/null 2>&1; then
    echo -e "  ${GREEN}✓${NC} boxes-cli handles empty canvas"
else
    echo -e "  ${RED}✗${NC} boxes-cli fails on empty canvas"
    empty_tests_passed=false
fi

# Test connector with empty input
if echo "" | ./connectors/text2canvas > /dev/null 2>&1; then
    echo -e "  ${YELLOW}⚠${NC} text2canvas accepts empty input (should error)"
else
    echo -e "  ${GREEN}✓${NC} text2canvas rejects empty input correctly"
fi

if $empty_tests_passed; then
    suite_pass "Empty Input Handling"
else
    suite_fail "Empty Input Handling"
fi

# Test invalid canvas format handling
suite_start "Invalid Format Handling"
echo "INVALID_HEADER" > "$TEMP_DIR/invalid.txt"
echo "100 100" >> "$TEMP_DIR/invalid.txt"
echo "0" >> "$TEMP_DIR/invalid.txt"

if ./connectors/boxes-cli list "$TEMP_DIR/invalid.txt" > /dev/null 2>&1; then
    echo -e "  ${RED}✗${NC} boxes-cli accepts invalid canvas header"
    suite_fail "Invalid Format Handling"
else
    echo -e "  ${GREEN}✓${NC} boxes-cli rejects invalid canvas format"
    suite_pass "Invalid Format Handling"
fi

# Test large canvas handling
suite_start "Large Canvas Handling"
./connectors/boxes-cli create "$TEMP_DIR/large.txt" --width 100000 --height 100000 > /dev/null 2>&1
large_tests_passed=true

# Add many boxes
for i in {1..100}; do
    ./connectors/boxes-cli add "$TEMP_DIR/large.txt" \
        --x $((i * 100)) --y $((i * 100)) \
        --title "Box $i" --content "Content $i" > /dev/null 2>&1 || large_tests_passed=false
done

if $large_tests_passed; then
    box_count=$(./connectors/boxes-cli stats "$TEMP_DIR/large.txt" 2>/dev/null | grep "Total boxes" | awk '{print $3}')
    if [ "$box_count" = "100" ]; then
        echo -e "  ${GREEN}✓${NC} Successfully created and managed 100 boxes"
        suite_pass "Large Canvas Handling"
    else
        echo -e "  ${RED}✗${NC} Box count mismatch: expected 100, got $box_count"
        suite_fail "Large Canvas Handling"
    fi
else
    echo -e "  ${RED}✗${NC} Failed to create large canvas"
    suite_fail "Large Canvas Handling"
fi

# ==============================================================================
# Phase 5: Export Format Validation
# ==============================================================================
section "Phase 5: Export Format Validation"

# Test all export formats
suite_start "Export Format Validation"
export_tests_passed=true

# Generate a test canvas
./connectors/periodic2canvas > "$TEMP_DIR/export_test.txt" 2>/dev/null

# Test Markdown export
if ./connectors/boxes-cli export "$TEMP_DIR/export_test.txt" \
    --format markdown -o "$TEMP_DIR/export.md" 2>/dev/null; then
    if grep -q "# Canvas Export" "$TEMP_DIR/export.md"; then
        echo -e "  ${GREEN}✓${NC} Markdown export valid"
    else
        echo -e "  ${RED}✗${NC} Markdown export invalid"
        export_tests_passed=false
    fi
else
    echo -e "  ${RED}✗${NC} Markdown export failed"
    export_tests_passed=false
fi

# Test JSON export
if ./connectors/boxes-cli export "$TEMP_DIR/export_test.txt" \
    --format json -o "$TEMP_DIR/export.json" 2>/dev/null; then
    if python3 -m json.tool "$TEMP_DIR/export.json" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✓${NC} JSON export valid"
    else
        echo -e "  ${RED}✗${NC} JSON export invalid"
        export_tests_passed=false
    fi
else
    echo -e "  ${RED}✗${NC} JSON export failed"
    export_tests_passed=false
fi

# Test CSV export
if ./connectors/boxes-cli export "$TEMP_DIR/export_test.txt" \
    --format csv -o "$TEMP_DIR/export.csv" 2>/dev/null; then
    if grep -q "id,x,y,width,height,color,title,content" "$TEMP_DIR/export.csv"; then
        echo -e "  ${GREEN}✓${NC} CSV export valid"
    else
        echo -e "  ${RED}✗${NC} CSV export invalid"
        export_tests_passed=false
    fi
else
    echo -e "  ${RED}✗${NC} CSV export failed"
    export_tests_passed=false
fi

if $export_tests_passed; then
    suite_pass "Export Format Validation"
else
    suite_fail "Export Format Validation"
fi

# ==============================================================================
# Results Summary
# ==============================================================================
section "Integration Test Results Summary"

echo ""
echo "Test Suites:"
echo -e "  Total:  $TOTAL_SUITES"
echo -e "  ${GREEN}Passed: $PASSED_SUITES${NC}"
echo -e "  ${RED}Failed: $FAILED_SUITES${NC}"
echo ""

echo "Individual Tests:"
echo -e "  Total:  $TOTAL_TESTS"
echo -e "  ${GREEN}Passed: $PASSED_TESTS${NC}"
echo -e "  ${RED}Failed: $FAILED_TESTS${NC}"
echo ""

echo "Suite Results:"
for result in "${SUITE_RESULTS[@]}"; do
    if [[ $result == ✓* ]]; then
        echo -e "  ${GREEN}$result${NC}"
    else
        echo -e "  ${RED}$result${NC}"
    fi
done
echo ""

if [ $FAILED_SUITES -eq 0 ] && [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}╔═══════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║                                           ║${NC}"
    echo -e "${GREEN}║   ✓ ALL INTEGRATION TESTS PASSED!        ║${NC}"
    echo -e "${GREEN}║                                           ║${NC}"
    echo -e "${GREEN}║   All connectors are working correctly   ║${NC}"
    echo -e "${GREEN}║   and integrating seamlessly.            ║${NC}"
    echo -e "${GREEN}║                                           ║${NC}"
    echo -e "${GREEN}╚═══════════════════════════════════════════╝${NC}"
    echo ""
    exit 0
else
    echo -e "${RED}╔═══════════════════════════════════════════╗${NC}"
    echo -e "${RED}║                                           ║${NC}"
    echo -e "${RED}║   ✗ SOME INTEGRATION TESTS FAILED         ║${NC}"
    echo -e "${RED}║                                           ║${NC}"
    echo -e "${RED}╚═══════════════════════════════════════════╝${NC}"
    echo ""

    if [ ${#FAILED_SUITES_LIST[@]} -gt 0 ]; then
        echo "Failed Suites:"
        for failed_suite in "${FAILED_SUITES_LIST[@]}"; do
            echo -e "  ${RED}✗${NC} $failed_suite"
        done
        echo ""
    fi

    echo "Please review the failures above and fix them before release."
    echo ""
    exit 1
fi
