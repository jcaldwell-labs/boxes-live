#!/bin/bash
# test_json2canvas.sh - Test suite for json2canvas connector
#
# Tests JSON parsing and canvas generation

# set -e  # Disabled for full test run

CONNECTOR="./connectors/json2canvas"
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
    rm -f test_json*.txt test_sample*.json
}

# Cleanup before starting
cleanup

echo "=========================================="
echo "json2canvas Connector Tests"
echo "=========================================="

# Test 1: Simple array of objects
test_start "Create sample JSON array"
cat > test_sample_array.json <<'EOF'
[
  {"name": "Alice", "age": 30, "city": "New York"},
  {"name": "Bob", "age": 25, "city": "Los Angeles"},
  {"name": "Charlie", "age": 35, "city": "Chicago"}
]
EOF
assert_file_exists "test_sample_array.json"

# Test 2: Parse simple JSON array
test_start "Parse simple JSON array"
$CONNECTOR test_sample_array.json > test_json_array.txt
assert_success "Connector execution"
assert_file_exists "test_json_array.txt"
assert_contains "test_json_array.txt" "BOXES_CANVAS_V1"

# Test 3: Verify box count
test_start "Verify correct number of boxes created"
box_count=$(head -3 test_json_array.txt | tail -1)
if [ "$box_count" -eq "3" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 3"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 3 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 4: Check auto-detected name field
test_start "Verify auto-detection of 'name' field as title"
assert_contains "test_json_array.txt" "Alice"
assert_contains "test_json_array.txt" "Bob"

# Test 5: Single object input
test_start "Create sample single JSON object"
cat > test_sample_object.json <<'EOF'
{
  "id": 123,
  "title": "Test Item",
  "description": "This is a test",
  "status": "active"
}
EOF
assert_file_exists "test_sample_object.json"

# Test 6: Parse single object
test_start "Parse single JSON object"
$CONNECTOR test_sample_object.json > test_json_object.txt
assert_success "Single object parsing"
assert_contains "test_json_object.txt" "BOXES_CANVAS_V1"

# Test 7: Verify single object creates one box
test_start "Verify single object creates one box"
box_count=$(head -3 test_json_object.txt | tail -1)
if [ "$box_count" -eq "1" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 1"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 1 box, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 8: Check title field detection
test_start "Verify 'title' field auto-detection"
assert_contains "test_json_object.txt" "Test Item"

# Test 9: Nested JSON objects
test_start "Create sample with nested objects"
cat > test_sample_nested.json <<'EOF'
[
  {
    "name": "Project A",
    "metadata": {"created": "2024-01-01", "owner": "Alice"},
    "tags": ["important", "urgent"]
  },
  {
    "name": "Project B",
    "metadata": {"created": "2024-01-02", "owner": "Bob"},
    "tags": ["review"]
  }
]
EOF
assert_file_exists "test_sample_nested.json"

# Test 10: Parse nested JSON
test_start "Parse nested JSON structures"
$CONNECTOR test_sample_nested.json > test_json_nested.txt
assert_success "Nested JSON parsing"
assert_contains "test_json_nested.txt" "Project A"

# Test 11: JSON with wrapper object
test_start "Create JSON with data wrapper"
cat > test_sample_wrapper.json <<'EOF'
{
  "data": [
    {"id": 1, "name": "Item 1"},
    {"id": 2, "name": "Item 2"}
  ],
  "total": 2,
  "status": "success"
}
EOF
assert_file_exists "test_sample_wrapper.json"

# Test 12: Parse wrapped JSON
test_start "Parse JSON with data wrapper"
$CONNECTOR test_sample_wrapper.json > test_json_wrapper.txt
assert_success "Wrapper JSON parsing"
box_count=$(head -3 test_json_wrapper.txt | tail -1)
if [ "$box_count" -eq "2" ]; then
    echo -e "  ${GREEN}✓${NC} Extracted items from wrapper: 2"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 2 items from wrapper, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 13: Custom key field
test_start "Test custom key field option"
cat > test_sample_custom.json <<'EOF'
[
  {"username": "alice123", "email": "alice@example.com"},
  {"username": "bob456", "email": "bob@example.com"}
]
EOF
$CONNECTOR test_sample_custom.json --key-field username > test_json_custom.txt
assert_success "Custom key field"
assert_contains "test_json_custom.txt" "alice123"

# Test 14: Color field mapping
test_start "Test color field mapping"
cat > test_sample_color.json <<'EOF'
[
  {"name": "Task 1", "status": "completed"},
  {"name": "Task 2", "status": "error"},
  {"name": "Task 3", "status": "pending"}
]
EOF
$CONNECTOR test_sample_color.json --color-field status > test_json_color.txt
assert_success "Color field mapping"
# completed should be green (2), error should be red (1), pending should be yellow (3)
grep -B 1 "Task 1" test_json_color.txt | grep -q " 2$"
assert_success "Completed task is green"

# Test 15: Grid layout (default)
test_start "Test grid layout (default)"
$CONNECTOR test_sample_array.json --layout grid > test_json_grid.txt
assert_success "Grid layout"
assert_contains "test_json_grid.txt" "BOXES_CANVAS_V1"

# Test 16: Flow layout
test_start "Test flow layout"
$CONNECTOR test_sample_array.json --layout flow > test_json_flow.txt
assert_success "Flow layout"
assert_contains "test_json_flow.txt" "BOXES_CANVAS_V1"

# Test 17: List layout
test_start "Test list layout"
$CONNECTOR test_sample_array.json --layout list > test_json_list.txt
assert_success "List layout"
assert_contains "test_json_list.txt" "BOXES_CANVAS_V1"

# Test 18: Max items limit
test_start "Test --max limit option"
cat > test_sample_many.json <<'EOF'
[
  {"name": "Item 1"}, {"name": "Item 2"}, {"name": "Item 3"},
  {"name": "Item 4"}, {"name": "Item 5"}, {"name": "Item 6"},
  {"name": "Item 7"}, {"name": "Item 8"}, {"name": "Item 9"}
]
EOF
$CONNECTOR test_sample_many.json --max 5 > test_json_max.txt
box_count=$(head -3 test_json_max.txt | tail -1)
if [ "$box_count" -eq "5" ]; then
    echo -e "  ${GREEN}✓${NC} Max limit respected: 5"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 5 boxes with --max 5, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 19: Integration with boxes-cli stats
test_start "Integration with boxes-cli stats"
$CLI stats test_json_array.txt > /dev/null
assert_success "boxes-cli stats works with json2canvas output"

# Test 20: Integration with boxes-cli list
test_start "Integration with boxes-cli list"
$CLI list test_json_array.txt > /dev/null
assert_success "boxes-cli list works with json2canvas output"

# Test 21: Integration with boxes-cli search
test_start "Integration with boxes-cli search for Alice"
output=$($CLI search test_json_array.txt "Alice")
echo "$output" | grep -q "Alice"
assert_success "boxes-cli search finds Alice"

# Test 22: Invalid JSON handling
test_start "Handle invalid JSON gracefully"
echo "{ invalid json }" | $CONNECTOR > test_json_invalid.txt 2>&1
assert_failure "Invalid JSON returns error"

# Test 23: Empty array
test_start "Handle empty JSON array"
echo "[]" | $CONNECTOR > test_json_empty.txt 2>&1
assert_failure "Empty array returns error"

# Test 24: Stdin input
test_start "Parse JSON from stdin"
cat test_sample_array.json | $CONNECTOR > test_json_stdin.txt
assert_success "Stdin input works"
assert_contains "test_json_stdin.txt" "BOXES_CANVAS_V1"

# Test 25: Array of primitives
test_start "Handle array of primitive values"
echo '["apple", "banana", "cherry"]' | $CONNECTOR > test_json_primitives.txt
assert_success "Primitive array parsing"
assert_contains "test_json_primitives.txt" "apple"

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
