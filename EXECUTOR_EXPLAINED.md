# Executor System Explained

## Overview

The executor system is the powerhouse of your minishell - it takes the parsed commands and actually runs them. Think of it as the stage manager in a theater production, coordinating all the moving parts to make sure each command runs correctly with proper input/output connections.

## The Big Picture: From Commands to Execution

```
Parsed Commands
    ↓
Executor determines execution type
    ↓
├── Built-in Command → Run in current process
├── Single Command → Fork and execute
└── Pipeline → Create multiple processes with pipes
    ↓
All processes coordinate and run
    ↓
Results returned to main loop
```

## System Components

### 1. **Command Executor** (`executor.c`)
- Decides how to execute commands
- Handles built-ins vs external commands
- Manages child processes

### 2. **Pipeline Handler** (`pipes.c`)
- Creates and manages pipes between commands
- Coordinates multiple processes
- Handles process synchronization

### 3. **Redirection Manager** (`redirections.c`)
- Sets up file redirections
- Manages file descriptors
- Handles input/output routing

### 4. **Command Path Resolver** (`command_path.c`)
- Finds executable files in PATH
- Handles command lookup
- Manages command validation

### 5. **Built-in Executor** (`builtin_executor.c`)
- Executes shell built-in commands
- Handles special built-in logic
- Manages built-in redirections

## Detailed Breakdown

### 1. Main Execution Router

The executor starts by determining what type of execution is needed:

```c
void execute_command_sequence(t_shell *shell)
{
    t_command *cmd = shell->commands;
    
    if (!cmd)
        return;  // Nothing to execute
    
    // Case 1: Redirection-only (no command)
    if (!cmd->args && cmd->redirs && !cmd->next)
    {
        process_command_redirections(cmd, shell);
        return;
    }
    
    // Case 2: Pipeline (multiple commands)
    if (cmd->next)
        setup_pipeline_execution(shell, cmd);
    
    // Case 3: Single command
    else
        execute_non_piped_command(shell, cmd);
}
```

### 2. Single Command Execution

For simple commands like `ls -la`:

```c
void execute_non_piped_command(t_shell *shell, t_command *cmd)
{
    // Built-in commands run in current process
    if (is_builtin(cmd))
    {
        run_builtin_command(shell, cmd);
        return;
    }
    
    // External commands need a child process
    pid_t child_pid = fork();
    
    if (child_pid == -1)
    {
        error("fork", NULL, strerror(errno));
        g_exit_status = 1;
        return;
    }
    
    if (child_pid == 0)
    {
        // Child process: setup and execute
        setup_and_execute_child_process(shell, cmd);
    }
    else
    {
        // Parent process: wait for child to complete
        ignore_sigint_and_wait(child_pid);
    }
}
```

#### Why Fork for External Commands?

**Built-in commands** (like `cd`, `export`) run in the current process because they need to modify the shell's environment.

**External commands** (like `ls`, `grep`) run in child processes because:
- They shouldn't affect the shell's environment
- If they crash, the shell continues running
- They can be part of pipelines
- They can have different signal handling

### 3. Child Process Setup

When executing an external command, the child process goes through several setup steps:

```c
void setup_and_execute_child_process(t_shell *shell, t_command *cmd)
{
    // 1. Reset signal handlers to default
    reset_signals_to_default();
    
    // 2. Handle redirections
    if (process_command_redirections(cmd, shell) == -1)
        clean_and_exit_shell(shell, 1);
    
    // 3. Apply file descriptor changes
    apply_command_redirections(cmd);
    
    // 4. Validate command and output
    if (!cmd->args || !cmd->args[0] || !is_fd_writable(STDOUT_FILENO, cmd->args[0]))
        clean_and_exit_shell(shell, 1);
    
    // 5. Execute the command
    execute_external_command(shell, cmd);
}
```

### 4. Command Path Resolution

Before executing an external command, the system needs to find it:

```c
char *get_command_path(t_shell *shell, t_command *cmd)
{
    char *command_name = cmd->args[0];
    
    // If path contains '/', it's an absolute or relative path
    if (ft_strchr(command_name, '/'))
        return ft_strdup(command_name);
    
    // Otherwise, search in PATH environment variable
    return search_path_for_exec(command_name, shell);
}
```

#### Path Search Process

```c
char *search_path_for_exec(char *command, t_shell *shell)
{
    char *path_env = get_env_value(shell, "PATH");
    char **path_dirs = ft_split(path_env, ':');
    
    for (int i = 0; path_dirs[i]; i++)
    {
        char *full_path = build_full_path(path_dirs[i], command);
        
        if (access(full_path, X_OK) == 0)  // Executable and accessible
        {
            free_string_array(path_dirs);
            return full_path;
        }
        
        free(full_path);
    }
    
    free_string_array(path_dirs);
    return NULL;  // Command not found
}
```

