#!/bin/bash

# Interactive test for minishell
# This script will test the shell manually

echo "=== MINISHELL INTERACTIVE TEST ==="
echo ""
echo "Testing minishell interactively..."
echo ""

# Test memory leaks with valgrind if available
if command -v valgrind &> /dev/null; then
    echo "Running memory leak test with valgrind..."
    echo "This will test basic functionality and check for memory leaks"
    echo ""
    # Create a test command file
    cat > /tmp/minishell_commands.txt << 'EOF'
echo "Testing memory management"
export TEST_VAR="hello world"
echo $TEST_VAR
pwd
echo "done"
exit
EOF
    
    # Run minishell with valgrind
    echo "Running: valgrind --leak-check=full --show-leak-kinds=all --suppressions=readline.supp ./minishell"
    echo "Note: You'll need to manually enter the commands from /tmp/minishell_commands.txt"
    echo ""
    echo "Commands to test:"
    cat /tmp/minishell_commands.txt
    echo ""
    echo "Press Ctrl+C to cancel valgrind test, or Enter to continue..."
    read -r
    
    valgrind --leak-check=full --show-leak-kinds=all --suppressions=readline.supp ./minishell 2>&1 | head -30
    
    rm -f /tmp/minishell_commands.txt
else
    echo "Valgrind not available, skipping memory leak test"
fi

echo ""
echo "=== BASIC FUNCTIONALITY TEST ==="
echo "Please manually test the following:"
echo "1. Start ./minishell"
echo "2. Test syntax error: echo hello | | echo world"
echo "3. Test basic command: echo 'Hello World'"
echo "4. Test environment variable: echo \$HOME"
echo "5. Test builtin: pwd"
echo "6. Test pipe: echo 'test' | cat"
echo "7. Test redirection: echo 'test' > /tmp/test.txt"
echo "8. Test heredoc: cat << EOF"
echo "9. Test export with invalid identifier: export 123invalid=value"
echo "10. Type 'exit' to quit"
echo ""
echo "All these should work without memory leaks or fd leaks"
echo ""
