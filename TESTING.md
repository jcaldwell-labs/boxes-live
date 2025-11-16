# Testing Strategy and Guide

## Overview

This document describes the comprehensive testing strategy for boxes-live. Even though boxes-live is an interactive terminal application, we've implemented automated testing to verify functionality without requiring manual interaction.

## Test Framework

We use a custom lightweight C test framework (`tests/test.h`) that provides:

- **TEST_START()** - Initialize test counters
- **TEST(name)** - Define a test case
- **ASSERT(condition, message)** - Basic assertion
- **ASSERT_EQ(actual, expected, message)** - Equality assertion
- **ASSERT_STR_EQ(actual, expected, message)** - String equality
- **ASSERT_NEAR(actual, expected, epsilon, message)** - Floating point comparison
- **ASSERT_NULL(ptr, message)** - Null pointer check
- **ASSERT_NOT_NULL(ptr, message)** - Non-null pointer check
- **TEST_END()** - Print summary and return exit code

This framework is designed for:
- Zero external dependencies (pure C)
- Headless execution (no terminal interaction required)
- Clear pass/fail output with detailed messages
- Exit codes suitable for CI/CD integration

## Test Suites

### 1. Unit Tests: Canvas (`tests/test_canvas.c`)

Tests the core canvas data structure and operations:

- **Canvas Initialization**: Verifies proper initial state (capacity, IDs, selection)
- **Adding Boxes**: Tests box creation, ID assignment, position storage
- **Box Content**: Validates multi-line content management
- **Removing Boxes**: Checks array compaction and ID persistence
- **Finding Boxes**: Tests coordinate-based hit detection
- **Selection System**: Verifies selection flags and state tracking
- **Dynamic Growth**: Tests array reallocation beyond initial capacity
- **Color Property**: Validates color assignment and storage

**Coverage**: 70 assertions across 8 test cases

### 2. Unit Tests: Viewport (`tests/test_viewport.c`)

Tests the viewport and coordinate transformation system:

- **Viewport Initialization**: Checks default camera and zoom values
- **World-to-Screen Transform**: Tests coordinate conversion with various pan/zoom states
- **Screen-to-World Transform**: Validates reverse coordinate conversion
- **Roundtrip Accuracy**: Verifies transformation precision
- **Panning**: Tests camera movement with and without zoom
- **Zooming**: Validates zoom operations and clamping (0.1x to 10.0x)
- **Visibility Testing**: Checks bounds detection for viewport culling

**Coverage**: 39 assertions across 11 test cases

### 3. Snapshot Tests: Persistence (`tests/test_persistence.c`)

Tests save/load functionality with expected vs actual verification:

- **Save to File**: Verifies file creation and format
- **Load from File**: Validates data restoration from saved files
- **Roundtrip Preservation**: Tests save→load→verify equality
- **Error Handling**: Validates graceful failure on missing files
- **Empty Canvas**: Tests edge case of empty canvas save/load
- **Color Preservation**: Verifies all 8 colors save/load correctly

**Coverage**: 69 assertions across 6 test cases

**Snapshot Strategy**:
- Uses temporary files for test isolation
- Compares loaded data with original structures
- Helper function `files_equal()` available for binary file comparison
- Cleans up test artifacts after execution

### 4. Integration Tests: Workflows (`tests/test_integration.c`)

Tests complete user workflows end-to-end:

- **Complete Workflow**: Create boxes → Add content → Apply colors → Move boxes → Save → Load → Verify
- **Selection Cycling**: Tests Tab key simulation for box navigation
- **Deletion Workflow**: Validates box removal and selection updates
- **Viewport Integration**: Tests visibility with viewport panning
- **Coordinate Interaction**: Simulates mouse click hit detection
- **Dynamic Growth**: Tests session with >20 boxes including save/load

**Coverage**: 67 assertions across 6 test cases

**Integration Approach**:
- Tests multiple modules together (canvas + viewport + persistence)
- Simulates real user interactions programmatically
- Validates state consistency across operations
- Includes save/load roundtrip verification

## Running Tests

### Run All Tests

```bash
make test
```

This will:
1. Build all test executables
2. Run each test suite in sequence
3. Display detailed pass/fail output
4. Exit with code 0 if all pass, 1 if any fail

### Run Individual Test Suites

```bash
make test_canvas       # Canvas unit tests
make test_viewport     # Viewport unit tests
make test_persistence  # Persistence snapshot tests
make test_integration  # Integration workflow tests
```

### Clean Test Artifacts

```bash
make clean            # Removes test binaries and temp files
```

## Test Output Format

Tests produce detailed output showing:

```
[TEST] Test name
  ✓ Assertion passed message
  ✗ Assertion failed message (if any)

=== Test Summary ===
Passed: 70
Failed: 0
```

