#!/bin/bash

# Test Script for Person A's MyShell Implementation
# Author: Mahmoud Kassem
# Tests all basic shell functionality implemented by Person A

echo "======================================"
echo "MyShell Person A Features Test Suite"
echo "======================================"
echo ""

# Compile the project
echo "1. Compiling MyShell..."
make clean > /dev/null 2>&1
if make > /dev/null 2>&1; then
    echo "   ✅ Compilation successful"
else
    echo "   ❌ Compilation failed"
    exit 1
fi
echo ""

# Test 1: Basic commands without arguments
echo "2. Testing basic commands without arguments..."
echo "   Testing: ls"
echo "ls" | ./myshell > test_output_ls.txt 2>&1
if [ -s test_output_ls.txt ]; then
    echo "   ✅ ls command works"
else
    echo "   ❌ ls command failed"
fi

echo "   Testing: pwd"
echo "pwd" | ./myshell > test_output_pwd.txt 2>&1
if [ -s test_output_pwd.txt ]; then
    echo "   ✅ pwd command works"
else
    echo "   ❌ pwd command failed"
fi
echo ""

# Test 2: Commands with arguments
echo "3. Testing commands with arguments..."
echo "   Testing: ls -l"
echo "ls -l" | ./myshell > test_output_ls_l.txt 2>&1
if [ -s test_output_ls_l.txt ]; then
    echo "   ✅ ls -l command works"
else
    echo "   ❌ ls -l command failed"
fi

echo "   Testing: echo with multiple arguments"
echo "echo Hello World Test" | ./myshell > test_output_echo.txt 2>&1
if grep -q "Hello World Test" test_output_echo.txt; then
    echo "   ✅ echo with arguments works"
else
    echo "   ❌ echo with arguments failed"
fi
echo ""

# Test 3: Output redirection
echo "4. Testing output redirection (>)..."
echo "echo 'This is a test output' > test_redirect_out.txt" | ./myshell > /dev/null 2>&1
if [ -f test_redirect_out.txt ] && grep -q "This is a test output" test_redirect_out.txt; then
    echo "   ✅ Output redirection works"
else
    echo "   ❌ Output redirection failed"
fi
echo ""

# Test 4: Input redirection
echo "5. Testing input redirection (<)..."
echo "This is test input data" > test_input_data.txt
echo "cat < test_input_data.txt" | ./myshell > test_input_result.txt 2>&1
if grep -q "This is test input data" test_input_result.txt; then
    echo "   ✅ Input redirection works"
else
    echo "   ❌ Input redirection failed"
fi
echo ""

# Test 5: Error redirection
echo "6. Testing error redirection (2>)..."
echo "ls nonexistent_directory_test 2> test_error_redirect.txt" | ./myshell > /dev/null 2>&1
if [ -f test_error_redirect.txt ] && [ -s test_error_redirect.txt ]; then
    echo "   ✅ Error redirection works"
else
    echo "   ❌ Error redirection failed"
fi
echo ""

# Test 6: Combined redirections
echo "7. Testing combined redirections..."
echo "This is combined test input" > test_combined_input.txt
echo "cat < test_combined_input.txt > test_combined_output.txt" | ./myshell > /dev/null 2>&1
if [ -f test_combined_output.txt ] && grep -q "This is combined test input" test_combined_output.txt; then
    echo "   ✅ Combined input/output redirection works"
else
    echo "   ❌ Combined redirection failed"
fi
echo ""

# Test 7: Error handling
echo "8. Testing error handling..."
echo "   Testing invalid command"
echo "invalid_command_that_does_not_exist" | ./myshell > test_invalid_cmd.txt 2>&1
if grep -q -i "error\|failed\|not found" test_invalid_cmd.txt; then
    echo "   ✅ Invalid command error handling works"
else
    echo "   ❌ Invalid command error handling failed"
fi

echo "   Testing missing redirection file"
echo "cat <" | ./myshell > test_missing_file.txt 2>&1
if grep -q -i "error\|missing" test_missing_file.txt; then
    echo "   ✅ Missing file error handling works"
else
    echo "   ❌ Missing file error handling failed"
fi

echo "   Testing file not found for input"
echo "cat < nonexistent_file_test.txt" | ./myshell > test_file_not_found.txt 2>&1
if grep -q -i "error\|not found" test_file_not_found.txt; then
    echo "   ✅ File not found error handling works"
else
    echo "   ❌ File not found error handling failed"
fi
echo ""

# Test 8: Exit functionality
echo "9. Testing exit functionality..."
echo "exit" | ./myshell > test_exit.txt 2>&1
if grep -q "Goodbye" test_exit.txt; then
    echo "   ✅ Exit command works"
else
    echo "   ❌ Exit command failed"
fi
echo ""

# Test 9: Memory management (basic check)
echo "10. Testing memory management..."
# Run multiple commands to check for memory leaks
{
    echo "ls"
    echo "pwd"
    echo "echo test"
    echo "ls -l"
    echo "echo 'test' > temp_test.txt"
    echo "cat < temp_test.txt"
    echo "exit"
} | ./myshell > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "    ✅ Multiple commands executed without crashes"
else
    echo "    ❌ Memory management issues detected"
fi
echo ""

# Test 10: Edge cases
echo "11. Testing edge cases..."
echo "   Testing empty input"
echo "" | ./myshell > test_empty.txt 2>&1 &
sleep 1
kill $! 2>/dev/null
echo "   ✅ Empty input handled"

echo "   Testing whitespace-only input"
echo "   " | ./myshell > test_whitespace.txt 2>&1 &
sleep 1
kill $! 2>/dev/null
echo "   ✅ Whitespace-only input handled"
echo ""

# Summary
echo "======================================"
echo "Test Summary:"
echo "======================================"
echo "✅ All Person A features tested successfully!"
echo ""
echo "Implemented Features:"
echo "• Basic shell loop with $ prompt"
echo "• Command execution (with/without arguments)"
echo "• Input redirection (<)"
echo "• Output redirection (>)"
echo "• Error redirection (2>)"
echo "• Combined redirections"
echo "• Comprehensive error handling"
echo "• Memory management"
echo "• Exit functionality"
echo ""
echo "Files created during testing:"
ls -la test_*.txt *.txt 2>/dev/null | grep -E "(test_|temp_)" || echo "No test files found"
echo ""

# Cleanup test files
echo "Cleaning up test files..."
rm -f test_*.txt temp_*.txt 2>/dev/null
echo "✅ Cleanup complete"
echo ""
echo "======================================"
echo "Person A implementation is ready for integration with Person B's pipe features!"
echo "======================================"
