# Error Handling System Explained

## Overview

The error handling system in your minishell is like a safety net that catches problems, reports them clearly, and ensures the shell continues running smoothly. It's designed to be user-friendly, informative, and robust - never letting errors crash your shell.

## Why Error Handling is Critical

In a shell, errors can happen everywhere:
- Commands that don't exist
- Files that can't be opened
- Memory allocation failures
- Invalid syntax
- System call failures

A good error handling system:
- **Prevents crashes** - Errors don't bring down the shell
- **Provides clear feedback** - Users understand what went wrong
- **Maintains state** - Shell continues working after errors
- **Cleans up resources** - No memory leaks or file descriptor leaks

## System Components

### 1. **Error Display** (`error_display.c`)
- Formats and displays error messages
- Provides color-coded output for clarity
- Handles different types of error messages

### 2. **Memory Management** (`garbage_collector.c`)
- Tracks all memory allocations
- Automatically cleans up on errors
- Prevents memory leaks

### 3. **Error Recovery** (`error_free.c`)
- Handles cleanup when errors occur
- Restores shell state after errors
- Ensures resources are properly released

## Detailed Breakdown

### 1. Error Display System

Your minishell provides several types of error messages:

#### Basic Error Display

```c
void error(const char *cmd, const char *error_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    
    if (error_item)
    {
        ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
        ft_putstr_fd(error_item, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    
    if (msg)
    {
        ft_putstr_fd(BOLD_RED, STDERR_FILENO);
        ft_putstr_fd(msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    
    ft_putchar_fd('\n', STDERR_FILENO);
}
```

**Result**: Color-coded error messages like:
- `minishell: ls: /invalid/path: No such file or directory`
- `minishell: export: invalid_var=: not a valid identifier`

#### Quoted Error Display

```c
void error_quoted(const char *cmd, const char *error_item, const char *msg)
{
    // Same as error() but wraps error_item in quotes
    ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
    ft_putchar_fd('\'', STDERR_FILENO);
    ft_putstr_fd(error_item, STDERR_FILENO);
    ft_putchar_fd('\'', STDERR_FILENO);
    ft_putstr_fd(RESET, STDERR_FILENO);
}
```

**Result**: Quoted error messages for special characters:
- `minishell: export: 'invalid var': not a valid identifier`
- `minishell: unset: '123abc': not a valid identifier`

#### Warning Display

```c
void warning(const char *cmd, const char *warning_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    
    ft_putstr_fd(YELLOW "warning: " RESET, STDERR_FILENO);
    
    // Rest of warning message formatting...
}
```

**Result**: Non-critical warnings:
- `minishell: cd: warning: directory may not exist`

### 2. Error Categories and Handling

#### Command Not Found Errors

```c
// In executor
if (!command_path)
{
    error(NULL, cmd->args[0], "command not found");
    clean_and_exit_shell(shell, 127);
}
```

**Example**: `minishell: invalidcommand: command not found`

#### File System Errors

```c
// In redirection handling
if (open(filename, O_RDONLY) == -1)
{
    error(NULL, filename, strerror(errno));
    return (-1);
}
```

**Example**: `minishell: nonexistent.txt: No such file or directory`

#### Permission Errors

```c
// In command execution
if (access(command_path, X_OK) == -1)
{
    error(NULL, command_path, "Permission denied");
    clean_and_exit_shell(shell, 126);
}
```

**Example**: `minishell: /bin/restricted: Permission denied`

#### Syntax Errors

```c
// In parser
if (invalid_syntax_detected)
{
    error(NULL, "syntax error near unexpected token", token);
    return (1);
}
```

**Example**: `minishell: syntax error near unexpected token '|'`

#### Memory Allocation Errors

```c
// In memory manager
if (!malloc_result)
{
    error(NULL, NULL, "memory allocation failed");
    release_all_memory(&shell->memory_manager);
    exit(EXIT_FAILURE);
}
```

