#!/bin/bash
# test_docker2canvas.sh - Test suite for docker2canvas connector
#
# Tests Docker container parsing and canvas generation

# set -e  # Disabled for full test run

CONNECTOR="./connectors/docker2canvas"
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
    rm -f test_docker*.txt test_sample_docker*.txt
}

# Cleanup before starting
cleanup

echo "=========================================="
echo "docker2canvas Connector Tests"
echo "=========================================="

# Test 1: Docker ps table format
test_start "Create sample docker ps table output"
cat > test_sample_docker_table.txt <<'EOF'
CONTAINER ID   IMAGE          COMMAND                  CREATED        STATUS                      PORTS     NAMES
abc123def456   nginx:latest   "/docker-entrypoint.…"   2 hours ago    Up 2 hours                  80/tcp    web_server
789ghi012jkl   redis:alpine   "redis-server"           3 hours ago    Exited (0) 10 minutes ago             cache
mno345pqr678   postgres:14    "postgres"               1 day ago      Up 1 day                    5432/tcp  database
stu901vwx234   nginx:alpine   "nginx"                  2 days ago     Exited (1) 1 hour ago                 old_web
EOF
assert_file_exists "test_sample_docker_table.txt"

# Test 2: Parse docker ps table
test_start "Parse docker ps table format"
$CONNECTOR test_sample_docker_table.txt > test_docker_table.txt
assert_success "Connector execution"
assert_file_exists "test_docker_table.txt"
assert_contains "test_docker_table.txt" "BOXES_CANVAS_V1"

# Test 3: Verify box count
test_start "Verify correct number of boxes created"
box_count=$(head -3 test_docker_table.txt | tail -1)
if [ "$box_count" -eq "4" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 4"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 4 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 4: Check container names
test_start "Verify container names are extracted"
assert_contains "test_docker_table.txt" "web_server"
assert_contains "test_docker_table.txt" "database"

# Test 5: Docker JSON format
test_start "Create sample docker ps JSON output"
cat > test_sample_docker_json.txt <<'EOF'
{"Command":"/docker-entrypoint.sh nginx","CreatedAt":"2024-11-17 10:00:00","ID":"abc123def456","Image":"nginx:latest","Names":"web_server","Ports":"0.0.0.0:80->80/tcp","Status":"Up 2 hours"}
{"Command":"redis-server","CreatedAt":"2024-11-17 09:00:00","ID":"789ghi012jkl","Image":"redis:alpine","Names":"cache","Ports":"","Status":"Exited (0) 10 minutes ago"}
{"Command":"postgres","CreatedAt":"2024-11-16 10:00:00","ID":"mno345pqr678","Image":"postgres:14","Names":"database","Ports":"5432/tcp","Status":"Up 1 day"}
EOF
assert_file_exists "test_sample_docker_json.txt"

# Test 6: Parse docker JSON format
test_start "Parse docker ps JSON format"
$CONNECTOR test_sample_docker_json.txt --format json > test_docker_json.txt
assert_success "JSON format parsing"
assert_contains "test_docker_json.txt" "BOXES_CANVAS_V1"

# Test 7: Verify JSON box count
test_start "Verify JSON format box count"
box_count=$(head -3 test_docker_json.txt | tail -1)
if [ "$box_count" -eq "3" ]; then
    echo -e "  ${GREEN}✓${NC} Box count is correct: 3"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 3 boxes, found: $box_count"
    ((FAIL_COUNT++))
fi

# Test 8: Color coding for running containers
test_start "Verify running containers are green (color 2)"
grep -B 1 "web_server" test_docker_table.txt | grep -q " 2$"
assert_success "Running container has green color"

# Test 9: Color coding for exited containers
test_start "Verify exited containers are colored appropriately"
# Exited (0) should be yellow (3), Exited (1) should be red (1)
grep -B 1 "cache" test_docker_table.txt | head -1 | grep -q " 3$"
assert_success "Clean exit container has yellow color"

# Test 10: Grid layout
test_start "Test grid layout"
$CONNECTOR test_sample_docker_table.txt --layout grid > test_docker_grid.txt
assert_success "Grid layout"
assert_contains "test_docker_grid.txt" "BOXES_CANVAS_V1"

# Test 11: Status layout (default)
test_start "Test status layout (grouped)"
$CONNECTOR test_sample_docker_table.txt --layout status > test_docker_status.txt
assert_success "Status layout"
assert_contains "test_docker_status.txt" "BOXES_CANVAS_V1"

# Test 12: Show ports option
test_start "Test --show-ports option"
$CONNECTOR test_sample_docker_json.txt --format json --show-ports > test_docker_ports.txt
assert_success "Show ports option"
assert_contains "test_docker_ports.txt" "Ports:"

# Test 13: Auto-detect table format
test_start "Test auto-detect of table format"
$CONNECTOR test_sample_docker_table.txt --format auto > test_docker_auto.txt
assert_success "Auto-detect table format"
assert_contains "test_docker_auto.txt" "web_server"

# Test 14: Auto-detect JSON format
test_start "Test auto-detect of JSON format"
$CONNECTOR test_sample_docker_json.txt --format auto > test_docker_auto_json.txt
assert_success "Auto-detect JSON format"
box_count=$(head -3 test_docker_auto_json.txt | tail -1)
if [ "$box_count" -eq "3" ]; then
    echo -e "  ${GREEN}✓${NC} Auto-detected JSON correctly"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Auto-detect failed"
    ((FAIL_COUNT++))
fi

# Test 15: Integration with boxes-cli stats
test_start "Integration with boxes-cli stats"
$CLI stats test_docker_table.txt > /dev/null
assert_success "boxes-cli stats works with docker2canvas output"

# Test 16: Integration with boxes-cli list
test_start "Integration with boxes-cli list"
$CLI list test_docker_table.txt > /dev/null
assert_success "boxes-cli list works with docker2canvas output"

# Test 17: Integration with boxes-cli search
test_start "Integration with boxes-cli search"
output=$($CLI search test_docker_table.txt "nginx")
echo "$output" | grep -q "web_server"
assert_success "boxes-cli search finds nginx containers"

# Test 18: Empty input handling
test_start "Handle empty input gracefully"
echo "" | $CONNECTOR > test_docker_empty.txt 2>&1
assert_failure "Empty input returns error"

# Test 19: Stdin input
test_start "Parse docker output from stdin"
cat test_sample_docker_table.txt | $CONNECTOR > test_docker_stdin.txt
assert_success "Stdin input works"
assert_contains "test_docker_stdin.txt" "BOXES_CANVAS_V1"

# Test 20: Single container
test_start "Handle single container"
cat > test_sample_docker_single.txt <<'EOF'
CONTAINER ID   IMAGE          COMMAND      STATUS        NAMES
abc123         nginx:latest   "nginx"      Up 1 hour     web
EOF
$CONNECTOR test_sample_docker_single.txt > test_docker_single_output.txt
assert_success "Single container parsing"
box_count=$(head -3 test_docker_single_output.txt | tail -1)
if [ "$box_count" -eq "1" ]; then
    echo -e "  ${GREEN}✓${NC} Single container processed"
    ((PASS_COUNT++))
else
    echo -e "  ${RED}✗${NC} Expected 1 box, found: $box_count"
    ((FAIL_COUNT++))
fi

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
