CC = gcc
CFLAGS = -Wall -Wextra -Werror -Iinclude -std=gnu99
LDFLAGS = -lncurses -lm
TARGET = boxes-live
SRCDIR = src
INCDIR = include
OBJDIR = obj
TESTDIR = tests
TESTBINDIR = $(TESTDIR)/bin

# Main application sources
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Library sources (everything except main.c)
LIB_SOURCES = $(filter-out $(SRCDIR)/main.c,$(SOURCES))
LIB_OBJECTS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(LIB_SOURCES))

# Test sources and executables
TEST_SOURCES = $(wildcard $(TESTDIR)/test_*.c)
TEST_BINS = $(patsubst $(TESTDIR)/test_%.c,$(TESTBINDIR)/test_%,$(TEST_SOURCES))

.PHONY: all clean run test

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

clean:
	rm -rf $(OBJDIR) $(TARGET) $(TESTBINDIR)

run: $(TARGET)
	./$(TARGET)
