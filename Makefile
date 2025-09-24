# Makefile for MyShell Project
# Author: Mahmoud Kassem (Person A)
# Course: Operating Systems - Fall 2025

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -pedantic
LDFLAGS = 

# Target executable
TARGET = myshell

# Source files
SOURCES = myshell.c shell_utils.c
HEADERS = shell_utils.h

# Object files (automatically generated from source files)
OBJECTS = $(SOURCES:.c=.o)

# Default target - build the executable
all: $(TARGET)

# Build the main executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful!"

# Compile source files to object files
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up generated files
clean:
	@echo "Cleaning up..."
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.txt *.log  # Clean up test files
	@echo "Clean complete!"

# Rebuild everything from scratch
rebuild: clean all

# Install (copy to a system directory) - optional
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/$(TARGET)
	chmod +x /usr/local/bin/$(TARGET)

# Uninstall - optional
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# Run basic tests
test: $(TARGET)
	@echo "Running basic tests..."
	@echo "Test 1: Simple command (ls)"
	./$(TARGET) <<< "ls"
	@echo ""
	@echo "Test 2: Command with arguments (ls -l)"
	./$(TARGET) <<< "ls -l"
	@echo ""
	@echo "Test 3: Output redirection"
	./$(TARGET) <<< "echo 'Hello World' > test_output.txt"
	@cat test_output.txt 2>/dev/null || echo "Output redirection test failed"
	@echo ""
	@echo "Test 4: Exit command"
	./$(TARGET) <<< "exit"
	@echo "Basic tests completed!"

# Debug build with extra debugging symbols
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release build with optimizations
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Check for memory leaks using valgrind (if available)
memcheck: $(TARGET)
	@echo "Running memory leak check..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) <<< "exit"

# Display help information
help:
	@echo "Available targets:"
	@echo "  all       - Build the myshell executable (default)"
	@echo "  clean     - Remove generated files"
	@echo "  rebuild   - Clean and build from scratch"
	@echo "  test      - Run basic functionality tests"
	@echo "  debug     - Build with debugging symbols"
	@echo "  release   - Build optimized release version"
	@echo "  memcheck  - Check for memory leaks with valgrind"
	@echo "  install   - Install to system directory"
	@echo "  uninstall - Remove from system directory"
	@echo "  help      - Show this help message"

# Declare phony targets (targets that don't create files)
.PHONY: all clean rebuild install uninstall test debug release memcheck help

# Prevent make from deleting intermediate files
.PRECIOUS: %.o