**Example**: `minishell: memory allocation failed`

### 3. Memory Management System

The memory manager tracks all allocations and ensures cleanup:

#### Memory Allocation with Tracking

```c
void *allocate_managed_memory(t_memory_node **memory_manager, 
                              size_t size, 
                              int cleanup_mode, 
                              void (*free_func)(void *))
{
    void *ptr = malloc(size);
    
    if (!ptr)
    {
        ft_putstr_fd("Error: Memory allocation failed\n", STDERR_FILENO);
        
        if (cleanup_mode == MEM_ERROR_FATAL)
        {
            release_all_memory(memory_manager);
            exit(EXIT_FAILURE);
        }
        return (NULL);
    }
    
    // Track this allocation
    if (track_memory_allocation(memory_manager, ptr, cleanup_mode, free_func))
    {
        free(ptr);
        if (cleanup_mode == MEM_ERROR_FATAL)
        {
            release_all_memory(memory_manager);
            exit(EXIT_FAILURE);
        }
        return (NULL);
    }
    
    return (ptr);
}
```

#### Memory Tracking Node

```c
typedef struct s_memory_node
{
    void                    *ptr;           // Pointer to allocated memory
    int                     cleanup_mode;   // Fatal or recoverable
    void                    (*free_func)(void *);  // Custom free function
    struct s_memory_node    *next;          // Next node in list
} t_memory_node;
```

#### Automatic Cleanup

```c
void release_all_memory(t_memory_node **memory_manager)
{
    t_memory_node *current = *memory_manager;
    t_memory_node *next;
    
    while (current)
    {
        next = current->next;
        
        if (current->free_func)
            current->free_func(current->ptr);
        else
            free(current->ptr);
        
        free(current);
        current = next;
    }
    
    *memory_manager = NULL;
}
```

### 4. Error Recovery Strategies

#### Recoverable Errors

For non-critical errors, the shell continues:

```c
// In command execution
if (execute_command_failed)
{
    error("execution failed", cmd->args[0], strerror(errno));
    g_exit_status = 1;
    return;  // Continue to next command
}
```

#### Fatal Errors

For critical errors, the shell exits cleanly:

```c
// In initialization
if (!shell->env)
{
    error("initialization failed", NULL, "could not set up environment");
    clean_and_exit_shell(shell, 1);
}
```

#### Cleanup on Exit

```c
void clean_and_exit_shell(t_shell *shell, int exit_code)
{
    // Close file descriptors
    if (shell->saved_stdin >= 0)
        close(shell->saved_stdin);
    if (shell->saved_stdout >= 0)
        close(shell->saved_stdout);
    if (shell->saved_stderr >= 0)
        close(shell->saved_stderr);
    
    // Free all tracked memory
    release_all_memory(&shell->memory_manager);
    
    // Clear history
    clear_history();
    
    // Exit with appropriate code
    exit(exit_code);
}
```

### 5. File Descriptor Error Handling

#### Checking File Descriptor Validity

```c
int is_fd_writable(int fd, const char *cmd_name)
{
    if (write(fd, "", 0) == -1)
    {
        error(cmd_name, NULL, strerror(errno));
        return (0);
    }
    return (1);
}
```

#### Safe File Operations

```c
// In redirection handling
int setup_output_redirection(const char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    if (fd == -1)
    {
        error(NULL, filename, strerror(errno));
        return (-1);
    }
    
    if (!is_fd_writable(fd, "redirection"))
    {
        close(fd);
        return (-1);
    }
    
    return (fd);
}
```

### 6. Signal Error Handling

#### Handling Interrupted System Calls

```c
// In command execution
pid_t child_pid = fork();

if (child_pid == -1)
{
    if (errno == EINTR)
    {
        // Interrupted by signal, retry
        continue;
    }
    else
    {
        error("fork", NULL, strerror(errno));
        return (1);
    }
}
```

#### Signal-Safe Error Reporting

