#!/bin/bash

# Test script for minishell
# Test basic functionality and error handling

echo "Testing minishell error handling and basic functionality..."

# Test 1: Invalid syntax (should show colored error)
echo "Test 1: Invalid syntax"
echo 'echo "hello" | | echo "world"' | timeout 2 /home/my42/my-minishell/minishell
echo "Exit code: $?"

# Test 2: Valid basic commands
echo "Test 2: Basic commands"
echo -e 'echo "hello world"\nexit' | timeout 2 /home/my42/my-minishell/minishell

# Test 3: Environment variables
echo "Test 3: Environment variables"
echo -e 'echo $HOME\nexit' | timeout 2 /home/my42/my-minishell/minishell

# Test 4: Builtin commands
echo "Test 4: Builtin commands"
echo -e 'pwd\nexit' | timeout 2 /home/my42/my-minishell/minishell

echo "Tests completed!"
