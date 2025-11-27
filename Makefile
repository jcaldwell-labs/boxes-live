# Detect OS for cross-platform support
ifeq ($(OS),Windows_NT)
    # Windows detected (Git Bash, MSYS2, etc.)
    # Add Scoop GCC paths if available
    ifneq ($(wildcard $(HOME)/scoop/apps/gcc/current/bin/gcc.exe),)
        CC = $(HOME)/scoop/apps/gcc/current/bin/gcc
        CFLAGS = -Wall -Wextra -Werror -Iinclude -I$(HOME)/scoop/apps/gcc/current/include -std=gnu99
        LDFLAGS = -L$(HOME)/scoop/apps/gcc/current/lib -lncurses -lm
    else
        CC = gcc
        CFLAGS = -Wall -Wextra -Werror -Iinclude -std=gnu99
        LDFLAGS = -lncurses -lm
    endif
    TARGET = boxes-live.exe
else
    # Unix/Linux/MacOS
    CC = gcc
    CFLAGS = -Wall -Wextra -Werror -Iinclude -std=gnu99
    LDFLAGS = -lncurses -lm
    TARGET = boxes-live
endif
SRCDIR = src
INCDIR = include
OBJDIR = obj
TESTDIR = tests
TESTBINDIR = $(TESTDIR)/bin

# Installation directories
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
MANDIR = $(PREFIX)/share/man/man1

# Build configuration (can be overridden: make BUILD=debug)
BUILD ?= standard

# Configuration-specific flags
ifeq ($(BUILD),debug)
    CFLAGS += -g -O0 -DDEBUG
else ifeq ($(BUILD),release)
    CFLAGS += -O2 -DNDEBUG
else
    # standard build (default)
    CFLAGS += -O1
endif

# Main application sources
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Library sources (everything except main.c)
LIB_SOURCES = $(filter-out $(SRCDIR)/main.c,$(SOURCES))
LIB_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(LIB_SOURCES))

# Test sources and executables
TEST_SOURCES = $(wildcard $(TESTDIR)/test_*.c)
TEST_BINS = $(patsubst $(TESTDIR)/test_%.c,$(TESTBINDIR)/test_%,$(TEST_SOURCES))

.PHONY: all clean run test install uninstall debug release valgrind

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Test compilation
$(TESTBINDIR)/test_%: $(TESTDIR)/test_%.c $(LIB_OBJECTS) | $(TESTBINDIR)
	$(CC) $(CFLAGS) $< $(LIB_OBJECTS) -o $@ $(LDFLAGS)

$(TESTBINDIR):
	mkdir -p $(TESTBINDIR)

# Run all tests
test: $(TEST_BINS)
	@echo "Running all tests..."
	@for test in $(TEST_BINS); do \
		echo ""; \
		echo "=========================================="; \
		echo "Running $$test..."; \
		echo "=========================================="; \
		$$test || exit 1; \
	done
	@echo ""
	@echo "=========================================="
	@echo "All tests passed!"
	@echo "=========================================="

# Individual test targets
test_canvas: $(TESTBINDIR)/test_canvas
	$(TESTBINDIR)/test_canvas

test_viewport: $(TESTBINDIR)/test_viewport
	$(TESTBINDIR)/test_viewport

test_persistence: $(TESTBINDIR)/test_persistence
	$(TESTBINDIR)/test_persistence

test_integration: $(TESTBINDIR)/test_integration
	$(TESTBINDIR)/test_integration

# Build with debug symbols
# Note: This builds only the main binary. Tests are NOT built automatically.
# To build and run tests, use 'make test' after building.
debug:
	@echo "Note: 'make debug' only builds the main binary. Tests are NOT built or run automatically."
	@echo "To build and run tests, use 'make test' after building."
	$(MAKE) BUILD=debug all

# Build optimized release version
# Note: This builds only the main binary. Tests are NOT built automatically.
# To build and run tests, use 'make test' after building.
release:
	@echo "Note: 'make release' only builds the main binary. Tests are NOT built or run automatically."
	@echo "To build and run tests, use 'make test' after building."
	$(MAKE) BUILD=release all

# Run valgrind on all tests
# Note: Tests should be rebuilt explicitly before running valgrind with different BUILD configurations.
# Example: make clean && make BUILD=debug test && make valgrind
valgrind: $(TEST_BINS)
	@echo "Running valgrind memory leak detection..."
	@for test in $(TEST_BINS); do \
		echo ""; \
		echo "Checking $$test for memory leaks..."; \
		valgrind --leak-check=full --error-exitcode=1 --quiet $$test || exit 1; \
	done
	@echo ""
	@echo "All tests passed valgrind checks (no memory leaks)"

# Install the application
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	install -d $(BINDIR)
	install -m 755 $(TARGET) $(BINDIR)/$(TARGET)
	@echo "Installation complete. Run with: $(TARGET)"

# Uninstall the application
uninstall:
	@echo "Removing $(TARGET) from $(BINDIR)..."
	rm -f $(BINDIR)/$(TARGET)
	@echo "Uninstallation complete."

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TESTBINDIR)

run: $(TARGET)
	./$(TARGET)
