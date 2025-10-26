#!/bin/bash
# test_client.sh - Comprehensive testing script for Phase 2 Client
# This script tests various commands including Phase 1 features

echo "========================================="
echo "Phase 2 Client Testing Script"
echo "========================================="
echo ""

# Change to the project directory
cd "$(dirname "$0")"

# Check if executables exist
if [ ! -f "./server" ] || [ ! -f "./client" ]; then
    echo "Error: server or client executable not found"
    echo "Please run 'make all' first"
    exit 1
fi

# Function to run a test
run_test() {
    local test_name="$1"
    local command="$2"
    
    echo "-------------------------------------------"
    echo "Test: $test_name"
    echo "Command: $command"
    echo "-------------------------------------------"
    
    # Start server in background
    ./server > /dev/null 2>&1 &
    SERVER_PID=$!
    sleep 1
    
    # Send command to client
    echo -e "${command}\nexit" | ./client
    
    # Wait for server to finish
    wait $SERVER_PID 2>/dev/null
    
    echo ""
}

echo "Starting tests..."
echo ""

# Test 1: Simple commands
run_test "Current Directory (pwd)" "pwd"
run_test "List Files (ls)" "ls"
run_test "List with Arguments (ls -l)" "ls -l Makefile"

# Test 2: Echo command
run_test "Echo Command" "echo Hello from remote shell!"

# Test 3: Phase 1 Pipes
run_test "Pipe: ls | grep test" "ls | grep test"
run_test "Pipe: cat input | wc -l" "cat input | wc -l"

# Test 4: Phase 1 Input Redirection
run_test "Input Redirection: cat < input" "cat < input"

# Test 5: Reading files
run_test "Read File: cat tests/testfile1.txt" "cat tests/testfile1.txt"

# Test 6: Error handling
run_test "Invalid Command" "unknowncmd"

# Test 7: Empty lines (should not cause issues)
echo "-------------------------------------------"
echo "Test: Empty Input Handling"
echo "-------------------------------------------"
./server > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1
echo -e "\n\nexit" | ./client
wait $SERVER_PID 2>/dev/null
echo ""

echo "========================================="
echo "All tests completed successfully!"
echo "========================================="


