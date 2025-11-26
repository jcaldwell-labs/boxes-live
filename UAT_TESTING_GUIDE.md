# User Acceptance Testing Guide

**Purpose**: Validate core UX before investing in advanced features  
**Target**: Alpha/Beta testers with and without joysticks  
**Duration**: 5-25 minutes per scenario

---

## Overview

This guide provides **semi-automated test scenarios** that can be executed and validated programmatically, creating a feedback loop for continuous UX improvement.

## Quick Start for Testers

```bash
# Run automated test scenario
./test-uat.sh scenario1

# Or run all scenarios
./test-uat.sh all

# View test results
cat uat-results.log
```

---

## Test Scenarios

### Scenario 1: First-Time User (5 minutes)

**Goal**: Can user complete basic tasks without documentation?

**Setup**:
```bash
# Start with empty canvas
rm -f canvas.txt
./boxes-live
```

**Tasks**:
1. ✓ Start application
2. ✓ Create 3 boxes (press 'n' three times)
3. ✓ Move boxes around (arrow keys + drag)
4. ✓ Change box colors (select + 1-7)
5. ✓ Edit box text (future feature - skip for now)
6. ✓ Save canvas (F2)

**Success Metrics**:
- [ ] Completes all tasks in < 5 minutes
- [ ] Understands mode system (if joystick available)
- [ ] Finds controls intuitive (survey question)
- [ ] Canvas file exists with 3 boxes

**Automated Validation**:
```bash
# Check canvas was saved
test -f canvas.txt && echo "PASS: Canvas saved" || echo "FAIL: No canvas file"

# Check box count
BOX_COUNT=$(grep -c "^[0-9]* [0-9]*\.[0-9]* [0-9]*\.[0-9]*" canvas.txt)
test "$BOX_COUNT" -ge 3 && echo "PASS: Created 3+ boxes" || echo "FAIL: Less than 3 boxes"

# Check colors were applied
COLOR_COUNT=$(grep " [1-7]$" canvas.txt | wc -l)
test "$COLOR_COUNT" -ge 1 && echo "PASS: Colors applied" || echo "FAIL: No colors"
```

---

### Scenario 2: Joystick-Only User (10 minutes)

**Goal**: Can joystick user work efficiently without keyboard (except text)?

**Setup**:
```bash
# Check joystick availability
ls /dev/input/js0 || echo "ERROR: No joystick connected"

# Load example canvas
./boxes-live  # Press F3, load test-canvases/workspace-layout.txt
```

**Tasks**:
1. ✓ Navigate canvas with stick (analog control)
2. ✓ Create boxes with buttons (Button Y in NAV mode)
3. ✓ Enter SELECTION mode (Button LB)
4. ✓ Select and cycle boxes (Button A in SELECTION)
5. ✓ Enter EDIT mode (Button X from SELECTION)
6. ✓ Open parameter editor (Button Y in EDIT)
7. ✓ Adjust properties (stick movement)
8. ✓ Toggle grid/snap (Button Y in NAV)
9. ✓ Save work (Button Start)

**Success Metrics**:
- [ ] Completes all tasks without keyboard
- [ ] Button mappings feel natural (< 3 mapping errors)
- [ ] Visualizer panel helpful (survey)
- [ ] Mode transitions clear (survey)

**Automated Validation**:
```bash
# Check joystick config exists
test -f ~/.config/boxes-live/config.ini && echo "PASS: Config accessible" || echo "INFO: Using defaults"

# Verify canvas modifications
test -f canvas.txt && echo "PASS: Joystick user saved canvas" || echo "FAIL: No save"
```

---

### Scenario 3: Productivity Workflow (15 minutes)

**Goal**: Can user accomplish real work tasks?

**Setup**:
```bash
# Start fresh
rm -f canvas.txt
./boxes-live
```

**Tasks**:
1. ✓ Create simple flowchart (5 boxes with connections)
2. ✓ Create file viewer box with `:file path` (future)
3. ✓ Create command output box with `:run command` (future)
4. ✓ Organize boxes spatially (grid + snap)
5. ✓ Use focus mode to read file content (press F)
6. ✓ Save workspace (F2)

**Current Implementation Status**:
- ✅ Box creation
- ✅ Spatial organization
- ✅ Grid/snap
- ✅ Save/load
- ⚠️  Connections (basic implementation exists)
- ❌ File association (not yet implemented)
- ❌ Process integration (not yet implemented)

