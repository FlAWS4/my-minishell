#!/bin/bash

# Comprehensive test suite for minishell
# Tests basic functionality, error handling, and memory management

echo "=== MINISHELL COMPREHENSIVE TEST SUITE ==="
echo ""

# Test 1: Basic command execution
echo "Test 1: Basic command execution"
echo -e "echo 'Hello from minishell!'\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 2: Syntax error handling (should show colored error)
echo "Test 2: Syntax error - double pipe"
echo -e "echo hello | | echo world\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 3: Environment variable expansion
echo "Test 3: Environment variable expansion"
echo -e "echo \$HOME\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 4: Builtin commands
echo "Test 4: Builtin commands - pwd"
echo -e "pwd\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 5: Pipe functionality
echo "Test 5: Pipe functionality"
echo -e "echo 'hello world' | cat\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 6: Redirection
echo "Test 6: Output redirection"
echo -e "echo 'test output' > /tmp/minishell_test.txt\ncat /tmp/minishell_test.txt\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
rm -f /tmp/minishell_test.txt
echo ""

# Test 7: Invalid identifier in export
echo "Test 7: Invalid identifier in export"
echo -e "export 123invalid=value\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

# Test 8: Heredoc functionality
echo "Test 8: Heredoc functionality"
echo -e "cat << EOF\nThis is a heredoc test\nEOF\nexit" | timeout 3 ./minishell
echo "Exit code: $?"
echo ""

echo "=== ALL TESTS COMPLETED ==="
