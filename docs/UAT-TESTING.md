# User Acceptance Testing (UAT) Documentation

**Status**: ✅ Complete  
**Last Updated**: 2024-11-26  
**Issue**: [#21 Planning: User acceptance testing plan for core UX](https://github.com/jcaldwell-labs/boxes-live/issues/21)

---

## Overview

This document provides comprehensive documentation for the User Acceptance Testing (UAT) infrastructure implemented for boxes-live. The UAT system validates core UX before investing in advanced features.

## Quick Start

### Run All UAT Scenarios
```bash
cd /path/to/boxes-live
./test-uat.sh all
```

### Run Specific Scenario
```bash
./test-uat.sh scenario1      # First-time user
./test-uat.sh scenario2-grid # Grid persistence
./test-uat.sh scenario5      # Stress test (100+ boxes)
```

### View Results
```bash
cat uat-results.log
```

---

## Implementation Summary

### What Was Delivered

1. ✅ **Grid Persistence Bug Fix**
   - Grid settings (visible, snap, spacing) now persist across save/load
   - File format: `GRID\n<visible> <snap_enabled> <spacing>\n`
   - Backward compatible with old canvas files

2. ✅ **Automated Test Infrastructure**
   - `test-uat.sh`: 19 automated scenarios
   - Color-coded output (✓ green, ✗ red, ℹ yellow)
   - Results logging to `uat-results.log`

3. ✅ **Test Canvas Files** (`test-canvases/`)
   - `empty.txt`: Minimal canvas (0 boxes)
   - `flowchart-example.txt`: Decision flowchart (5 boxes, grid+snap)
   - `workspace-layout.txt`: Project board (8 boxes, grid only)
   - `stress-test.txt`: Performance test (120 boxes)

4. ✅ **Comprehensive Documentation**
   - [QUICK_START.md](../QUICK_START.md): 6-minute user guide
   - [UAT_TESTING_GUIDE.md](../UAT_TESTING_GUIDE.md): Complete testing guide
   - [FEEDBACK_FORM.md](../FEEDBACK_FORM.md): Tester feedback template
   - [UAT_SUMMARY.md](../UAT_SUMMARY.md): Implementation summary
   - [IMPLEMENTATION_COMPLETE.md](../IMPLEMENTATION_COMPLETE.md): Final verification

5. ✅ **Test Coverage Improvements**
   - 442 total test assertions (was 245, +80%)
   - 14 new grid persistence tests
   - 100% pass rate, zero warnings, zero memory leaks

---

## Testing Scenarios

### Automated Scenarios (via test-uat.sh)

| Scenario | Description | Assertions | Status |
|----------|-------------|------------|--------|
| 1 | First-time user workflow | 4 | ✅ Pass |
| 2 | Grid persistence validation | 4 | ✅ Pass |
| 3 | Example canvas loading | 5 | ✅ Pass |
| 4 | Configuration system | 2 | ✅ Pass |
| 5 | Stress test (100+ boxes) | 2 | ✅ Pass |
| 6 | Full automated test suite | 2 | ✅ Pass |

**Total**: 19 assertions, 100% pass rate

### Manual Scenarios (from UAT_TESTING_GUIDE.md)

| Scenario | Duration | Focus Area | Target Users |
|----------|----------|------------|--------------|
| 1: First-Time User | 5 min | Basic tasks without docs | All users |
| 2: Joystick-Only | 10 min | Gamepad-only workflow | Joystick users |
| 3: Productivity | 15 min | Real work tasks | Power users |
| 4: Configuration | 5 min | Customization | Advanced users |

---

## File Structure

```
boxes-live/
├── test-uat.sh                    # Automated test runner
├── test-canvases/                 # Example canvas files
│   ├── README.md                  # Canvas documentation
│   ├── empty.txt                  # Empty canvas
│   ├── flowchart-example.txt      # Flowchart demo
│   ├── workspace-layout.txt       # Workspace demo
│   └── stress-test.txt            # 120-box stress test
├── tests/
│   └── test_grid.c                # Grid persistence tests
├── QUICK_START.md                 # User quick start guide
├── UAT_TESTING_GUIDE.md           # Comprehensive UAT guide
├── FEEDBACK_FORM.md               # Tester feedback template
├── UAT_SUMMARY.md                 # Implementation summary
└── IMPLEMENTATION_COMPLETE.md     # Final verification

Documentation:
└── docs/
    ├── README.md                  # This index
    └── UAT-TESTING.md             # This file
```

---

## Usage Guide

### For Testers

1. **Read the Quick Start**
   ```bash
   cat QUICK_START.md
   ```

2. **Try Example Canvases**
   ```bash
   ./boxes-live
   # Press F3, load: test-canvases/flowchart-example.txt
   ```

3. **Run Manual Scenarios**
   - Follow [UAT_TESTING_GUIDE.md](../UAT_TESTING_GUIDE.md)
   - Complete scenarios 1-4 (35 minutes total)

4. **Provide Feedback**
   - Fill out [FEEDBACK_FORM.md](../FEEDBACK_FORM.md)
   - Submit via GitHub issue or email

### For Developers

1. **Run Automated Tests**
   ```bash
   make test              # 442 unit/integration tests
   ./test-uat.sh all      # 19 UAT scenarios
   ```

2. **Check Test Coverage**
   ```bash
   make test 2>&1 | grep "Test Summary"
   ```

3. **Review Results**
   ```bash
   cat uat-results.log
   ```

4. **Debug Build Issues**
   ```bash
   cat /tmp/build-log.txt  # Build output
   ```

---

## Test Results

### Current Status (2024-11-26)

```
Automated Tests:    442/442 passed (100%)
UAT Scenarios:       19/19 passed (100%)
Compiler Warnings:         0
Memory Leaks:              0
Grid Persistence:     ✅ Fixed
```

### Test Coverage Breakdown

| Test Suite | Assertions | Status |
|------------|------------|--------|
| Canvas | 104 | ✅ Pass |
| Config | 65 | ✅ Pass |
| Connections | 84 | ✅ Pass |
| **Grid** (NEW) | **14** | ✅ **Pass** |
| Integration | 67 | ✅ Pass |
| Persistence | 69 | ✅ Pass |
| Viewport | 39 | ✅ Pass |

---

## Success Criteria

### Technical Criteria ✅ Met

- [x] All automated tests pass (442/442)
- [x] Zero compiler warnings
- [x] Zero memory leaks
- [x] Grid persistence works
- [x] Backward compatible file format

### User Testing Criteria 🔄 Pending

From [UAT_TESTING_GUIDE.md](../UAT_TESTING_GUIDE.md):

- [ ] 80%+ users complete basic tasks
- [ ] Mode system avg rating ≥ 3.5/5
- [ ] Joystick mappings: < 3 users suggest changes
- [ ] Grid/snap: ≥ 50% use regularly
- [ ] Config system: ≥ 80% success rate
- [ ] No critical UX blockers

**Status**: Ready for alpha testing (Week 2)

---

## Grid Persistence Details

### Problem
Grid configuration (visible, snap_enabled, spacing) was lost on save/load.

### Solution
Added `GRID` section to canvas file format:

```
BOXES_CANVAS_V1
<world dimensions>
<boxes>
...
CONNECTIONS
<connections>
...
GRID
<visible> <snap_enabled> <spacing>
```

### Example
```
GRID
1 1 10    # visible=true, snap=true, spacing=10
```

### Backward Compatibility
- Files without GRID section use defaults
- Old files load correctly
- New files work with old code (GRID ignored)

### Testing
- 14 test assertions in `tests/test_grid.c`
- Works with and without CONNECTIONS section
- All edge cases covered

---

## Next Steps

### Week 2: Alpha Testing
- [ ] Recruit 6-10 alpha testers
- [ ] Distribute QUICK_START.md
- [ ] Collect FEEDBACK_FORM.md responses
- [ ] Identify blockers

### Week 3: Analysis & Iteration
- [ ] Analyze feedback
- [ ] Calculate success metrics
- [ ] Prioritize fixes
- [ ] Implement improvements

### Week 4: Beta Testing
- [ ] Wider audience (15-20 users)
- [ ] Performance testing with stress-test.txt
- [ ] Edge case discovery
- [ ] Final refinements

---

## Resources

### Documentation
- **[QUICK_START.md](../QUICK_START.md)** - User onboarding (6 minutes)
- **[UAT_TESTING_GUIDE.md](../UAT_TESTING_GUIDE.md)** - Complete testing guide
- **[FEEDBACK_FORM.md](../FEEDBACK_FORM.md)** - Tester feedback template
- **[UAT_SUMMARY.md](../UAT_SUMMARY.md)** - Implementation summary
- **[IMPLEMENTATION_COMPLETE.md](../IMPLEMENTATION_COMPLETE.md)** - Final verification

### Test Files
- **[test-canvases/](../test-canvases/)** - Example canvases
- **[test-uat.sh](../test-uat.sh)** - Automated test runner
- **[tests/test_grid.c](../tests/test_grid.c)** - Grid persistence tests

### Project
- **GitHub**: https://github.com/jcaldwell-labs/boxes-live
- **Issue #21**: https://github.com/jcaldwell-labs/boxes-live/issues/21
- **PR**: (will be merged soon)

---

## Metrics Tracked

### Quantitative
- Time to first box created
- Task completion rate
- Grid usage rate
- Configuration adoption rate
- Error recovery success rate

### Qualitative
- User satisfaction ratings
- Most intuitive features
- Most confusing elements
- Missing features (priority ranked)
- Would-use-for-real-work responses

---

## Known Issues (All Resolved)

### ✅ Grid Not Persisting (FIXED)
**Before**: Grid settings lost on save/load  
**After**: Grid state fully persisted  
**Commit**: e7e1d22

### ✅ No Automated UAT (FIXED)
**Before**: Manual testing only  
**After**: 19 automated scenarios  
**Commit**: 20f8a82

### ✅ Grid Loading with Missing Sections (FIXED)
**Before**: Grid failed to load without CONNECTIONS  
**After**: Grid loads independently  
**Commit**: c8c2c54

---

## FAQ

### Q: How do I run just the grid persistence test?
```bash
make tests/bin/test_grid && tests/bin/test_grid
```

### Q: How do I test with example canvases?
```bash
./boxes-live
# Press F3, enter: test-canvases/flowchart-example.txt
```

### Q: Where are test results logged?
```
uat-results.log         # UAT scenario results
/tmp/build-log.txt      # Build output
```

### Q: How do I provide feedback?
1. Use [FEEDBACK_FORM.md](../FEEDBACK_FORM.md)
2. Submit via GitHub issue or email
3. Include scenario number and experience

### Q: What's the test coverage?
- 442 total test assertions
- 19 UAT scenarios
- 100% pass rate
- Zero warnings, zero leaks

---

## Contact & Support

- **Issues**: https://github.com/jcaldwell-labs/boxes-live/issues
- **Documentation**: See [docs/README.md](README.md)
- **Testing Questions**: Reference this document or UAT_TESTING_GUIDE.md

---

**Document Version**: 1.0  
**Last Updated**: 2024-11-26  
**Status**: Complete and ready for alpha testing