Exit codes:
- `0` - All tests passed
- `1` - One or more tests failed

## Headless Testing

All tests are designed to run headless without requiring:
- Terminal interaction
- ncurses initialization (for unit tests)
- User input
- Display/graphics

**Key Design Decisions**:

1. **Isolated Unit Tests**: Canvas and viewport tests don't call ncurses functions
2. **Mock-Free Approach**: Tests use actual implementation code, not mocks
3. **Temporary Files**: Persistence tests create/clean up temp files automatically
4. **Deterministic**: No random data, all tests produce consistent results

## Snapshot Testing Strategy

For persistence testing, we use a snapshot approach:

1. **Create Known State**: Build canvas with specific data
2. **Save to File**: Write canvas to disk
3. **Load from File**: Read canvas back
4. **Verify Equality**: Compare all properties match original
5. **Clean Up**: Remove temporary test files

This validates:
- File format correctness
- Data serialization accuracy
- Deserialization completeness
- Roundtrip preservation

## Coverage Summary

| Module | Test Suite | Assertions | Test Cases |
|--------|-----------|------------|------------|
| Canvas | test_canvas.c | 70 | 8 |
| Viewport | test_viewport.c | 39 | 11 |
| Persistence | test_persistence.c | 69 | 6 |
| Integration | test_integration.c | 67 | 6 |
| **Total** | **4 suites** | **245** | **31** |

## CI/CD Integration

Tests are suitable for continuous integration:

```bash
#!/bin/bash
# Example CI script
make clean
make test
if [ $? -eq 0 ]; then
    echo "All tests passed"
    exit 0
else
    echo "Tests failed"
    exit 1
fi
```

The test target is designed to:
- Fail fast (stop on first failure)
- Provide clear error messages
- Return proper exit codes
- Clean up after itself

## Adding New Tests

### 1. Create Test File

```c
#include <math.h>
#include "test.h"
#include "../include/your_module.h"

int main(void) {
    TEST_START();

    TEST("Your test name") {
        // Test code here
        ASSERT_EQ(actual, expected, "Description");
    }

    TEST_END();
}
```

### 2. Add to Makefile

Tests are automatically discovered by pattern matching `tests/test_*.c`.

No Makefile changes needed - just create `tests/test_yourmodule.c` and run `make test`.

### 3. Follow Best Practices

- **Descriptive Names**: Use clear test and assertion descriptions
- **Isolation**: Each test should be independent
- **Cleanup**: Free allocated memory, close files
- **Edge Cases**: Test boundaries, empty inputs, error conditions
- **Documentation**: Comment complex test logic

## Test-Driven Development Workflow

For new features:

1. **Write Tests First**: Define expected behavior in tests
2. **Run Tests**: Verify they fail (no implementation yet)
3. **Implement Feature**: Write minimal code to pass tests
4. **Refactor**: Improve code while keeping tests green
5. **Document**: Update this file with new test coverage

## Known Limitations

1. **No ncurses Mocking**: Integration tests don't test rendering code (requires terminal)
2. **Mouse Events**: Mouse drag is tested at the coordinate level, not event level
3. **Terminal Resize**: Not tested (requires actual terminal)
4. **Keyboard Input**: Not tested (requires user interaction)

These limitations are acceptable because:
- Core logic (canvas, viewport, persistence) is fully tested
- ncurses is a well-tested external library
- Manual testing verifies interactive features

## Future Test Enhancements

Potential improvements:

1. **Connection Lines**: When implemented, test line creation/persistence
2. **Text Editing**: Test box content editing workflows
3. **Box Resizing**: Test dimension changes and collision detection
4. **Performance**: Add benchmarking tests for large canvases (1000+ boxes)
5. **Fuzzing**: Random input testing for crash detection
6. **Memory**: Valgrind integration for leak detection

## Debugging Failed Tests

When a test fails:

1. **Read the Message**: Assertion messages show expected vs actual
2. **Run Individual Suite**: `make test_canvas` to isolate failures
3. **Add Debug Output**: Use printf in test code to inspect state
4. **Use GDB**: `gdb tests/bin/test_canvas` for deep debugging
5. **Check Cleanup**: Verify previous tests cleaned up properly

## Conclusion

This testing strategy provides:

- ✅ **Automated Verification**: No manual testing required for core features
- ✅ **Fast Feedback**: Tests run in <1 second total
- ✅ **Headless Execution**: Suitable for CI/CD pipelines
- ✅ **Comprehensive Coverage**: 245 assertions across 31 test cases
- ✅ **Regression Protection**: Catch bugs before they reach users
- ✅ **Documentation**: Tests serve as executable specifications

The test suite validates that boxes-live maintains correctness as new features are added, ensuring a stable foundation for future development.
