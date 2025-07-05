# Main System and Program Flow Explained

## Overview

The main system is the heart of your minishell - it's the conductor that coordinates all other subsystems. Think of it as the central control room that manages the entire shell experience from startup to shutdown.

## The Big Picture: What main.c Does

The main system handles:
1. **Program startup** - Initialize everything needed to run
2. **Main loop** - Continuously read and execute commands
3. **Command routing** - Decide how to handle different types of commands
4. **Cleanup** - Properly shut down when exiting

## Detailed Breakdown

### 1. Program Entry Point: `main()` Function

```c
int main(int argc, char **argv, char **envp)
```

This is where your minishell starts. Here's what happens step by step:

#### Step 1: Argument Validation
```c
if (argc != 1)
{
    ft_putstr_fd(BOLD_RED "Usage: ./minishell\n" RESET, STDERR_FILENO);
    return (1);
}
```
**What it does**: Checks that you ran the program correctly (just `./minishell` with no arguments)
**Why it matters**: Prevents misuse and provides clear error messages

#### Step 2: Shell Structure Initialization
```c
t_shell shell;
ft_memset(&shell, 0, sizeof(t_shell));
```
**What it does**: Creates the main shell structure and zeros out all memory
**Why it matters**: Ensures clean state - no garbage values that could cause bugs

#### Step 3: Environment Setup
```c
shell.env = init_env(envp, &shell);
if (!shell.env)
    clean_and_exit_shell(&shell, 1);
```
**What it does**: Inherits environment variables from parent shell
**Why it matters**: Your shell needs PATH, HOME, etc. to function properly

#### Step 4: File Descriptor Setup
```c
init_shell_fds(&shell);
```
**What it does**: Saves copies of stdin, stdout, stderr for later restoration
**Why it matters**: Commands might redirect these - we need to restore them

#### Step 5: Interactive Mode Check
```c
if (!isatty(STDIN_FILENO))
{
    ft_putstr_fd(BOLD_YELLOW "dont pipe minishell into minishell.\n" RESET, STDERR_FILENO);
    return (0);
}
```
**What it does**: Ensures the shell is running in an interactive terminal
**Why it matters**: Prevents issues with piping or redirection into the shell

#### Step 6: Welcome Display and Main Loop
```c
ft_display_welcome();
run_command_loop(&shell);
```
**What it does**: Shows the welcome message and starts the main command loop
**Why it matters**: This is where the actual shell functionality begins

#### Step 7: Cleanup and Exit
```c
clean_and_exit_shell(&shell, g_exit_status);
return (g_exit_status);
```
**What it does**: Properly cleans up all resources and exits with the correct status
**Why it matters**: Prevents memory leaks and ensures proper exit codes

### 2. The Main Command Loop: `run_command_loop()`

This is the heart of your shell - it runs continuously until you exit:

```c
static int run_command_loop(t_shell *shell)
{
    char *prompt;
    char *input;
    int status;

    while (1)  // Infinite loop - runs until break
    {
        // 1. Generate and display prompt
        prompt = format_shell_prompt(shell);
        
        // 2. Setup signal handlers
        setup_signals();
        
        // 3. Read user input
        status = read_user_command(&input, prompt);
        free(prompt);
        
        // 4. Handle special cases (EOF, empty input)
        if (status == 1) break;      // EOF (Ctrl+D)
        if (status == 2) continue;   // Empty command
        
        // 5. Parse and validate input
        if (tokenize_and_validate_input(shell, input))
            continue;  // Syntax error - try again
        
        // 6. Execute the command
        if (safely_execute_command(shell))
            continue;  // Execution error - try again
        
        // 7. Restore file descriptors
        restore_standard_fds(shell);
    }
    return (0);
}
```

#### The Loop Cycle Explained:

1. **Prompt Generation**: Creates the prompt you see (e.g., `[user@minishell ~/path] $`)
2. **Signal Setup**: Prepares to handle Ctrl+C, Ctrl+\ properly
3. **Input Reading**: Uses readline to get your command
4. **Input Validation**: Checks for empty input or EOF
5. **Parsing**: Converts your command into tokens and validates syntax
6. **Execution**: Runs the command(s) you entered
7. **Cleanup**: Restores file descriptors for next iteration

### 3. Command Input Reading: `read_user_command()`

```c
static int read_user_command(char **input, const char *prompt)
{
    *input = readline(prompt);
    
    if (*input == NULL)  // EOF (Ctrl+D)
    {
        ft_putstr_fd("exit\n", STDOUT_FILENO);
        return (1);  // Signal to exit
    }
    
    if (**input == '\0')  // Empty command
    {
        free(*input);
        return (2);  // Signal to continue (ignore)
    }
    
    return (0);  // Success - process the command
}
```