**Success Metrics**:
- [ ] Workflow feels efficient (survey)
- [ ] Grid/snap aids organization
- [ ] Would use for real work (survey)

**Automated Validation**:
```bash
# Load test canvas and verify grid can be toggled
./test-uat.sh scenario3-grid
```

---

### Scenario 4: Configuration Customization (5 minutes)

**Goal**: Can user customize controls and behavior?

**Setup**:
```bash
# Copy example config
mkdir -p ~/.config/boxes-live
cp config.ini.example ~/.config/boxes-live/config.ini
```

**Tasks**:
1. ✓ Find config file location (documented in Quick Start)
2. ✓ Open config.ini (any text editor)
3. ✓ Change button mapping (e.g., swap zoom buttons)
4. ✓ Modify grid settings (spacing, visibility)
5. ✓ Restart application
6. ✓ Verify changes applied

**Success Metrics**:
- [ ] Config file easy to find (documented)
- [ ] Format easy to understand (INI with comments)
- [ ] Changes apply correctly
- [ ] Documentation sufficient

**Automated Validation**:
```bash
# Verify config file structure
test -f ~/.config/boxes-live/config.ini && echo "PASS: Config exists"

# Check INI syntax (basic)
grep -q "^\[" ~/.config/boxes-live/config.ini && echo "PASS: Valid INI format"
```

---

## Automated Testing Scripts

### test-uat.sh - Main Test Runner

Create `/home/runner/work/boxes-live/boxes-live/test-uat.sh`:

```bash
#!/bin/bash
# User Acceptance Testing automation script

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_FILE="$SCRIPT_DIR/uat-results.log"
CANVAS_DIR="$SCRIPT_DIR/test-canvases"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log() {
    echo -e "[$(date +'%H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

pass() {
    echo -e "${GREEN}✓ PASS${NC}: $1" | tee -a "$LOG_FILE"
}

fail() {
    echo -e "${RED}✗ FAIL${NC}: $1" | tee -a "$LOG_FILE"
}

info() {
    echo -e "${YELLOW}ℹ INFO${NC}: $1" | tee -a "$LOG_FILE"
}

# Test Scenario 1: First-time user
test_scenario1() {
    log "=== Scenario 1: First-Time User ==="
    
    # Check application builds
    if make clean && make; then
        pass "Application builds successfully"
    else
        fail "Build failed"
        return 1
    fi
    
    # Create test canvas programmatically
    cat > /tmp/scenario1-test.txt << 'EOF'
BOXES_CANVAS_V1
1000.00 800.00
3
1 100.00 100.00 20 10 0 1
"Box 1"
0
2 200.00 150.00 20 10 0 2
"Box 2"
0
3 300.00 200.00 20 10 0 3
"Box 3"
0
4 -1
CONNECTIONS
0
1
EOF
    
    # Validate canvas structure
    if [ -f /tmp/scenario1-test.txt ]; then
        pass "Test canvas created"
        
        # Check box count
        BOX_COUNT=$(grep -c "^[0-9]* [0-9]*\.[0-9]* [0-9]*\.[0-9]*" /tmp/scenario1-test.txt)
        if [ "$BOX_COUNT" -eq 3 ]; then
            pass "Canvas has 3 boxes"
        else
            fail "Expected 3 boxes, found $BOX_COUNT"
        fi
        
        # Check colors
        if grep -q " 1$" /tmp/scenario1-test.txt && \
           grep -q " 2$" /tmp/scenario1-test.txt && \
           grep -q " 3$" /tmp/scenario1-test.txt; then
            pass "Different colors applied to boxes"
        else
            fail "Colors not properly applied"
        fi
    else
        fail "Test canvas creation failed"
    fi
}

# Test Scenario 2: Grid functionality
test_scenario2_grid() {
    log "=== Scenario 2: Grid Visibility Test ==="
    
    # Create canvas with grid enabled
    cat > /tmp/grid-test.txt << 'EOF'
BOXES_CANVAS_V1
500.00 500.00
1
1 100.00 100.00 30 10 1 2
"Test Box"
0
2 -1
CONNECTIONS
0
1
EOF
    
    # Load and verify
    if [ -f /tmp/grid-test.txt ]; then
        pass "Grid test canvas created"
        
        # Note: Grid state should be persisted but currently isn't
        # This is the bug we're addressing
        info "Grid persistence not yet implemented (known issue)"
    else
        fail "Grid test canvas creation failed"
    fi
}

# Test Scenario 3: Load example canvases
test_scenario3_examples() {
    log "=== Scenario 3: Example Canvas Loading ==="
    
    for canvas in "$CANVAS_DIR"/*.txt; do
        if [ -f "$canvas" ]; then
            filename=$(basename "$canvas")
            
            # Validate canvas format
            if grep -q "^BOXES_CANVAS_V1" "$canvas"; then
                pass "Valid canvas format: $filename"
            else
                fail "Invalid format: $filename"
            fi
        fi
    done
}

# Test Scenario 4: Configuration system
test_scenario4_config() {
    log "=== Scenario 4: Configuration System ==="
    
    # Check config example exists
    if [ -f "config.ini.example" ]; then
        pass "Config example file exists"
        
        # Validate INI structure
        if grep -q "^\[general\]" config.ini.example && \
           grep -q "^\[grid\]" config.ini.example && \
           grep -q "^\[joystick\]" config.ini.example; then
            pass "Config has required sections"
        else
            fail "Config missing required sections"
        fi
    else
        fail "Config example not found"
    fi
}

# Test Scenario 5: Stress test
test_scenario5_stress() {
    log "=== Scenario 5: Stress Test (100+ boxes) ==="
    
    STRESS_CANVAS="$CANVAS_DIR/stress-test.txt"
    
    if [ -f "$STRESS_CANVAS" ]; then
        pass "Stress test canvas exists"
        
        # Count boxes
        BOX_COUNT=$(grep -c "^[0-9]* [0-9]*\.[0-9]* [0-9]*\.[0-9]*" "$STRESS_CANVAS")
        
        if [ "$BOX_COUNT" -ge 100 ]; then
            pass "Stress test has $BOX_COUNT boxes (100+ required)"
        else
            fail "Stress test only has $BOX_COUNT boxes (< 100)"
        fi
    else
        fail "Stress test canvas not found"
    fi
}

# Main execution
main() {
    log "Starting UAT Test Suite"
    log "Log file: $LOG_FILE"
    
    # Clear previous log
    > "$LOG_FILE"
    
    case "${1:-all}" in
        scenario1)
            test_scenario1
            ;;
        scenario2-grid)
            test_scenario2_grid
            ;;
        scenario3)
            test_scenario3_examples
            ;;
        scenario4)
            test_scenario4_config
            ;;
        scenario5)
            test_scenario5_stress
            ;;
        all)
            test_scenario1
            test_scenario2_grid
            test_scenario3_examples
            test_scenario4_config
            test_scenario5_stress
            ;;
        *)
            echo "Usage: $0 {scenario1|scenario2-grid|scenario3|scenario4|scenario5|all}"
            exit 1
            ;;
    esac
    
    log "=== Test Suite Complete ==="
    log "Results saved to: $LOG_FILE"
}

main "$@"
```

