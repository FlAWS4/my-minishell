# Signal Handling System Explained

## Overview

The signal handling system in your minishell manages how the shell responds to keyboard interrupts and system signals. It's like the shell's nervous system - detecting external stimuli (like Ctrl+C) and responding appropriately without disrupting the overall operation.

## Why Signal Handling is Important

Signals are how the operating system and users communicate with your shell:
- **Ctrl+C (SIGINT)**: User wants to interrupt current operation
- **Ctrl+\ (SIGQUIT)**: User wants to quit with core dump
- **Ctrl+Z (SIGTSTP)**: User wants to suspend process
- **SIGPIPE**: Writing to broken pipe

Without proper signal handling:
- Ctrl+C would kill your entire shell
- Users couldn't interrupt long-running commands
- The shell would behave unpredictably

## System Components

### 1. **Main Signal Handler** (`signals.c`)
- Handles interactive shell signals
- Manages Ctrl+C behavior during command input
- Sets up appropriate signal masks

### 2. **Heredoc Signal Handler** (`signal_heredoc.c`)
- Special handling for here-document input
- Prevents interruption during multi-line input
- Manages cleanup when interrupted

### 3. **Signal Utilities** (`signal_utils.c`)
- Helper functions for signal management
- Signal restoration utilities
- File descriptor restoration

## Detailed Breakdown

### 1. Interactive Shell Signal Handling

#### Main Signal Setup

```c
void setup_signals(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    struct sigaction sa_tstp;
    
    // Handle SIGINT (Ctrl+C) with custom handler
    sa_int.sa_handler = handle_interrupt;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_int, NULL);
    
    // Ignore SIGQUIT (Ctrl+\)
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    // Ignore SIGTSTP (Ctrl+Z)
    sa_tstp.sa_handler = SIG_IGN;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0;
    sigaction(SIGTSTP, &sa_tstp, NULL);
}
```

**Why these choices?**
- **SIGINT handling**: Allows graceful interruption without killing shell
- **SIGQUIT ignored**: Prevents accidental shell termination
- **SIGTSTP ignored**: Minishell doesn't support job control

#### The Interrupt Handler

```c
void handle_interrupt(int sig)
{
    (void)sig;  // Unused parameter
    
    // Special case: command is executing
    if (g_exit_status == 999)
    {
        g_exit_status = 130;  // Standard Ctrl+C exit status
        write(STDOUT_FILENO, "\n", 1);
        return;
    }
    
    // Normal case: waiting for input
    g_exit_status = 130;
    write(STDOUT_FILENO, "\n", 1);
    
    // Readline cleanup and redisplay
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

**What this does**:
1. **Sets exit status**: 130 is standard for Ctrl+C termination
2. **Prints newline**: Shows ^C effect visually
3. **Cleans readline**: Clears current input line
4. **Redisplays prompt**: Shows fresh prompt for new command

### 2. Signal Behavior in Different States

#### State 1: Waiting for Input

When you're at the prompt waiting to type a command:

```bash
[user@minishell ~/path] $ <Ctrl+C>
[user@minishell ~/path] $
```

**What happens**:
- Signal handler prints newline
- Clears any partial input
- Shows new prompt
- Shell continues running

#### State 2: Command Executing

When a command is running:

```bash
[user@minishell ~/path] $ sleep 10
<Ctrl+C>
[user@minishell ~/path] $
```

**What happens**:
- Child process receives SIGINT and terminates
- Parent shell detects child termination
- Exit status set to 130
- New prompt appears

#### State 3: In Pipeline

When pipeline is running:

```bash
[user@minishell ~/path] $ cat large_file | grep pattern | sort
<Ctrl+C>
[user@minishell ~/path] $
```

**What happens**:
- All processes in pipeline receive SIGINT
- Shell waits for all children to terminate
- Cleans up pipe file descriptors
- Returns to prompt

### 3. Child Process Signal Handling

#### Default Signal Behavior for Commands

```c
void reset_signals_to_default(void)
{
    struct sigaction sa;
    
    ft_memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;  // Default behavior
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    // Reset all signals to default
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
    sigaction(SIGPIPE, &sa, NULL);
}
```

**Why reset to default?**
- Commands should behave normally
- Ctrl+C should terminate the command, not be ignored
- Programs expect standard signal behavior

#### When Reset Happens

```c
void setup_and_execute_child_process(t_shell *shell, t_command *cmd)
{
    // First thing in child process
    reset_signals_to_default();
    
    // Then setup I/O and execute
    // ...
}
```

### 4. Here-Document Signal Handling

Here-documents need special signal handling because they're multi-line input:

#### Heredoc Signal Setup

```c
void setup_heredoc_signal_handlers(struct sigaction *old_int, 
                                   struct sigaction *old_quit)
{
    struct sigaction act_int;
    struct sigaction act_quit;
    
    // Custom SIGINT handler for heredoc
    sigemptyset(&act_int.sa_mask);
    act_int.sa_handler = handle_heredoc_interrupt;
    act_int.sa_flags = 0;
    sigaction(SIGINT, &act_int, old_int);
    
