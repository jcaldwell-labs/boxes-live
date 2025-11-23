# Contributing to boxes-live

Thank you for your interest in contributing to boxes-live! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Building and Testing](#building-and-testing)
- [Code Style](#code-style)
- [Submitting Changes](#submitting-changes)
- [Security Vulnerabilities](#security-vulnerabilities)
- [Architecture Overview](#architecture-overview)

## Code of Conduct

This project adheres to a simple code of conduct: Be respectful, constructive, and professional in all interactions. We welcome contributions from everyone who shares our goal of creating high-quality, maintainable software.

## Getting Started

### Prerequisites

Before you begin, ensure you have the following installed:

- **GCC** or **Clang** compiler (C99 compatible)
- **GNU Make** (for building)
- **ncurses library** development files
  - Debian/Ubuntu: `sudo apt-get install libncurses-dev`
  - Fedora/RHEL: `sudo dnf install ncurses-devel`
  - macOS: `brew install ncurses`
- **Git** (for version control)
- **Valgrind** (optional, for memory leak detection)
- **GDB** (optional, for debugging)

### Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork locally:
   ```bash
   git clone https://github.com/YOUR_USERNAME/boxes-live.git
   cd boxes-live
   ```
3. Add the upstream repository:
   ```bash
   git remote add upstream https://github.com/jcaldwell-labs/boxes-live.git
   ```

## Development Setup

### Building the Project

```bash
# Clean build from scratch
make clean
make

# Run the application
./boxes-live

# Or build and run in one command
make run
```

### Development Build Options

The Makefile supports different build configurations:

```bash
# Standard build (with -Wall -Wextra -Werror)
make

# Debug build (with -g -O0)
make debug

# Release build (with -O2)
make release
```

## Building and Testing

### Running Tests

The project includes comprehensive automated tests (245 assertions across 32 test cases):

```bash
# Run all tests
make test

# Run specific test suites
make test_canvas        # Canvas unit tests
make test_viewport      # Viewport unit tests
make test_persistence   # Persistence tests
make test_integration   # Integration workflow tests
```

### Memory Leak Detection

Always run valgrind before submitting changes:

```bash
# Run tests with valgrind
valgrind --leak-check=full --error-exitcode=1 ./tests/bin/test_canvas
valgrind --leak-check=full --error-exitcode=1 ./tests/bin/test_viewport
valgrind --leak-check=full --error-exitcode=1 ./tests/bin/test_persistence
valgrind --leak-check=full --error-exitcode=1 ./tests/bin/test_integration
```

**All tests must pass with zero memory leaks before submitting a pull request.**

## Code Style

### C Code Conventions

The project follows these C coding standards:

- **Standard**: GNU99 (C99 with GNU extensions for POSIX compatibility)
- **Naming Conventions**:
  - Functions and variables: `snake_case`
  - Macros and constants: `UPPER_CASE`
  - Type names: `PascalCase` (e.g., `Canvas`, `Viewport`, `Box`)
- **Indentation**: 4 spaces (no tabs)
- **Line Length**: 100 characters maximum (soft limit)
- **Braces**: K&R style (opening brace on same line, except for functions)

### Example Code Style

```c
/* Function declarations with clear documentation */
int canvas_add_box(Canvas *canvas, double x, double y,
                   int width, int height, const char *title);

/* Function definitions */
int canvas_add_box(Canvas *canvas, double x, double y,
                   int width, int height, const char *title) {
    if (canvas_ensure_capacity(canvas) != 0) {
        return -1;
    }

    Box *box = &canvas->boxes[canvas->box_count];
    box->x = x;
    box->y = y;
    box->width = width;
    box->height = height;
    box->title = title ? strdup(title) : NULL;

    canvas->box_count++;
    return box->id;
}
```

### Code Quality Requirements

- **Compiler Warnings**: Code must compile with `-Wall -Wextra -Werror` with zero warnings
- **Memory Safety**: All dynamically allocated memory must be properly freed
- **Error Handling**: Check return values and handle errors gracefully
- **Comments**: Add comments for complex logic, especially coordinate transformations
- **Modularity**: Keep functions focused on a single responsibility

### Code Formatting

We recommend using `clang-format` with the project's style:

```bash
# Format a single file
clang-format -i src/canvas.c

# Format all C source files
find src include -name "*.c" -o -name "*.h" | xargs clang-format -i
```

## Submitting Changes

### Before Submitting a Pull Request

Ensure you have completed the following checklist:

- [ ] All tests pass (`make test`)
- [ ] Zero compiler warnings (`make clean && make`)
- [ ] No memory leaks (valgrind on all test binaries)
- [ ] Code follows project style guidelines
- [ ] New features have corresponding tests
- [ ] Documentation is updated (README.md, CLAUDE.md, etc.)
- [ ] Commit messages are clear and descriptive

### Pull Request Process

1. **Create a Feature Branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

2. **Make Your Changes**
   - Write clean, well-documented code
   - Add tests for new functionality
   - Update documentation as needed

3. **Test Thoroughly**
   ```bash
   make clean
   make test
   # Run valgrind on all test binaries
   ```

4. **Commit Your Changes**
   ```bash
   git add .
   git commit -m "Brief description of changes

   More detailed explanation if needed.
   - Bullet point 1
   - Bullet point 2

   Fixes #123"
   ```

5. **Push to Your Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

6. **Create a Pull Request**
   - Go to GitHub and create a pull request from your fork
   - Fill out the PR template with:
     - Clear description of changes
     - Test results (all tests passed, no memory leaks)
     - Screenshots/demos if applicable
     - Related issue numbers

### Commit Message Guidelines

- Use present tense ("Add feature" not "Added feature")
- Use imperative mood ("Move cursor to..." not "Moves cursor to...")
- Limit first line to 72 characters
- Reference issues and pull requests liberally
- Provide context in the commit body for non-trivial changes

Example commit message:
```
Fix memory leak in canvas_load function

The canvas_load function was calling canvas_init without first
cleaning up any existing canvas data, causing memory leaks when
loading multiple times. Added canvas_cleanup call before reinit
with NULL pointer check for safety.

Fixes #42
```

## Security Vulnerabilities

### Reporting Security Issues

If you discover a security vulnerability, please do NOT open a public issue. Instead, email the maintainers directly at:

**security@jcaldwell-labs.com** (or open a private security advisory on GitHub)

Include:
- Description of the vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if you have one)

### Security Guidelines for Contributors

When contributing code, pay special attention to:

- **Buffer Overflows**: Always check array bounds
- **String Safety**: Use `strncpy`, `snprintf` instead of unsafe variants
- **Input Validation**: Validate all user input and file data
- **Integer Overflow**: Check for overflow in arithmetic operations
- **Format String Vulnerabilities**: Never use user input as format strings
- **Memory Safety**: Avoid use-after-free and double-free bugs

## Architecture Overview

### Module Organization

```
boxes-live/
├── include/              # Header files
│   ├── types.h          # Core data structures (Box, Canvas, Viewport)
│   ├── canvas.h         # Canvas management
│   ├── viewport.h       # Camera and coordinate transformations
│   ├── render.h         # Box rendering
│   ├── input.h          # Input handling
│   ├── terminal.h       # Terminal initialization/control
│   ├── persistence.h    # Save/load functionality
│   └── signal_handler.h # Signal handling for reload
├── src/                 # Implementation files
│   ├── main.c          # Main loop and initialization
│   ├── canvas.c        # Dynamic box array management
│   ├── viewport.c      # Viewport transformations
│   ├── render.c        # ncurses-based rendering
│   ├── input.c         # Keyboard and mouse input
│   ├── terminal.c      # Terminal setup/cleanup
│   ├── persistence.c   # File I/O for canvas state
│   └── signal_handler.c # Signal handlers
└── tests/              # Test suites
    ├── test_canvas.c
    ├── test_viewport.c
    ├── test_persistence.c
    └── test_integration.c
```

### Key Concepts

- **Canvas**: Manages dynamic array of boxes in world coordinates
- **Viewport**: Handles camera position and coordinate transformations
- **Rendering**: Draws boxes using ncurses ACS characters
- **Coordinate Systems**: Separation between world and screen coordinates
- **Persistence**: Text-based file format for saving/loading canvas state

See [CLAUDE.md](CLAUDE.md) for detailed architecture documentation.

## Questions?

If you have questions about contributing, feel free to:

- Open an issue with the `question` label
- Check existing documentation in CLAUDE.md and README.md
- Review the TESTING.md file for testing guidelines
- Look at recent pull requests for examples

Thank you for contributing to boxes-live!