### 5. Built-in Command Execution

Built-in commands require special handling:

```c
void run_builtin_command(t_shell *shell, t_command *cmd)
{
    // 1. Process redirections (built-ins can be redirected too)
    if (process_command_redirections(cmd, shell) == -1)
        return;
    
    // 2. Apply the redirections
    apply_command_redirections(cmd);
    
    // 3. Validate command
    if (!cmd->args || !cmd->args[0] || !is_fd_writable(STDOUT_FILENO, cmd->args[0]))
    {
        restore_standard_fds(shell);
        g_exit_status = 1;
        return;
    }
    
    // 4. Handle 'exit' specially (needs to clean up)
    if (!ft_strcmp(cmd->args[0], "exit"))
    {
        builtin_exit(shell, cmd);
        restore_standard_fds(shell);
        return;
    }
    
    // 5. Execute the built-in
    g_exit_status = run_builtin(shell, cmd);
    
    // 6. Handle special error cases
    check_device_full_error(cmd);
    
    // 7. Restore original file descriptors
    restore_standard_fds(shell);
}
```

### 6. Pipeline Execution

Pipelines are the most complex part of the executor. Here's how `ls | grep txt` works:

#### Step 1: Create Process Chain

```c
void setup_pipeline_execution(t_shell *shell, t_command *cmd)
{
    int pipe_fds[2];
    int input_fd = STDIN_FILENO;
    pid_t *child_pids = allocate_pid_array(count_commands(cmd));
    int cmd_index = 0;
    
    while (cmd)
    {
        // Create pipe for output (except for last command)
        if (cmd->next && pipe(pipe_fds) == -1)
        {
            error("pipe", NULL, strerror(errno));
            return;
        }
        
        // Fork child process
        child_pids[cmd_index] = fork();
        
        if (child_pids[cmd_index] == 0)
        {
            // Child process
            handle_pipe_child(shell, cmd, input_fd, pipe_fds);
        }
        
        // Parent process: manage file descriptors
        if (input_fd != STDIN_FILENO)
            close(input_fd);
        
        if (cmd->next)
        {
            close(pipe_fds[1]);  // Close write end
            input_fd = pipe_fds[0];  // Read end becomes next input
        }
        
        cmd = cmd->next;
        cmd_index++;
    }
    
    // Wait for all children to complete
    wait_for_all_children(child_pids, cmd_index);
}
```

#### Step 2: Child Process in Pipeline

```c
void handle_pipe_child(t_shell *shell, t_command *cmd, int input_fd, int pipe_fds[2])
{
    // 1. Reset signals for child
    reset_signals_to_default();
    
    // 2. Clean up parent's file descriptors
    cleanup_parent_fds(shell);
    
    // 3. Set up input/output connections
    setup_command_io(cmd, input_fd, pipe_fds);
    
    // 4. Close unused file descriptors
    close_unused_command_fds(shell->commands, cmd);
    
    // 5. Handle command-specific redirections
    if (process_command_redirections(cmd, shell) == -1)
        clean_and_exit_shell(shell, 1);
    
    // 6. Apply redirections
    apply_command_redirections(cmd);
    
    // 7. Execute command
    if (is_builtin(cmd))
    {
        int exit_code = run_builtin(shell, cmd);
        clean_and_exit_shell(shell, exit_code);
    }
    else
    {
        execute_external_command_or_exit(shell, cmd);
    }
}
```

#### Step 3: I/O Connection Setup

```c
void setup_command_io(t_command *cmd, int input_fd, int pipe_fds[2])
{
    // Set up input
    if (input_fd != STDIN_FILENO)
    {
        dup2(input_fd, STDIN_FILENO);
        close(input_fd);
    }
    
    // Set up output
    if (cmd->next)  // Not the last command
    {
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
        close(pipe_fds[0]);
    }
}
```

### 7. Redirection System

The redirection system handles `<`, `>`, `>>`, and `<<` operators:

#### File Redirection Processing

```c
int process_command_redirections(t_command *cmd, t_shell *shell)
{
    t_redir *redir = cmd->redirs;
    
    while (redir)
    {
        if (redir->type == REDIR_IN)
        {
            redir->fd = open(redir->file, O_RDONLY);
            if (redir->fd == -1)
            {
                error(NULL, redir->file, strerror(errno));
                return -1;
            }
        }
        else if (redir->type == REDIR_OUT)
        {
            redir->fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (redir->fd == -1)
            {
                error(NULL, redir->file, strerror(errno));
                return -1;
            }
        }
        else if (redir->type == REDIR_APPEND)
        {
            redir->fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (redir->fd == -1)
            {
                error(NULL, redir->file, strerror(errno));
                return -1;
            }
        }
        else if (redir->type == REDIR_HEREDOC)
        {
            redir->fd = process_heredoc(redir->file, shell);
            if (redir->fd == -1)
                return -1;
        }
        
        redir = redir->next;
    }
    
    return 0;
}
```