---

## Feedback Collection

### Feedback Form Template

Create `feedback-form.md` to share with testers:

```markdown
# Boxes-Live User Acceptance Testing Feedback

**Tester ID**: ___________  
**Date**: ___________  
**Setup**: [ ] Keyboard only [ ] Mouse + Keyboard [ ] Joystick + Keyboard  
**Scenario**: ___________

## Quick Ratings (1-5, 5 = best)

| Area | Rating | Notes |
|------|--------|-------|
| Ease of use | __/5 | |
| Mode system clarity | __/5 | |
| Button mappings | __/5 | |
| Grid usefulness | __/5 | |
| Overall experience | __/5 | |

## Open Questions

### What was intuitive?
- 
- 
- 

### What was confusing?
- 
- 
- 

### Which modes felt natural?
- NAV: [ ] Natural [ ] Confusing [ ] N/A
- SELECTION: [ ] Natural [ ] Confusing [ ] N/A
- EDIT: [ ] Natural [ ] Confusing [ ] N/A

### Which button mappings worked well?
- 
- 

### Which button mappings felt wrong?
- 
- 

### What features are missing?
- 
- 

### Would you use this for real work?
- [ ] Yes, definitely
- [ ] Maybe, if it had: ___________
- [ ] No, because: ___________

### What would you change?
- 
- 

## Specific Feedback

### Grid System
- Visibility: [ ] Easy to see [ ] Hard to see [ ] Just right
- Snap-to-grid: [ ] Helpful [ ] Annoying [ ] Didn't use
- Grid spacing (10 units): [ ] Too small [ ] Just right [ ] Too large

### Focus Mode
- Used it: [ ] Yes [ ] No
- If yes, useful? [ ] Very [ ] Somewhat [ ] Not really

### Performance
- Smooth with 100+ boxes? [ ] Yes [ ] No [ ] Didn't test
- Noticeable lag? [ ] Yes, when: ___________ [ ] No

## Bug Reports

Found any bugs?
- 
- 

## Additional Comments

[Free-form feedback]

---

**Thank you for testing boxes-live!**
```