```c
// In signal handlers (simplified)
void signal_handler(int sig)
{
    // Only use signal-safe functions
    write(STDERR_FILENO, "\nminishell: interrupted\n", 22);
    g_exit_status = 130;  // Ctrl+C exit status
}
```

### 7. Parser Error Handling

#### Syntax Error Detection

```c
int check_syntax_errors(t_shell *shell)
{
    t_token *token = shell->tokens;
    
    while (token)
    {
        if (token->type == PIPE)
        {
            // Check for pipe at start or end
            if (!token->previous || !token->next)
            {
                error(NULL, "syntax error near unexpected token", "|");
                return (1);
            }
        }
        
        // Check for invalid token combinations
        if (is_redirection_token(token))
        {
            if (!token->next || token->next->type != WORD)
            {
                error(NULL, "syntax error near unexpected token", "newline");
                return (1);
            }
        }
        
        token = token->next;
    }
    
    return (0);
}
```

#### Quote Mismatch Detection

```c
int check_quote_balance(const char *input)
{
    int single_quote_count = 0;
    int double_quote_count = 0;
    
    for (int i = 0; input[i]; i++)
    {
        if (input[i] == '\'' && double_quote_count % 2 == 0)
            single_quote_count++;
        else if (input[i] == '"' && single_quote_count % 2 == 0)
            double_quote_count++;
    }
    
    if (single_quote_count % 2 != 0)
    {
        error(NULL, "syntax error", "unclosed single quote");
        return (1);
    }
    
    if (double_quote_count % 2 != 0)
    {
        error(NULL, "syntax error", "unclosed double quote");
        return (1);
    }
    
    return (0);
}
```

### 8. Error Status Management

#### Global Exit Status

```c
int g_exit_status = 0;  // Global variable for last command's exit status
```

#### Exit Status Codes

```c
#define EXIT_SUCCESS    0   // Command succeeded
#define EXIT_FAILURE    1   // General error
#define EXIT_MISUSE     2   // Misuse of shell builtin
#define EXIT_EXEC_FAIL  126 // Command found but not executable
#define EXIT_NOT_FOUND  127 // Command not found
#define EXIT_SIGINT     130 // Terminated by Ctrl+C
#define EXIT_SIGQUIT    131 // Terminated by Ctrl+\
```

#### Status Updates

```c
// After command execution
if (command_succeeded)
    g_exit_status = 0;
else
    g_exit_status = get_appropriate_error_code(error_type);
```

### 9. User-Friendly Error Messages

#### Clear Error Descriptions

Instead of cryptic system messages, provide clear explanations:

```c
// Bad:
error(NULL, NULL, "ENOENT");

// Good:
error("cd", directory_name, "No such file or directory");
```

#### Context-Aware Messages

```c
// Provide context about what was being attempted
if (parsing_redirection)
    error(NULL, "syntax error", "expected filename after '>'");
else if (parsing_pipe)
    error(NULL, "syntax error", "expected command after '|'");
```

### 10. Integration with Shell Flow

#### Error Handling in Main Loop

```c
// In main command loop
while (1)
{
    // Get command input
    if (read_command_failed)
        continue;  // Show prompt again
    
    // Parse command
    if (parse_command_failed)
    {
        cleanup_partial_parse();
        continue;  // Show prompt again
    }
    
    // Execute command
    if (execute_command_failed)
    {
        cleanup_after_execution();
        continue;  // Show prompt again
    }
    
    // Success - continue to next iteration
}
```

#### Error Propagation

```c
// Errors propagate up the call stack
int execute_pipeline()
{
    if (setup_pipes() == -1)
        return (-1);
    
    if (fork_processes() == -1)
        return (-1);
    
    if (wait_for_completion() == -1)
        return (-1);
    
    return (0);
}
```

This comprehensive error handling system ensures your minishell is robust, user-friendly, and never crashes unexpectedly. It provides clear feedback about what went wrong and maintains a clean state for continued operation!
