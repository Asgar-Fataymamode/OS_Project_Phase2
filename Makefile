# # this is the Makefile for MyShell Project

# # compiler and flags
# CC = gcc
# CFLAGS = -Wall -Wextra -std=c99 -g -pedantic
# LDFLAGS = 

# # for the target executable
# TARGET = myshell

# # source files
# SOURCES = myshell.c shell_utils.c
# HEADERS = shell_utils.h

# # object files -- automatically generated from source files
# OBJECTS = $(SOURCES:.c=.o)

# # default target -- build the executable
# # gets built when running make
# all: $(TARGET)

# # build the main executable
# # links all the object files together into the final program
# $(TARGET): $(OBJECTS)
# 	@echo "Linking $(TARGET)..."
# 	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
# 	@echo "Build successful!"

# # compile source files to object files
# %.o: %.c $(HEADERS)
# 	@echo "Compiling $<..."
# 	$(CC) $(CFLAGS) -c $< -o $@

# # clean up generated files
# clean:
# 	@echo "Cleaning up..."
# 	rm -f $(OBJECTS) $(TARGET)
# 	rm -f *.txt *.log
# 	@echo "Clean complete!"

# # rebuild everything from scratch
# rebuild: clean all

# # Install to make myshell available system-wide
# install: $(TARGET)
# 	cp $(TARGET) /usr/local/bin/$(TARGET)
# 	chmod +x /usr/local/bin/$(TARGET)

# # uninstall to remove myshell from the system directory
# uninstall:
# 	rm -f /usr/local/bin/$(TARGET)

# # run basic tests
# test: $(TARGET)
# 	@echo "Running basic tests..."
# 	@echo "Test 1: Simple command (ls)"
# 	@echo "ls" | ./$(TARGET)
# 	@echo ""
# 	@echo "Test 2: Command with arguments (ls -l)"
# 	@echo "ls -l" | ./$(TARGET)
# 	@echo ""
# 	@echo "Test 3: Output redirection"
# 	@echo "echo 'Hello World' > test_output.txt" | ./$(TARGET)
# 	@cat test_output.txt 2>/dev/null || echo "Output redirection test failed"
# 	@echo ""
# 	@echo "Test 4: Exit command"
# 	@echo "exit" | ./$(TARGET)
# 	@echo "Basic tests completed!"

# # debug build with extra debugging symbols
# debug: CFLAGS += -DDEBUG -O0
# debug: $(TARGET)

# # release build with optimizations
# release: CFLAGS += -O2 -DNDEBUG
# release: clean $(TARGET)

# # check for memory leaks
# memcheck: $(TARGET)
# 	@echo "Running memory leak check..."
# 	@echo "exit" | valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

# # help information
# help:
# 	@echo "Available targets:"
# 	@echo "  all       - Build the myshell executable (default)"
# 	@echo "  clean     - Remove generated files"
# 	@echo "  rebuild   - Clean and build from scratch"
# 	@echo "  test      - Run basic functionality tests"
# 	@echo "  debug     - Build with debugging symbols"
# 	@echo "  release   - Build optimized release version"
# 	@echo "  memcheck  - Check for memory leaks with valgrind"
# 	@echo "  install   - Install to system directory"
# 	@echo "  uninstall - Remove from system directory"
# 	@echo "  help      - Show this help message"

# # declare phony targets -- i.e don't create files
# .PHONY: all clean rebuild install uninstall test debug release memcheck help

# # to prevent make from deleting intermediate files
# .PRECIOUS: %.o

















# the code above is makefile for phase 1
# this is the new makefile

# compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -pedantic
LDFLAGS =

# target executables
TARGET_SHELL = myshell      # Phase 1 local shell
TARGET_SERVER = server      # Phase 2 server
TARGET_CLIENT = client      # Phase 2 client (will be implemented by Person B)

# source files
# Phase 1 shell sources
SHELL_SOURCES = myshell.c shell_utils.c

# Phase 2 server sources (reuses shell_utils.c from Phase 1)
SERVER_SOURCES = server.c shell_utils.c

# Phase 2 client sources (standalone, no Phase 1 dependency)
CLIENT_SOURCES = client.c

# header files
HEADERS = shell_utils.h

# Object files are automatically generated from source files
SHELL_OBJECTS = $(SHELL_SOURCES:.c=.o)
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)

# Build everything by default (shell, server, and client)
all: $(TARGET_SHELL) $(TARGET_SERVER) $(TARGET_CLIENT)

# Note: All three executables are now built by default (Phase 1 shell, Phase 2 server, Phase 2 client)

# build targets
# Build the original Phase 1 myshell
$(TARGET_SHELL): $(SHELL_OBJECTS)
	@echo "Linking $(TARGET_SHELL)..."
	$(CC) $(SHELL_OBJECTS) -o $(TARGET_SHELL) $(LDFLAGS)
	@echo "Build successful: $(TARGET_SHELL)"

# Build the Phase 2 server
$(TARGET_SERVER): $(SERVER_OBJECTS)
	@echo "Linking $(TARGET_SERVER)..."
	$(CC) $(SERVER_OBJECTS) -o $(TARGET_SERVER) $(LDFLAGS)
	@echo "Build successful: $(TARGET_SERVER)"

# Build the Phase 2 client (to be implemented by Person B)
$(TARGET_CLIENT): $(CLIENT_OBJECTS)
	@echo "Linking $(TARGET_CLIENT)..."
	$(CC) $(CLIENT_OBJECTS) -o $(TARGET_CLIENT) $(LDFLAGS)
	@echo "Build successful: $(TARGET_CLIENT)"

# compilation rules
%.o: %.c $(HEADERS)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@


# individual build targets
# Build only the server
server: $(TARGET_SERVER)

# Build only the client
client: $(TARGET_CLIENT)

# Build only the Phase 1 shell
myshell: $(TARGET_SHELL)

# utility targets
# clean up all generated files (object files and executables)
clean:
	@echo "Cleaning up..."
	rm -f $(SHELL_OBJECTS) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
	rm -f $(TARGET_SHELL) $(TARGET_SERVER) $(TARGET_CLIENT)
	rm -f *.txt *.log
	@echo "Clean complete!"

# rebuild everything from scratch (clean then build)
rebuild: clean all

# testing targets
# run server (starts server on port 8080)
run-server: $(TARGET_SERVER)
	@echo "Starting server on port 8080..."
	@echo "To test: Use 'telnet localhost 8080' or 'nc localhost 8080' in another terminal"
	@echo "Or wait for Person B to implement the client"
	./$(TARGET_SERVER)

# run client (to be used after Person B implements it)
run-client: $(TARGET_CLIENT)
	@echo "Starting client..."
	./$(TARGET_CLIENT)

# test Phase 1 shell
test-shell: $(TARGET_SHELL)
	@echo "Running Phase 1 shell..."
	./$(TARGET_SHELL)

# help target
help:
	@echo "Available targets:"
	@echo "  all          - Build myshell and server (default)"
	@echo "  server       - Build only the Phase 2 server"
	@echo "  client       - Build only the Phase 2 client"
	@echo "  myshell      - Build only the Phase 1 shell"
	@echo "  clean        - Remove all generated files"
	@echo "  rebuild      - Clean and build from scratch"
	@echo "  run-server   - Build and run the server"
	@echo "  run-client   - Build and run the client"
	@echo "  test-shell   - Build and run Phase 1 shell"
	@echo "  help         - Show this help message"

# phony targets
.PHONY: all clean rebuild server client myshell run-server run-client test-shell help

# precious files
# prevent make from deleting intermediate object files
.PRECIOUS: %.o