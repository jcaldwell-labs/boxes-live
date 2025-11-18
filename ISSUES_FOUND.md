# Issues Found in Demos and Examples

## Critical Issues (Blockers for 1.0.0)

### 1. Test Suite Compilation Failure ❌
**Location**: `make test`
**Severity**: CRITICAL
**Status**: BLOCKING

**Problem**: Tests fail to compile with linker error:
```
undefined reference to `get_current_file'
```

**Root Cause**:
- `get_current_file()` is defined in `src/main.c:139`
- `src/input.c` uses `get_current_file()` (line 255)
- Tests link against `input.o` but NOT `main.o` (Makefile line 16)
- This creates an unresolved symbol

**Impact**:
- Cannot run any automated tests
- No way to verify code changes
- Blocks CI/CD integration
- Risk of regressions

**Solution Options**:
1. Move `get_current_file()` to a shared module (canvas.c or new app_state.c)
2. Make input.c take current_file as parameter instead of calling extern function
3. Create stub implementations for tests

**Recommended**: Option 2 - pass current_file as parameter to avoid global state coupling

---

### 2. Demo Scripts Path Issues ❌
**Location**: `demos/*.sh` and `examples/*.sh`
**Severity**: HIGH
**Status**: BLOCKING

**Problem**: Scripts have inconsistent assumptions about working directory:

#### 2a. demos/ scripts assume running FROM demos/
```bash
# demos/live-monitor.sh line 33:
../connectors/boxes-cli create ...
```
Works: `cd demos && ./live-monitor.sh`
Fails: `./demos/live-monitor.sh` (from project root)

#### 2b. examples/ scripts assume running FROM project root
```bash
# examples/cli_demo.sh line 9:
CLI="./connectors/boxes-cli"
```
Works: `./examples/cli_demo.sh` (from project root)
Fails: `cd examples && ./cli_demo.sh`

**Impact**:
- Users get "command not found" errors
- README instructions ambiguous about working directory
- Inconsistent user experience

**Solution**: Make all scripts work from ANY directory using:
```bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLI="$PROJECT_ROOT/connectors/boxes-cli"
```

---

## High Priority Issues

### 3. Missing Test Documentation 📋
**Location**: README.md, TESTING.md
**Severity**: MEDIUM

**Problem**:
- Makefile has `test` target (line 44-56)
- README doesn't mention how to run tests
- TESTING.md exists but doesn't document `make test`

**Impact**: Developers don't know tests exist or how to run them

**Solution**: Add test running instructions to README

---

### 4. No Automated Demo Verification ❌
**Location**: demos/, examples/
**Severity**: MEDIUM

**Problem**:
- Demos and examples have no automated tests
- No way to verify they work without manual testing
- Changes can break demos silently

**Impact**: Untested assumptions stack up faster than visible bugs

**Solution**: Create automated test scripts:
- Syntax validation: `bash -n script.sh`
- Dry-run tests: Test scripts with mock data
- Integration tests: Full end-to-end demo execution

---

## Medium Priority Issues

### 5. Incomplete Gherkin/BDD Test Coverage 📋
**Location**: tests/
**Severity**: MEDIUM

**Problem**:
- No behavior-driven development (BDD) specs
- No Gherkin/Given-When-Then user stories
- Tests are unit tests only, no acceptance tests

**Impact**:
- No clear acceptance criteria
- Difficult to verify features match requirements
- No executable specifications

**Solution**: Create `tests/acceptance/` with Gherkin scenarios

---

### 6. Canvas File Format Not Validated 📋
**Location**: src/persistence.c
**Severity**: MEDIUM

**Problem**:
- Parser doesn't validate canvas file format thoroughly
- Malformed files may cause crashes or undefined behavior
- No schema validation

**Impact**: Users can create corrupt canvas files

**Solution**: Add format validation and better error messages

---

## Low Priority Issues

### 7. Missing Examples Output 📋
**Location**: examples/
**Severity**: LOW

**Problem**: Generated example files (.md, .json, .csv) are gitignored but not shown in docs

**Solution**: Add sample outputs to examples/README.md

---

### 8. Demos Require Manual Interaction 📋
**Location**: demos/
**Severity**: LOW

**Problem**: Live demos require manual F3 press to reload

**Solution**: Document workaround, wait for plugin system for true auto-reload

---

## Summary Statistics

- **Critical Issues**: 2
- **High Priority**: 2
- **Medium Priority**: 2
- **Low Priority**: 2
- **Total Issues**: 8
- **Blocking 1.0.0**: 4

## Recommendations for Pre-Release

### Must Fix for 1.0.0-rc1:
1. ✅ Fix test compilation (get_current_file issue)
2. ✅ Fix demo/example script paths
3. ✅ Add test documentation
4. ✅ Create automated demo verification

### Should Fix:
5. Add Gherkin acceptance tests
6. Improve canvas format validation

### Nice to Have:
7. Add example outputs to docs
8. Document manual interaction requirement

---

## Next Steps

1. Fix all critical issues
2. Create comprehensive Gherkin user stories
3. Build verification framework for smaller pieces
4. Run full test suite
5. Create pre-release tag off main
