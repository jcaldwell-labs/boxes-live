#!/bin/bash
# test_pstree2canvas.sh - Test suite for pstree2canvas connector
#
# Tests process tree parsing and canvas generation

# set -e  # Disabled for full test run

CONNECTOR="./connectors/pstree2canvas"
CLI="./connectors/boxes-cli"
FAIL_COUNT=0
PASS_COUNT=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Test helper functions
test_start() {
    echo -e "\n${YELLOW}[TEST]${NC} $1"
}

assert_success() {
    if [ $? -eq 0 ]; then
        echo -e "  ${GREEN}✓${NC} $1"
        ((PASS_COUNT++))
    else
        echo -e "  ${RED}✗${NC} $1"
        ((FAIL_COUNT++))
    fi
}

assert_failure() {
    if [ $? -ne 0 ]; then
        echo -e "  ${GREEN}✓${NC} $1"
        ((PASS_COUNT++))
    else
        echo -e "  ${RED}✗${NC} $1"
        ((FAIL_COUNT++))
    fi
}

assert_file_exists() {
    if [ -f "$1" ]; then
        echo -e "  ${GREEN}✓${NC} File exists: $1"
        ((PASS_COUNT++))
    else
        echo -e "  ${RED}✗${NC} File exists: $1"
        ((FAIL_COUNT++))
    fi
}

assert_contains() {
    if grep -q "$2" "$1"; then
        echo -e "  ${GREEN}✓${NC} File contains: $2"
        ((PASS_COUNT++))
    else
        echo -e "  ${RED}✗${NC} File contains: $2"
        ((FAIL_COUNT++))
    fi
}

cleanup() {
    rm -f test_pstree*.txt test_sample_ps*.txt
}

# Cleanup before starting
cleanup

echo "=========================================="
echo "pstree2canvas Connector Tests"
echo "=========================================="

# Test 1: Simple pstree format
test_start "Create sample pstree output"
cat > test_sample_pstree.txt <<'EOF'
systemd(1)
├─sshd(1234)
│ └─sshd(2345)
│   └─bash(3456)
│     └─vim(4567)
├─nginx(5678)
│ ├─nginx(5679)
│ └─nginx(5680)
└─postgres(6789)
  ├─postgres(6790)
  └─postgres(6791)
EOF
assert_file_exists "test_sample_pstree.txt"

# Test 2: Parse pstree format
test_start "Parse pstree format"
$CONNECTOR test_sample_pstree.txt > test_pstree_output.txt
assert_success "Connector execution"
assert_file_exists "test_pstree_output.txt"
assert_contains "test_pstree_output.txt" "BOXES_CANVAS_V1"