**What each return value means**:
- `0`: Normal command to process
- `1`: Exit signal (Ctrl+D pressed)
- `2`: Empty input (just Enter pressed)

### 4. Input Processing: `tokenize_and_validate_input()`

This function handles the complex process of turning your command into executable tokens:

```c
static int tokenize_and_validate_input(t_shell *shell, char *input)
{
    // Handle multiline commands (commands ending with |)
    if (end_with_pipe(input))
    {
        if (read_complete_command(shell, &input))
            return (1);
    }
    
    // Add to history for up/down arrow navigation
    add_history(input);
    
    // Convert input string into tokens
    shell->tokens = tokenize_input(input);
    
    // Check for syntax errors
    if (syntax_check(shell))
        return (free_command(&shell->commands), 1);
    
    return (0);
}
```

**Step-by-step process**:
1. **Multiline handling**: If command ends with `|`, read more lines
2. **History**: Add command to history for later recall
3. **Tokenization**: Break command into individual tokens
4. **Syntax validation**: Check for proper syntax
5. **Error handling**: Clean up and return error if invalid

### 5. Command Execution Router: `execute_command_sequence()`

This function decides how to execute your command based on its type:

```c
void execute_command_sequence(t_shell *shell)
{
    t_command *cmd = shell->commands;
    
    if (!cmd)
        return;  // No commands to execute
    
    // Case 1: Redirection-only command (e.g., "> file")
    if (!cmd->args && cmd->redirs && !cmd->next)
    {
        int redir_status = process_command_redirections(cmd, shell);
        g_exit_status = (redir_status == -1) ? 1 : 0;
        free_command(&shell->commands);
        return;
    }
    
    // Case 2: Pipeline (multiple commands connected with |)
    if (cmd->next)
        setup_pipeline_execution(shell, cmd);
    
    // Case 3: Single command
    else
        execute_non_piped_command(shell, cmd);
    
    // Always clean up after execution
    free_command(&shell->commands);
}
```

**The three execution paths**:
1. **Redirection-only**: `> file` or `< file` with no command
2. **Pipeline**: `ls | grep txt | sort`
3. **Single command**: `ls -la`

## Data Flow Through the System

Here's how data flows through the main system:

```
User Input: "ls -la | grep txt"
    ↓
read_user_command() → Returns command string
    ↓
tokenize_and_validate_input() → Converts to tokens
    ↓ 
Parser creates command structure
    ↓
execute_command_sequence() → Routes to pipeline execution
    ↓
setup_pipeline_execution() → Handles the pipe
    ↓
Commands execute with proper input/output connections
    ↓
restore_standard_fds() → Restore normal I/O
    ↓
Loop continues waiting for next command
```

## Global Variables and State

### `g_exit_status`
```c
int g_exit_status = 0;
```
**Purpose**: Stores the exit status of the last executed command
**Why global**: Signal handlers need access to modify exit status
**Usage**: `echo $?` shows this value

### The Shell Structure
The `t_shell` structure contains all the shell's state:
- `env`: Environment variables
- `commands`: Parsed command structure  
- `tokens`: Tokenized input
- `memory_manager`: Memory management system
- File descriptors for I/O restoration

## Error Handling Strategy

The main system uses a layered error handling approach:

1. **Validation errors**: Bad arguments, syntax errors
2. **System errors**: Memory allocation failures, file errors
3. **Command errors**: Command not found, permission denied
4. **Signal handling**: Ctrl+C, Ctrl+\ interruption

Each layer handles its errors appropriately and propagates critical errors up.

## Integration with Other Subsystems

The main system connects to:
- **Parser**: Converts input to executable commands
- **Executor**: Runs the parsed commands
- **Built-ins**: Handles shell built-in commands
- **Environment**: Manages environment variables
- **Signals**: Handles interrupt signals
- **Memory**: Manages all memory allocation
- **Error handling**: Displays and manages errors

## Key Design Principles

1. **Separation of Concerns**: Each function has a single responsibility
2. **Error Isolation**: Errors in one command don't crash the shell
3. **Memory Safety**: All allocated memory is properly freed
4. **Signal Safety**: Proper handling of interrupts
5. **User Experience**: Clear prompts, helpful error messages

## Real-World Example

When you type `ls -la | grep txt`:

1. `main()` initializes everything
2. `run_command_loop()` displays prompt and waits
3. `read_user_command()` gets your input
4. `tokenize_and_validate_input()` parses it
5. `execute_command_sequence()` sees it's a pipeline
6. `setup_pipeline_execution()` handles the pipe
7. Both `ls` and `grep` execute with connected I/O
8. Loop continues for next command

This is the beautiful orchestration that makes your shell work seamlessly!
