# this is the Makefile for MyShell Project

# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -pedantic
LDFLAGS = 

# for the target executable
TARGET = myshell

# source files
SOURCES = myshell.c shell_utils.c
HEADERS = shell_utils.h

# object files -- automatically generated from source files
OBJECTS = $(SOURCES:.c=.o)

# default target -- build the executable
# gets built when running make
all: $(TARGET)

# build the main executable
# links all the object files together into the final program
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "Build successful!"

# compile source files to object files
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# clean up generated files
clean:
	@echo "Cleaning up..."
	rm -f $(OBJECTS) $(TARGET)
	rm -f *.txt *.log
	@echo "Clean complete!"

# rebuild everything from scratch
rebuild: clean all

# Install to make myshell available system-wide
install: $(TARGET)
	cp $(TARGET) /usr/local/bin/$(TARGET)
	chmod +x /usr/local/bin/$(TARGET)

# uninstall to remove myshell from the system directory
uninstall:
	rm -f /usr/local/bin/$(TARGET)

# run basic tests
test: $(TARGET)
	@echo "Running basic tests..."
	@echo "Test 1: Simple command (ls)"
	@echo "ls" | ./$(TARGET)
	@echo ""
	@echo "Test 2: Command with arguments (ls -l)"
	@echo "ls -l" | ./$(TARGET)
	@echo ""
	@echo "Test 3: Output redirection"
	@echo "echo 'Hello World' > test_output.txt" | ./$(TARGET)
	@cat test_output.txt 2>/dev/null || echo "Output redirection test failed"
	@echo ""
	@echo "Test 4: Exit command"
	@echo "exit" | ./$(TARGET)
	@echo "Basic tests completed!"

# debug build with extra debugging symbols
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# release build with optimizations
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# check for memory leaks
memcheck: $(TARGET)
	@echo "Running memory leak check..."
	@echo "exit" | valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# help information
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

# declare phony targets -- i.e don't create files
.PHONY: all clean rebuild install uninstall test debug release memcheck help

# to prevent make from deleting intermediate files
.PRECIOUS: %.o
