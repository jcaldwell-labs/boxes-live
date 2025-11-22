# Contributing to boxes-live

Thank you for your interest in contributing to boxes-live! This document provides guidelines for contributing to this project.

## Project Vision

boxes-live is a Unix-style terminal application that brings interactive visual workspace capabilities (similar to Miro boards) to the terminal. We aim to:

- Maintain Unix philosophy: do one thing well
- Keep it terminal-first (no GUI dependencies)
- Work over SSH and in tmux/screen
- Provide scriptable interfaces and connectors
- Support rich integrations with other terminal tools

## Getting Started

### Prerequisites

- GCC compiler
- ncurses library (`libncurses-dev` on Debian/Ubuntu)
- Terminal with box-drawing character support
- Basic knowledge of C programming
- Familiarity with ncurses is helpful but not required

### Setting Up Development Environment

```bash
# Clone the repository
git clone https://github.com/jcaldwell-labs/boxes-live.git
cd boxes-live

# Build the project
make

# Run tests
make test

# Run the application
./boxes-live
```

## How to Contribute

### Reporting Bugs

Before submitting a bug report:

1. Check existing issues to avoid duplicates
2. Use the bug report template
3. Include:
   - Operating system and version
   - Terminal emulator and version
   - Steps to reproduce
   - Expected vs. actual behavior
   - Screenshots if applicable (use `script` command to capture terminal output)

### Suggesting Enhancements

Enhancement suggestions are welcome! Please:

1. Check existing feature requests
2. Describe the use case clearly
3. Explain how it aligns with the project vision
4. Consider if it could be implemented as a connector instead

### Pull Requests

#### Before You Start

1. Check existing PRs to avoid duplicate work
2. For large changes, open an issue first to discuss
3. Read the code style guide below

#### PR Process

1. **Fork and branch**: Create a feature branch from `master`
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make your changes**:
   - Write clear, focused commits
   - Add tests for new functionality
   - Update documentation as needed
   - Ensure `make` builds without warnings
   - Ensure `make test` passes

3. **Test thoroughly**:
   ```bash
   # Build and test
   make clean && make
   make test

   # Test in actual terminal
   ./boxes-live

   # Test connectors if applicable
   ./tests/connector_integration_test.sh
   ```

4. **Commit guidelines**:
   - Use conventional commits format
   - Examples:
     - `feat: add box rotation support`
     - `fix: correct viewport pan calculation`
     - `docs: improve connector guide`
     - `test: add canvas persistence tests`
     - `refactor: simplify render loop`

5. **Submit PR**:
   - Fill out the PR template
   - Reference related issues
   - Describe what changed and why
   - Add screenshots/recordings for visual changes

6. **Code review**:
   - Address review feedback promptly
   - Keep PR scope focused
   - Be open to suggestions

## Code Style Guide

### C Code Standards

- **Standard**: GNU99 (for POSIX extensions like `strdup`)
- **Indentation**: 4 spaces (no tabs)
- **Line length**: Aim for 80-100 characters
- **Naming**:
  - Functions and variables: `snake_case`
  - Macros and constants: `UPPER_CASE`
  - Struct types: `PascalCase` (e.g., `Canvas`, `Viewport`)

### Code Organization

- **Header files** (`.h`): Interface declarations only
- **Implementation files** (`.c`): Function definitions
- **Modularity**: Each module handles one concern (terminal, viewport, render, input, etc.)

### Example

```c
// Good
typedef struct {
    int x;
    int y;
    float zoom;
} Viewport;

void viewport_pan(Viewport *vp, int dx, int dy) {
    if (!vp) return;
    vp->x += dx;
    vp->y += dy;
}

// Avoid
typedef struct { int x; int y; float zoom; } viewport; // Too compact
void ViewportPan(viewport *v,int dx,int dy){v->x+=dx;v->y+=dy;} // Bad style
```

### Memory Management

- Always free dynamically allocated memory
- Use `strdup` for string copying, remember to `free`
- Check return values of `malloc/calloc`
- Initialize pointers to NULL

### Error Handling

- Return `-1` or `NULL` on error (where applicable)
- Check return values
- Don't crash on bad input
- Log errors to stderr if needed

### Comments

- Comment complex logic, especially coordinate transformations
- Use `//` for single-line comments
- Use `/* */` for multi-line comments
- Document public API in header files

```c
/**
 * Transform world coordinates to screen coordinates
 * @param vp Viewport containing camera position and zoom
 * @param world_x X coordinate in world space
 * @param world_y Y coordinate in world space
 * @param screen_x Output: X coordinate in screen space
 * @param screen_y Output: Y coordinate in screen space
 */
void world_to_screen(Viewport *vp, int world_x, int world_y,
                     int *screen_x, int *screen_y);
```

## Testing Guidelines

### Unit Tests

- Add tests for new functionality
- Tests go in `tests/test_*.c`
- Use the test framework in `tests/test.h`
- Aim for >80% coverage of critical paths

### Integration Tests

- Shell scripts for connector testing
- Located in `tests/`
- Must pass before PR merge

### Manual Testing

Always test manually in a terminal:

```bash
# Build and run
make clean && make
./boxes-live

# Test:
# - Pan with arrow keys
# - Zoom with +/-
# - Mouse clicks and drags
# - Box creation (N key)
# - Save/load (F2/F3)
# - Signal handling (SIGUSR1/SIGUSR2)
```

## Creating Connectors

Connectors transform data from other sources into canvas format. They are shell scripts or standalone programs.

### Connector Guidelines

1. **Location**: `connectors/` directory
2. **Naming**: `source2canvas` (e.g., `git2canvas`, `docker2canvas`)
3. **Output**: Canvas text format to stdout
4. **Documentation**: Add README section and example
5. **Testing**: Add integration test in `tests/`

### Connector Template

```bash
#!/bin/bash
# connector-name2canvas - Convert X to canvas format

usage() {
    echo "Usage: $0 [options]"
    echo "Convert X data to boxes-live canvas format"
    exit 1
}

# Parse arguments
# Fetch/process data
# Output canvas format

# Canvas format:
# BOX <id> <x> <y> <width> <height> <color>
# TITLE <id> <title>
# CONTENT <id> <line1>
# CONTENT <id> <line2>
```

See existing connectors for examples.

## Documentation

### What to Document

- New features: Update README.md
- Architecture changes: Update CLAUDE.md
- Connectors: Update connectors/README.md
- Workflows: Update docs/WORKFLOW-EXAMPLES.md
- Breaking changes: Update CHANGELOG.md

### Documentation Style

- Clear and concise
- Code examples for APIs
- Screenshots/recordings for visual features
- Link to related documentation

## Release Process

(For maintainers)

1. Update CHANGELOG.md
2. Update version in README.md
3. Tag release: `git tag v1.x.0`
4. Create GitHub release with notes
5. Update documentation if needed

## Community Guidelines

### Code of Conduct

- Be respectful and inclusive
- Provide constructive feedback
- Help others learn
- Focus on the code, not the person

### Getting Help

- Check documentation first (README, CLAUDE.md, docs/)
- Search existing issues
- Ask in discussions or create an issue
- Be specific about your problem

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

## Questions?

- Open an issue for discussion
- Check existing documentation
- Review closed issues and PRs for context

Thank you for contributing to boxes-live! 🎨📦