    // Ignore SIGQUIT during heredoc
    sigemptyset(&act_quit.sa_mask);
    act_quit.sa_handler = SIG_IGN;
    act_quit.sa_flags = 0;
    sigaction(SIGQUIT, &act_quit, old_quit);
}
```

#### Heredoc Interrupt Handler

```c
void handle_heredoc_interrupt(int sig)
{
    (void)sig;
    g_exit_status = 19;  // Special status for heredoc interruption
    write(2, "\n", 1);   // Signal interruption occurred
}
```

#### Heredoc Example

```bash
[user@minishell ~/path] $ cat << EOF
> line 1
> line 2
> <Ctrl+C>
[user@minishell ~/path] $
```

**What happens**:
- Special signal handler detects interruption
- Sets status 19 (internal marker)
- Abandons heredoc input
- Returns to main prompt

### 5. Signal-Safe Programming

Signal handlers must be very careful about what functions they call:

#### What's Safe in Signal Handlers

```c
void handle_interrupt(int sig)
{
    // SAFE: System calls that are signal-safe
    write(STDOUT_FILENO, "\n", 1);
    
    // SAFE: Simple variable assignments
    g_exit_status = 130;
    
    // SAFE: Readline functions (documented as signal-safe)
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}
```

#### What's NOT Safe

```c
void bad_signal_handler(int sig)
{
    // UNSAFE: malloc/free
    char *buffer = malloc(100);
    
    // UNSAFE: printf family functions
    printf("Signal received\n");
    
    // UNSAFE: Complex library functions
    complex_function_call();
    
    // These could cause deadlocks or corruption
}
```

### 6. Signal Masking and Timing

#### Critical Sections

Some operations need to complete without interruption:

```c
void critical_cleanup_operation(t_shell *shell)
{
    sigset_t mask, old_mask;
    
    // Block all signals temporarily
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK, &mask, &old_mask);
    
    // Critical cleanup code here
    cleanup_resources(shell);
    
    // Restore original signal mask
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
}
```

#### Pipeline Signal Coordination

In pipelines, signals need to reach all processes:

```c
void setup_pipeline_execution(t_shell *shell, t_command *cmd)
{
    // Create process group for signal delivery
    setpgid(0, 0);
    
    // All children inherit the process group
    // Signals sent to group reach all processes
}
```

### 7. Exit Status Management

Different termination causes have different exit statuses:

```c
// Standard exit statuses
#define EXIT_SUCCESS    0   // Normal completion
#define EXIT_FAILURE    1   // General error
#define EXIT_SIGINT     130 // Terminated by SIGINT (Ctrl+C)
#define EXIT_SIGQUIT    131 // Terminated by SIGQUIT (Ctrl+\)
#define EXIT_SIGTERM    143 // Terminated by SIGTERM
```

#### Status Setting

```c
void wait_for_child_and_set_status(pid_t child_pid)
{
    int status;
    
    waitpid(child_pid, &status, 0);
    
    if (WIFEXITED(status))
    {
        // Normal exit
        g_exit_status = WEXITSTATUS(status);
    }
    else if (WIFSIGNALED(status))
    {
        // Terminated by signal
        int sig = WTERMSIG(status);
        
        if (sig == SIGINT)
            g_exit_status = 130;
        else if (sig == SIGQUIT)
            g_exit_status = 131;
        else
            g_exit_status = 128 + sig;
    }
}
```

### 8. Signal Handling in Built-ins

Built-in commands handle signals differently:

#### Built-in in Main Process

```c
void run_builtin_command(t_shell *shell, t_command *cmd)
{
    // Built-ins run in main process
    // They inherit shell's signal handling
    // Ctrl+C during built-in returns to prompt
    
    int result = execute_builtin(shell, cmd);
    
    // If interrupted, status might be 130
    if (g_exit_status == 130)
        return;  // Return to main loop
    
    g_exit_status = result;
}
```

#### Built-in in Pipeline

```c
void handle_pipe_child(t_shell *shell, t_command *cmd, int input_fd, int pipe_fds[2])
{
    // Reset signals for child process
    reset_signals_to_default();
    
    // Built-in in pipeline behaves like external command
    if (is_builtin(cmd))
    {
        int exit_code = run_builtin(shell, cmd);
        clean_and_exit_shell(shell, exit_code);
    }
}
```

### 9. Integration with Main Loop

#### Signal Handling in Command Loop

```c
static int run_command_loop(t_shell *shell)
{
    while (1)
    {
        // Setup signals before each command
        setup_signals();
        
        // Read command (can be interrupted)
        char *input = readline(prompt);
        
        if (!input)  // EOF (Ctrl+D)
            break;
        
        // Check if interrupted during input
        if (g_exit_status == 130)
        {
            g_exit_status = 0;  // Reset for next command
            continue;
        }
        
        // Process and execute command
        // ...
    }
    
    return (0);
}
```

### 10. Real-World Signal Scenarios

#### Scenario 1: Long-Running Command

```bash
[user@minishell ~/path] $ find / -name "*.txt" 2>/dev/null
<Ctrl+C>
[user@minishell ~/path] $ echo $?
130
```

**Flow**:
1. `find` starts in child process
2. Child has default signal handling
3. Ctrl+C sends SIGINT to child
4. Child terminates with signal
5. Parent detects signal termination
6. Sets exit status to 130

#### Scenario 2: Pipeline Interruption

```bash
[user@minishell ~/path] $ cat huge_file | grep pattern | sort
<Ctrl+C>
[user@minishell ~/path] $ echo $?
130
```

**Flow**:
1. Three processes created for pipeline
2. All processes in same process group
3. Ctrl+C sends SIGINT to process group
4. All processes receive signal and terminate
5. Parent waits for all children
6. Sets exit status based on last process

#### Scenario 3: Heredoc Interruption

```bash
[user@minishell ~/path] $ cat << EOF
> first line
> <Ctrl+C>
[user@minishell ~/path] $ echo $?
1
```

**Flow**:
1. Heredoc input starts with special signal handling
2. Ctrl+C triggers heredoc interrupt handler
3. Handler sets special status and abandons input
4. Returns to main loop with error status

This signal handling system makes your minishell behave predictably and user-friendly, just like real shells!
