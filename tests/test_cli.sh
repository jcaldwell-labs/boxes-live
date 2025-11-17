#!/bin/bash
# test_cli.sh - CLI-based integration tests for boxes-live
#
# These tests verify the boxes-cli tool works correctly and can be used
# for test-driven development workflows

set -e  # Exit on error

CLI="./connectors/boxes-cli"
TEST_CANVAS="test_cli_canvas.txt"
FAIL_COUNT=0
PASS_COUNT=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

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
    rm -f "$TEST_CANVAS" test_*.txt
}

# Cleanup before starting
cleanup

echo "=========================================="
echo "CLI Integration Tests"
echo "=========================================="

# Test 1: Create canvas
test_start "Create new canvas"
$CLI create "$TEST_CANVAS" --width 1000 --height 800
assert_success "Canvas creation succeeded"
assert_file_exists "$TEST_CANVAS"
assert_contains "$TEST_CANVAS" "BOXES_CANVAS_V1"
assert_contains "$TEST_CANVAS" "1000.0 800.0"

# Test 2: Add boxes
test_start "Add boxes to canvas"
$CLI add "$TEST_CANVAS" --x 100 --y 100 --title "Task 1" --content "Do this" "Do that" --color 1
assert_success "Add first box"
$CLI add "$TEST_CANVAS" --x 200 --y 100 --title "Task 2" --content "Another task" --color 2
assert_success "Add second box"

# Test 3: List boxes
test_start "List all boxes"
output=$($CLI list "$TEST_CANVAS")
assert_success "List command succeeded"
echo "$output" | grep -q "Task 1"
assert_success "Found Task 1 in list"
echo "$output" | grep -q "Task 2"
assert_success "Found Task 2 in list"

# Test 4: Search boxes
test_start "Search functionality"
output=$($CLI search "$TEST_CANVAS" "Task")
assert_success "Search command succeeded"
echo "$output" | grep -q "Found 2 box"
assert_success "Found 2 boxes matching 'Task'"

# Test 5: Get specific box
test_start "Get box by ID"
$CLI get "$TEST_CANVAS" 1 > /dev/null
assert_success "Get box #1"
$CLI get "$TEST_CANVAS" 999 2>/dev/null
assert_failure "Get non-existent box fails correctly"

# Test 6: Update box
test_start "Update box properties"
$CLI update "$TEST_CANVAS" 1 --title "Updated Task" --color 3
assert_success "Update box succeeded"
output=$($CLI get "$TEST_CANVAS" 1)
echo "$output" | grep -q "Updated Task"
assert_success "Title was updated"
echo "$output" | grep -q "Color: 3"
assert_success "Color was updated"

# Test 7: Filter by color
test_start "Filter boxes by color"
$CLI add "$TEST_CANVAS" --x 300 --y 100 --title "Task 3" --color 1
assert_success "Add box with color 1"
output=$($CLI list "$TEST_CANVAS" --color 1)
echo "$output" | grep -q "Task 3"
assert_success "Filter by color works"

# Test 8: JSON output
test_start "JSON output format"
output=$($CLI list "$TEST_CANVAS" --json)
assert_success "JSON output succeeded"
echo "$output" | python3 -m json.tool > /dev/null 2>&1
assert_success "Valid JSON produced"

# Test 9: Export to markdown
test_start "Export to Markdown"
$CLI export "$TEST_CANVAS" --format markdown -o test_export.md
assert_success "Export to markdown succeeded"
assert_file_exists "test_export.md"
assert_contains "test_export.md" "# Canvas Export"

# Test 10: Export to JSON
test_start "Export to JSON"
$CLI export "$TEST_CANVAS" --format json -o test_export.json
assert_success "Export to JSON succeeded"
python3 -m json.tool test_export.json > /dev/null 2>&1
assert_success "Valid JSON export"

# Test 11: Export to CSV
test_start "Export to CSV"
$CLI export "$TEST_CANVAS" --format csv -o test_export.csv
assert_success "Export to CSV succeeded"
assert_contains "test_export.csv" "id,x,y,width,height,color,title,content"

# Test 12: Statistics
test_start "Canvas statistics"
output=$($CLI stats "$TEST_CANVAS")
assert_success "Stats command succeeded"
echo "$output" | grep -q "Total boxes:"
assert_success "Stats contain box count"

# Test 13: Arrange boxes
test_start "Auto-arrange boxes"
$CLI arrange "$TEST_CANVAS" --layout grid --spacing 60
assert_success "Grid arrangement succeeded"

# Test 14: Delete box
test_start "Delete box"
$CLI delete "$TEST_CANVAS" 2
assert_success "Delete box succeeded"
$CLI get "$TEST_CANVAS" 2 2>/dev/null
assert_failure "Deleted box no longer exists"

# Test 15: Workflow test (create, populate, search, export)
test_start "Complete workflow test"
workflow_canvas="test_workflow.txt"
$CLI create "$workflow_canvas"
for i in {1..5}; do
    $CLI add "$workflow_canvas" --x $((i*100)) --y 100 \
         --title "Item $i" --content "Content $i" --color $((i % 7))
done
assert_success "Created 5-box workflow canvas"

search_result=$($CLI search "$workflow_canvas" "Item 3")
echo "$search_result" | grep -q "Item 3"
assert_success "Workflow search works"

$CLI export "$workflow_canvas" --format markdown > /dev/null
assert_success "Workflow export works"

# Cleanup
cleanup

# Summary
echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo -e "${GREEN}Passed: $PASS_COUNT${NC}"
if [ $FAIL_COUNT -gt 0 ]; then
    echo -e "${RED}Failed: $FAIL_COUNT${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