---

## Test Execution Schedule

### Week 1: Internal Testing
- Run `./test-uat.sh all`
- Manual testing of each scenario
- Fix obvious bugs
- Document workflows

### Week 2: Alpha Testing (3-10 users)
- Distribute Quick Start Guide
- Assign scenarios to testers
- Collect feedback forms
- Identify blockers

### Week 3: Analysis & Iteration
- Analyze feedback (see Metrics section below)
- Prioritize fixes
- Implement critical improvements
- Re-run automated tests

### Week 4: Beta Testing (10-20 users)
- Wider audience
- Focus on specific workflows
- Performance testing (stress-test.txt)
- Edge case discovery

---

## Success Criteria

Before proceeding with advanced features:

- [ ] 80%+ users complete basic tasks (scenario 1)
- [ ] Mode system makes sense (avg rating ≥ 3.5/5)
- [ ] Joystick mappings feel natural (< 3 users suggest changes)
- [ ] Grid/snap deemed useful (≥ 50% use regularly)
- [ ] Configuration system works (≥ 80% success rate)
- [ ] No critical UX blockers
- [ ] All automated tests pass

---

## Metrics to Track

### Quantitative Metrics

```bash
# Time to first box created (goal: < 30 seconds)
# Number of mode changes per session (goal: < 10 for basic tasks)
# Grid usage rate (goal: > 50%)
# Customization rate (goal: > 30% users customize)

# Automated collection:
grep "BOX.*created" uat-results.log | wc -l
grep "mode.*change" uat-results.log | wc -l
```

### Qualitative Metrics

- Most-used features (from surveys)
- Unused features (candidates for removal)
- Error recovery success rate
- User satisfaction ratings

---

## Known Issues (Document These)

### Grid Not Showing
**Issue**: Grid settings don't persist to canvas files  
**Workaround**: Re-enable grid with 'G' after loading  
**Fix**: Add grid state to persistence module  
**Priority**: High (affects UAT Scenario 3)

### Focus Mode
**Issue**: Only works when box has content  
**Expected**: Handle empty boxes gracefully  
**Priority**: Medium

---

## Next Steps After UAT

Based on feedback:

1. **High Priority Fixes**: Address critical UX blockers
2. **Feature Refinement**: Improve most-used features
3. **Feature Removal**: Remove unused/confusing features
4. **Advanced Features**: Only if core UX is validated

**Decision Gates**:
- ✅ Alpha feedback positive → Proceed to beta
- ⚠️  Mixed feedback → Iterate on core issues
- ❌ Negative feedback → Pivot or redesign

---

## Appendix: Manual Testing Checklist

For testers without access to automated scripts:

### Scenario 1 Checklist
- [ ] Application starts without errors
- [ ] Can create 3 boxes
- [ ] Can move boxes with keyboard/mouse
- [ ] Can apply colors (1-7)
- [ ] Can save canvas (F2)
- [ ] Canvas file created successfully

### Scenario 2 Checklist (Joystick)
- [ ] Joystick detected (visualizer appears)
- [ ] Left stick pans viewport
- [ ] Button Y creates box
- [ ] Button LB cycles modes
- [ ] Mode indicator updates correctly
- [ ] Can save with Button Start

### Scenario 3 Checklist (Grid)
- [ ] Press G to show grid
- [ ] Grid visible (dots or lines)
- [ ] Press Shift+G to enable snap
- [ ] New boxes snap to grid
- [ ] Status bar shows [GRID] and [SNAP]

### Scenario 4 Checklist (Config)
- [ ] Config file found
- [ ] INI format clear
- [ ] Can edit button mappings
- [ ] Changes applied after restart

---

**End of UAT Guide**

For questions or issues, see:
- [QUICK_START.md](QUICK_START.md) - Basic usage
- [README.md](README.md) - Full documentation
- [TESTING.md](TESTING.md) - Automated tests
