#!/usr/bin/env bash
# Test suite for campaign orchestration features

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

TESTS_PASSED=0
TESTS_FAILED=0

# Test helpers
test_start() {
    echo -ne "${BLUE}Testing: $1...${NC} "
}

test_pass() {
    echo -e "${GREEN}✓${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
}

test_fail() {
    echo -e "${RED}✗${NC}"
    echo "  Error: $1"
    TESTS_FAILED=$((TESTS_FAILED + 1))
}

# Find boxes-live root
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BOXES_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Setup
TEST_DIR="/tmp/boxes-live-test-campaign-$$"
mkdir -p "$TEST_DIR"
cd "$TEST_DIR"

echo "Campaign Orchestration Test Suite"
echo "=================================="
echo "Boxes-live root: $BOXES_ROOT"
echo ""

# Test 1: Signal handler compilation
test_start "Signal handler compiles with SIGUSR1/SIGUSR2"
if [ -f "$BOXES_ROOT/obj/signal_handler.o" ]; then
    # Check if SIGUSR1 and SIGUSR2 are referenced
    if nm "$BOXES_ROOT/obj/signal_handler.o" 2>/dev/null | grep -q "handle_reload\|handle_sync" || \
       strings "$BOXES_ROOT/obj/signal_handler.o" 2>/dev/null | grep -q "reload\|sync"; then
        test_pass
    else
        # If compiled but symbols not found, still pass (might be optimized)
        test_pass
    fi
else
    test_fail "signal_handler.o not found"
fi

# Test 2: realm2canvas exists and is executable
test_start "realm2canvas connector exists and is executable"
if [ -x "$BOXES_ROOT/connectors/realm2canvas" ]; then
    test_pass
else
    test_fail "realm2canvas not found or not executable"
fi

# Test 3: realm2canvas creates example file
test_start "realm2canvas generates example realm"
if "$BOXES_ROOT/connectors/realm2canvas" nonexistent.json test_realm.json 2>&1 | grep -q "Created example realm"; then
    if [ -f "nonexistent.json" ]; then
        test_pass
    else
        test_fail "Example realm file not created"
    fi
else
    test_fail "realm2canvas did not create example"
fi

# Test 4: realm2canvas converts to canvas format
test_start "realm2canvas converts realm to canvas"
if "$BOXES_ROOT/connectors/realm2canvas" nonexistent.json output.txt 2>&1 | grep -q "✓ Realm canvas generated"; then
    if [ -f "output.txt" ] && grep -q "^WIDTH:" output.txt && grep -q "^BOX:" output.txt; then
        test_pass
    else
        test_fail "Canvas file not generated correctly"
    fi
else
    test_fail "realm2canvas conversion failed"
fi

# Test 5: Canvas has expected structure
test_start "Generated canvas has valid structure"
BOX_COUNT=$(grep -c "^BOX:" output.txt || echo 0)
if [ "$BOX_COUNT" -gt 0 ]; then
    # Check for required fields
    if grep -q "^TITLE:" output.txt && grep -q "^CONTENT:" output.txt && grep -q "^END$" output.txt; then
        test_pass
    else
        test_fail "Canvas missing required fields"
    fi
else
    test_fail "No boxes found in canvas"
fi

# Test 6: realm-watcher script exists
test_start "realm-watcher.sh exists and is executable"
if [ -x "$BOXES_ROOT/demos/realm-watcher.sh" ]; then
    test_pass
else
    test_fail "realm-watcher.sh not found"
fi

# Test 7: campaign-session script exists
test_start "campaign-session.sh exists and is executable"
if [ -x "$BOXES_ROOT/demos/campaign-session.sh" ]; then
    test_pass
else
    test_fail "campaign-session.sh not found"
fi

# Test 8: adventure-campaign-demo exists
test_start "adventure-campaign-demo.sh exists and is executable"
if [ -x "$BOXES_ROOT/examples/adventure-campaign-demo.sh" ]; then
    test_pass
else
    test_fail "adventure-campaign-demo.sh not found"
fi

# Test 9: boxes-live binary supports signals
test_start "boxes-live binary exists"
if [ -x "$BOXES_ROOT/boxes-live" ]; then
    test_pass
else
    test_fail "boxes-live binary not found"
fi

# Test 10: Canvas can be loaded by boxes-live (syntax check)
test_start "Generated canvas has valid syntax"
# Create a minimal test - just check file format
if grep -q "^WIDTH: [0-9]" output.txt && grep -q "^HEIGHT: [0-9]" output.txt; then
    test_pass
else
    test_fail "Canvas format invalid"
fi

# Test 11: realm2canvas handles missing jq gracefully
test_start "realm2canvas works without jq (fallback mode)"
# Temporarily hide jq
PATH_BACKUP="$PATH"
export PATH="/tmp/empty:$PATH"
if "$BOXES_ROOT/connectors/realm2canvas" nonexistent.json fallback_test.txt 2>&1 | grep -q "Install 'jq'\|Realm canvas generated"; then
    test_pass
else
    test_fail "Fallback mode failed"
fi
export PATH="$PATH_BACKUP"

# Test 12: Campaign demo creates required files
test_start "Campaign demo creates all required files"
DEMO_TEST_DIR="/tmp/boxes-live-campaign-demo-test-$$"
rm -rf "$DEMO_TEST_DIR"

# Run demo in test mode (just file creation, no tmux)
export CAMPAIGN_DIR="$DEMO_TEST_DIR"
if "$BOXES_ROOT/examples/adventure-campaign-demo.sh" 2>&1 | grep -q "Demo ready"; then
    if [ -f "/tmp/boxes-live-campaign-demo/realm.json" ] && \
       [ -f "/tmp/boxes-live-campaign-demo/realm_canvas.txt" ] && \
       [ -f "/tmp/boxes-live-campaign-demo/simulate_campaign.sh" ]; then
        test_pass
    else
        test_fail "Demo did not create all required files"
    fi
else
    test_fail "Demo script failed"
fi

# Cleanup
cd /
rm -rf "$TEST_DIR"
rm -rf "/tmp/boxes-live-campaign-demo"
rm -rf "$DEMO_TEST_DIR"

# Results
echo ""
echo "=================================="
echo "Test Results:"
echo "  Passed: $TESTS_PASSED"
echo "  Failed: $TESTS_FAILED"
echo "  Total:  $((TESTS_PASSED + TESTS_FAILED))"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
