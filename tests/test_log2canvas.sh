#!/bin/bash
# test_log2canvas.sh - Test suite for log2canvas connector
#
# Tests log file parsing and canvas generation

# set -e  # Disabled for full test run

CONNECTOR="./connectors/log2canvas"
CLI="./connectors/boxes-cli"
TEST_DIR="./tests"
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

assert_count() {
    count=$(grep -c "$2" "$1" || true)
    if [ "$count" -eq "$3" ]; then
        echo -e "  ${GREEN}✓${NC} Found $3 occurrences of: $2"
        ((PASS_COUNT++))
    else
        echo -e "  ${RED}✗${NC} Expected $3 occurrences of '$2', found $count"
        ((FAIL_COUNT++))
    fi
}

cleanup() {
    rm -f test_log*.txt test_sample*.log
}

# Cleanup before starting
cleanup

echo "=========================================="
echo "log2canvas Connector Tests"
echo "=========================================="

# Create sample log files

# Test 1: Generic log format
test_start "Create sample generic log file"
cat > test_sample_generic.log <<'EOF'
2024-11-17 10:00:00 INFO Application started successfully
2024-11-17 10:00:05 DEBUG Loading configuration from config.yaml
2024-11-17 10:00:10 INFO Connected to database
2024-11-17 10:00:15 WARNING Cache miss for key: user_123
2024-11-17 10:00:20 ERROR Failed to connect to external API: timeout
2024-11-17 10:00:25 INFO Request processed in 45ms
2024-11-17 10:00:30 CRITICAL System memory exceeded 90% threshold
EOF
assert_file_exists "test_sample_generic.log"

# Test 2: Parse generic log format
test_start "Parse generic log format"
$CONNECTOR test_sample_generic.log > test_log_generic.txt
assert_success "Connector execution"
assert_file_exists "test_log_generic.txt"
assert_contains "test_log_generic.txt" "BOXES_CANVAS_V1"

# Test 3: Verify box count
test_start "Verify correct number of boxes created"
box_count=$(head -3 test_log_generic.txt | tail -1)
if [ "$box_count" -eq "7" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 7"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 7 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 4: Check color coding for ERROR
test_start "Verify ERROR log entries are colored red (color 1)"
assert_contains "test_log_generic.txt" "ERROR"
# The box with ERROR should have color 1 (red)
grep -B 1 "\[ERROR\]" test_log_generic.txt | grep -q " 1$"
assert_success "ERROR log entry has red color"

# Test 5: Check color coding for WARNING
test_start "Verify WARNING log entries are colored yellow (color 3)"
grep -B 1 "\[WARNING\]" test_log_generic.txt | grep -q " 3$"
assert_success "WARNING log entry has yellow color"

# Test 6: Check color coding for INFO
test_start "Verify INFO log entries are colored blue (color 4)"
grep -B 1 "\[INFO\]" test_log_generic.txt | head -2 | grep -q " 4$"
assert_success "INFO log entry has blue color"

# Test 7: Check color coding for DEBUG
test_start "Verify DEBUG log entries are colored green (color 2)"
grep -B 1 "\[DEBUG\]" test_log_generic.txt | grep -q " 2$"
assert_success "DEBUG log entry has green color"

# Test 8: JSON log format
test_start "Create sample JSON log file"
cat > test_sample_json.log <<'EOF'
{"timestamp": "2024-11-17T10:00:00Z", "level": "INFO", "message": "Server started", "source": "app"}
{"timestamp": "2024-11-17T10:00:05Z", "level": "ERROR", "message": "Database connection failed", "source": "db"}
{"timestamp": "2024-11-17T10:00:10Z", "level": "WARNING", "message": "High memory usage detected", "source": "monitor"}
EOF
assert_file_exists "test_sample_json.log"

# Test 9: Parse JSON log format
test_start "Parse JSON log format"
$CONNECTOR test_sample_json.log --format json > test_log_json.txt
assert_success "JSON log parsing"
assert_contains "test_log_json.txt" "BOXES_CANVAS_V1"

# Test 10: Verify JSON log box count
test_start "Verify JSON log box count"
box_count=$(head -3 test_log_json.txt | tail -1)
if [ "$box_count" -eq "3" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 3"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 3 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 11: Syslog format
test_start "Create sample syslog format"
cat > test_sample_syslog.log <<'EOF'
Nov 17 10:00:00 server1 kernel: eth0: link up
Nov 17 10:00:05 server1 sshd: ERROR Failed authentication for user admin
Nov 17 10:00:10 server1 nginx: INFO Request from 192.168.1.100
Nov 17 10:00:15 server1 cron: WARNING Task took longer than expected
EOF
assert_file_exists "test_sample_syslog.log"

# Test 12: Parse syslog format
test_start "Parse syslog format"
$CONNECTOR test_sample_syslog.log --format syslog > test_log_syslog.txt
assert_success "Syslog parsing"
assert_contains "test_log_syslog.txt" "BOXES_CANVAS_V1"

# Test 13: Test max entries limit
test_start "Test --max limit option"
$CONNECTOR test_sample_generic.log --max 3 > test_log_max.txt
box_count=$(head -3 test_log_max.txt | tail -1)
if [ "$box_count" -eq "3" ]; then
    echo -e "  ${GREEN}✓${NC} Max limit respected: 3"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 3 boxes with --max 3, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 14: Test grid layout
test_start "Test grid layout option"
$CONNECTOR test_sample_generic.log --layout grid > test_log_grid.txt
assert_success "Grid layout"
assert_contains "test_log_grid.txt" "BOXES_CANVAS_V1"

# Test 15: Integration with boxes-cli stats
test_start "Integration with boxes-cli stats"
$CLI stats test_log_generic.txt > /dev/null
assert_success "boxes-cli stats works with log2canvas output"

# Test 16: Integration with boxes-cli list
test_start "Integration with boxes-cli list"
$CLI list test_log_generic.txt > /dev/null
assert_success "boxes-cli list works with log2canvas output"

# Test 17: Integration with boxes-cli search
test_start "Integration with boxes-cli search for ERROR"
output=$($CLI search test_log_generic.txt "ERROR")
echo "$output" | grep -q "ERROR"
assert_success "boxes-cli search finds ERROR entries"

# Test 18: Filter by color (red/ERROR)
test_start "Filter boxes by color (red for errors)"
$CLI list test_log_generic.txt --color 1 > test_log_filter.txt
assert_success "Color filtering works"

# Test 19: Empty log file handling
test_start "Handle empty log file gracefully"
touch test_sample_empty.log
$CONNECTOR test_sample_empty.log > test_log_empty.txt 2>&1
assert_failure "Empty log file returns error"

# Test 20: Stdin input
test_start "Parse logs from stdin"
cat test_sample_generic.log | $CONNECTOR > test_log_stdin.txt
assert_success "Stdin input works"
assert_contains "test_log_stdin.txt" "BOXES_CANVAS_V1"

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
