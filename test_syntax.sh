#!/bin/bash

# Test script for minishell syntax error handling

echo "Testing minishell syntax error handling..."

# Create a temporary file with commands
cat > /tmp/test_commands.txt << 'EOF'
echo "hello" | | echo "world"
exit
EOF

# Test invalid syntax
echo "Test 1: Invalid syntax (should show colored error)"
cat /tmp/test_commands.txt | timeout 3 /home/my42/my-minishell/minishell

# Clean up
rm -f /tmp/test_commands.txt

echo "Test completed!"