#### Applying Redirections

```c
void apply_command_redirections(t_command *cmd)
{
    t_redir *redir = cmd->redirs;
    
    while (redir)
    {
        if (redir->type == REDIR_IN || redir->type == REDIR_HEREDOC)
        {
            dup2(redir->fd, STDIN_FILENO);
            close(redir->fd);
        }
        else if (redir->type == REDIR_OUT || redir->type == REDIR_APPEND)
        {
            dup2(redir->fd, STDOUT_FILENO);
            close(redir->fd);
        }
        
        redir = redir->next;
    }
}
```

### 8. Here Document Processing

Here documents (`<<`) are handled specially:

```c
int process_heredoc(char *delimiter, t_shell *shell)
{
    int pipe_fds[2];
    pid_t child_pid;
    
    if (pipe(pipe_fds) == -1)
        return -1;
    
    child_pid = fork();
    
    if (child_pid == 0)
    {
        // Child: read lines until delimiter
        close(pipe_fds[0]);
        
        char *line;
        while ((line = readline("> ")))
        {
            if (ft_strcmp(line, delimiter) == 0)
            {
                free(line);
                break;
            }
            
            // Expand variables in here document
            char *expanded = expand_variables(line, shell);
            write(pipe_fds[1], expanded, ft_strlen(expanded));
            write(pipe_fds[1], "\n", 1);
            
            free(line);
            free(expanded);
        }
        
        close(pipe_fds[1]);
        exit(0);
    }
    else
    {
        // Parent: return read end of pipe
        close(pipe_fds[1]);
        waitpid(child_pid, NULL, 0);
        return pipe_fds[0];
    }
}
```

## Data Flow Examples

### Simple Command: `ls -la`

```
1. executor.c: execute_non_piped_command()
2. Fork child process
3. Child: setup_and_execute_child_process()
4. Child: get_command_path() → "/bin/ls"
5. Child: execve("/bin/ls", ["ls", "-la"], env)
6. Parent: wait for child completion
7. Update g_exit_status
```

### Pipeline: `ls | grep txt`

```
1. executor.c: setup_pipeline_execution()
2. Create pipe: pipe_fds[0] (read), pipe_fds[1] (write)
3. Fork child 1:
   - Connect stdout to pipe_fds[1]
   - Execute "ls"
4. Fork child 2:
   - Connect stdin to pipe_fds[0]
   - Execute "grep txt"
5. Parent: close pipe ends, wait for both children
6. Update g_exit_status from last command
```

### Redirection: `cat < input.txt > output.txt`

```
1. executor.c: execute_non_piped_command()
2. Fork child process
3. Child: process_command_redirections()
   - Open "input.txt" for reading
   - Open "output.txt" for writing
4. Child: apply_command_redirections()
   - dup2(input_fd, STDIN_FILENO)
   - dup2(output_fd, STDOUT_FILENO)
5. Child: execute "cat"
6. Parent: wait for completion
```

## Error Handling

The executor handles many types of errors:

### Command Not Found
```c
if (!full_path)
{
    error(NULL, cmd->args[0], "command not found");
    clean_and_exit_shell(shell, 127);
}
```

### Permission Denied
```c
if (access(full_path, X_OK) != 0)
{
    error(NULL, full_path, "Permission denied");
    clean_and_exit_shell(shell, 126);
}
```

### File Errors
```c
if (redir->fd == -1)
{
    error(NULL, redir->file, strerror(errno));
    return -1;
}
```

### Fork Failures
```c
if (child_pid == -1)
{
    error("fork", NULL, strerror(errno));
    g_exit_status = 1;
    return;
}
```

## Integration with Other Systems

### With Parser
- Receives structured command objects
- Uses redirection and argument information
- Handles command chains from pipes

### With Built-ins
- Detects built-in commands
- Calls appropriate built-in functions
- Handles built-in redirections

### With Environment
- Passes environment to child processes
- Uses PATH for command lookup
- Handles environment variable expansion

### With Signals
- Sets up signal handling for children
- Handles Ctrl+C in pipelines
- Manages process groups

## Memory Management

The executor carefully manages memory:
- Cleans up on errors
- Frees command structures after execution
- Properly closes file descriptors
- Handles child process cleanup

## Performance Considerations

- Minimizes file descriptor leaks
- Efficient process creation
- Proper signal handling
- Optimized path searching

This executor system is what makes your shell capable of running complex commands, pipelines, and redirections just like a real shell!
