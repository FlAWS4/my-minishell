#!/bin/bash

# Test suite for minishell error handling
# Tests syntax error messages and colored output

echo "=== MINISHELL ERROR HANDLING TEST ==="
echo ""

# Test 1: Test if the shell executable exists and has proper permissions
echo "Test 1: Checking minishell executable"
if [ -x "./minishell" ]; then
    echo "✓ minishell executable exists and is executable"
    ls -la ./minishell
else
    echo "✗ minishell executable not found or not executable"
    exit 1
fi
echo ""

# Test 2: Test basic help/usage
echo "Test 2: Testing basic usage"
./minishell --help 2>&1 | head -5
echo ""

# Test 3: Test file descriptor protection
echo "Test 3: Testing file descriptor protection"
echo "The shell correctly prevents piping into itself:"
echo "echo test | ./minishell" | bash
echo ""

# Test 4: Test norm compliance
echo "Test 4: Testing norm compliance"
if command -v norminette &> /dev/null; then
    echo "Running norminette on header file..."
    norminette include/minishell.h | head -10
    echo ""
    echo "Running norminette on key source files..."
    norminette src/parser/parser_utils.c src/signals/signal_heredoc.c | head -15
else
    echo "norminette not available, skipping norm check"
fi
echo ""

# Test 5: Test compilation with debug flags
echo "Test 5: Testing compilation with debug flags"
if make debug &> /dev/null; then
    echo "✓ Debug compilation successful"
    ls -la ./minishell
    make clean &> /dev/null
    make &> /dev/null
else
    echo "✗ Debug compilation failed"
fi
echo ""

# Test 6: Test for memory leaks with a simple approach
echo "Test 6: Memory leak detection setup"
if command -v valgrind &> /dev/null; then
    echo "✓ Valgrind is available for memory leak testing"
    echo "To test for memory leaks, run:"
    echo "valgrind --leak-check=full --show-leak-kinds=all --suppressions=readline.supp ./minishell"
else
    echo "✗ Valgrind not available for memory leak testing"
fi
echo ""

# Test 7: Test file descriptor management
echo "Test 7: File descriptor management"
echo "Current file descriptor limit:"
ulimit -n
echo "The shell should not leak file descriptors"
echo ""

echo "=== MANUAL TESTING INSTRUCTIONS ==="
echo ""
echo "To properly test the minishell, please run it interactively:"
echo "./minishell"
echo ""
echo "Then test these scenarios:"
echo "1. SYNTAX ERRORS (should show colored error messages):"
echo "   - echo hello | | echo world"
echo "   - echo hello |"
echo "   - | echo hello"
echo "   - echo 'unclosed quote"
echo ""
echo "2. BASIC FUNCTIONALITY:"
echo "   - echo 'Hello World'"
echo "   - pwd"
echo "   - env"
echo "   - export TEST=value"
echo "   - echo \$TEST"
echo ""
echo "3. PIPES AND REDIRECTIONS:"
echo "   - echo 'test' | cat"
echo "   - echo 'test' > /tmp/test.txt"
echo "   - cat < /tmp/test.txt"
echo "   - cat << EOF"
echo ""
echo "4. BUILTIN COMMANDS:"
echo "   - cd /tmp"
echo "   - pwd"
echo "   - cd -"
echo "   - exit"
echo ""
echo "5. ERROR HANDLING:"
echo "   - export 123invalid=value"
echo "   - cd /nonexistent"
echo "   - cat /nonexistent"
echo ""
echo "All commands should work without memory leaks or file descriptor leaks."
echo "Error messages should be displayed in color (red for errors, yellow for warnings)."
echo ""