# Test 3: Verify box count
test_start "Verify correct number of boxes created"
box_count=$(head -3 test_pstree_output.txt | tail -1)
if [ "$box_count" -eq "11" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 11"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 11 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 4: Check process names
test_start "Verify process names are extracted"
assert_contains "test_pstree_output.txt" "systemd"
assert_contains "test_pstree_output.txt" "bash"

# Test 5: PS format
test_start "Create sample ps output"
cat > test_sample_ps.txt <<'EOF'
PID  PPID COMM
1    0    systemd
1234 1    sshd
2345 1234 bash
3456 2345 vim
5678 1    nginx
5679 5678 nginx
EOF
assert_file_exists "test_sample_ps.txt"

# Test 6: Parse ps format
test_start "Parse ps format"
$CONNECTOR test_sample_ps.txt --format ps > test_pstree_ps.txt
assert_success "PS format parsing"
assert_contains "test_pstree_ps.txt" "BOXES_CANVAS_V1"

# Test 7: Verify ps box count
test_start "Verify ps format box count"
box_count=$(head -3 test_pstree_ps.txt | tail -1)
if [ "$box_count" -eq "6" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 6"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 6 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 8: Color coding for systemd (root)
test_start "Verify systemd is blue (color 4)"
grep -B 1 "systemd" test_pstree_output.txt | head -1 | grep -q " 4$"
assert_success "Systemd has blue color"

# Test 9: Color coding for shells
test_start "Verify bash is yellow (color 3)"
grep -B 1 "bash" test_pstree_output.txt | head -1 | grep -q " 3$"
assert_success "Bash has yellow color"

# Test 10: Tree layout (default)
test_start "Test tree layout (default)"
$CONNECTOR test_sample_pstree.txt --layout tree > test_pstree_tree.txt
assert_success "Tree layout"
assert_contains "test_pstree_tree.txt" "BOXES_CANVAS_V1"

# Test 11: Grid layout
test_start "Test grid layout"
$CONNECTOR test_sample_pstree.txt --layout grid > test_pstree_grid.txt
assert_success "Grid layout"
assert_contains "test_pstree_grid.txt" "BOXES_CANVAS_V1"

# Test 12: Max depth limit
test_start "Test --max-depth option"
$CONNECTOR test_sample_pstree.txt --max-depth 2 > test_pstree_maxdepth.txt
box_count=$(head -3 test_pstree_maxdepth.txt | tail -1)
# Should filter out deeper processes (depth 3+)
# Expect: 1 at depth 0, 3 at depth 1, 5 at depth 2 = 9 total
if [ "$box_count" -eq "9" ]; then
    echo -e "  ${GREEN}✓${NC} Max depth limit applied correctly (got $box_count boxes)"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 9 boxes with max-depth 2, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 13: Auto-detect pstree format
test_start "Test auto-detect of pstree format"
$CONNECTOR test_sample_pstree.txt --format auto > test_pstree_auto.txt
assert_success "Auto-detect pstree format"
assert_contains "test_pstree_auto.txt" "systemd"

# Test 14: Auto-detect ps format
test_start "Test auto-detect of ps format"
$CONNECTOR test_sample_ps.txt --format auto > test_pstree_auto_ps.txt
assert_success "Auto-detect ps format"
box_count=$(head -3 test_pstree_auto_ps.txt | tail -1)
if [ "$box_count" -eq "6" ]; then
    echo -e "  ${GREEN}✓${NC} Auto-detected ps correctly"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Auto-detect failed"
    ((FAIL_COUNT++))
fi

# Test 15: Integration with boxes-cli stats
test_start "Integration with boxes-cli stats"
$CLI stats test_pstree_output.txt > /dev/null
assert_success "boxes-cli stats works with pstree2canvas output"

# Test 16: Integration with boxes-cli list
test_start "Integration with boxes-cli list"
$CLI list test_pstree_output.txt > /dev/null
assert_success "boxes-cli list works with pstree2canvas output"

# Test 17: Integration with boxes-cli search
test_start "Integration with boxes-cli search"
output=$($CLI search test_pstree_output.txt "nginx")
echo "$output" | grep -q "nginx"
assert_success "boxes-cli search finds nginx"

# Test 18: Empty input handling
test_start "Handle empty input gracefully"
echo "" | $CONNECTOR > test_pstree_empty.txt 2>&1
assert_failure "Empty input returns error"

# Test 19: Stdin input
test_start "Parse pstree from stdin"
cat test_sample_pstree.txt | $CONNECTOR > test_pstree_stdin.txt
assert_success "Stdin input works"
assert_contains "test_pstree_stdin.txt" "BOXES_CANVAS_V1"

# Test 20: Single process
test_start "Handle single process"
echo "init(1)" | $CONNECTOR > test_pstree_single.txt
assert_success "Single process parsing"
box_count=$(head -3 test_pstree_single.txt | tail -1)
if [ "$box_count" -eq "1" ]; then
    echo -e "  ${GREEN}✓${NC} Single process processed"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 1 box, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 21: Daemon color coding
test_start "Verify daemon processes are cyan (color 6)"
# sshd should be colored as daemon (ends with 'd')
grep -B 1 "sshd" test_pstree_output.txt | head -1 | grep -q " 6$"
assert_success "Daemon process has cyan color"

# Cleanup
cleanup

# Summary
echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "${GREEN}Passed:${NC} $PASS_COUNT"
echo -e "${RED}Failed:${NC} $FAIL_COUNT"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "\n${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "\n${RED}Some tests failed.${NC}"
    exit 1
fi
