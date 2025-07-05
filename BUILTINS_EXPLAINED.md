# Built-in Commands System Explained

## Overview

Built-in commands are special commands that are built directly into your shell, rather than being separate programs. They need to run in the shell's own process because they modify the shell's environment, change directories, or control the shell itself. Think of them as the shell's internal tools.

## Why Built-ins Are Special

**Built-in commands run in the shell process** because they need to:
- Modify the shell's environment variables
- Change the shell's current directory
- Control the shell's execution (like `exit`)
- Access the shell's internal state

**External commands run in child processes** because they:
- Don't need to modify the shell
- Should be isolated from the shell
- Can be part of pipelines safely

## Your Minishell's Built-in Commands

### 1. `echo` - Display Text

**Purpose**: Prints text to stdout, with optional newline control.

**Usage**: 
- `echo hello world` → prints "hello world" with newline
- `echo -n hello` → prints "hello" without newline
- `echo -nnn text` → multiple -n flags work the same

#### How Echo Works

```c
int builtin_echo(t_command *cmd)
{
    int i = 1;
    int print_newline = 1;
    
    // Check for -n option(s)
    while (cmd->args[i] && is_valid_n_option(cmd->args[i]))
    {
        print_newline = 0;
        i++;
    }
    
    // Print arguments with spaces between them
    int is_first_arg = 1;
    while (cmd->args[i])
    {
        if (!is_first_arg)
            ft_putchar_fd(' ', STDOUT_FILENO);
        ft_putstr_fd(cmd->args[i], STDOUT_FILENO);
        is_first_arg = 0;
        i++;
    }
    
    // Add newline unless -n was used
    if (print_newline)
        ft_putchar_fd('\n', STDOUT_FILENO);
    
    return (0);
}
```

**Key Features**:
- Handles multiple `-n` flags correctly
- Properly spaces multiple arguments
- Works with redirections: `echo hello > file.txt`

### 2. `cd` - Change Directory

**Purpose**: Changes the shell's current working directory.

**Usage**:
- `cd /path/to/directory` → change to absolute path
- `cd ../..` → change to relative path
- `cd` → change to home directory
- `cd -` → change to previous directory

#### How CD Works

```c
int builtin_cd(t_shell *shell, t_command *cmd)
{
    char *target_path;
    
    // Determine target directory
    if (!cmd->args[1])
        target_path = get_env_value(shell, "HOME");  // cd with no args
    else if (ft_strcmp(cmd->args[1], "-") == 0)
        target_path = get_env_value(shell, "OLDPWD");  // cd -
    else
        target_path = cmd->args[1];  // cd <path>
    
    // Validate target
    if (!target_path)
    {
        error("cd", "HOME not set", NULL);
        return (1);
    }
    
    // Change directory
    if (chdir(target_path) != 0)
    {
        error("cd", target_path, strerror(errno));
        return (1);
    }
    
    // Update PWD and OLDPWD environment variables
    update_pwd_vars_with_logical_path(shell, target_path);
    
    return (0);
}
```

**Key Features**:
- Handles symbolic links correctly
- Updates `PWD` and `OLDPWD` environment variables
- Supports `cd -` to go to previous directory
- Defaults to `$HOME` when no argument given

#### PWD and OLDPWD Management

```c
static int update_pwd_vars_with_logical_path(t_shell *shell, char *target)
{
    char *oldpwd = get_env_value(shell, "PWD");
    char *logical_path;
    
    // Handle symbolic links
    if (is_symlink(target))
        logical_path = create_symlink_path(oldpwd, target);
    else
        logical_path = getcwd(NULL, 0);
    
    // Update environment variables
    if (oldpwd)
        update_env(shell, "OLDPWD", oldpwd);
    update_env(shell, "PWD", logical_path);
    
    free(logical_path);
    return (0);
}
```

### 3. `pwd` - Print Working Directory

**Purpose**: Displays the current working directory.

**Usage**: `pwd` → prints current directory path

#### How PWD Works

```c
int builtin_pwd(t_shell *shell, t_command *cmd)
{
    char *pwd;
    
    // Try to get PWD from environment first
    pwd = get_env_value(shell, "PWD");
    
    // If PWD not set or invalid, use getcwd
    if (!pwd || !is_valid_directory(pwd))
    {
        pwd = getcwd(NULL, 0);
        if (!pwd)
        {
            error("pwd", NULL, strerror(errno));
            return (1);
        }
        ft_putstr_fd(pwd, STDOUT_FILENO);
        ft_putchar_fd('\n', STDOUT_FILENO);
        free(pwd);
    }
    else
    {
        ft_putstr_fd(pwd, STDOUT_FILENO);
        ft_putchar_fd('\n', STDOUT_FILENO);
    }
    
    return (0);
}
```

**Key Features**:
- Uses `PWD` environment variable when available
- Falls back to `getcwd()` if needed
- Handles errors gracefully

### 4. `export` - Set Environment Variables

**Purpose**: Sets or displays environment variables.

**Usage**:
- `export` → display all environment variables
- `export VAR=value` → set variable with value
- `export VAR` → mark variable for export (without value)

#### How Export Works (Already Explained Earlier)

**Key Features**:
- Displays variables in sorted order
- Handles variables with and without values
- Validates variable names
- Filters out internal variables

### 5. `unset` - Remove Environment Variables

**Purpose**: Removes environment variables.

**Usage**: `unset VAR1 VAR2` → removes specified variables

#### How Unset Works

```c
int builtin_unset(t_shell *shell, t_command *cmd)
{
    int i = 1;
    int status = 0;
    
    if (!cmd->args[1])
        return (0);  // No arguments, nothing to unset
    
    while (cmd->args[i])
    {
        if (!is_valid_identifier(cmd->args[i]))
        {
            error_quoted("unset", cmd->args[i], "not a valid identifier");
            status = 1;
        }
        else
        {
            remove_env_var(shell, cmd->args[i]);
        }
        i++;
    }
    
    return (status);
}
```

**Key Features**:
- Validates variable names
- Removes multiple variables in one command
- Continues processing even if some variables are invalid

### 6. `env` - Display Environment

**Purpose**: Displays all environment variables.

**Usage**: `env` → shows all environment variables

#### How Env Works

```c
int builtin_env(t_shell *shell, t_command *cmd)
{
    int i = 0;
    
    if (!shell || !cmd)
        return (1);
    
    // Check for arguments (env doesn't accept arguments in minishell)
    if (cmd->args[1])
    {
        error("env", "too many arguments", NULL);
        return (1);
    }
    
    // Print all environment variables
    while (shell->env[i])
    {
        ft_putstr_fd(shell->env[i], STDOUT_FILENO);
        ft_putchar_fd('\n', STDOUT_FILENO);
        i++;
    }
    
    return (0);
}
```

**Key Features**:
- Simple display of all environment variables
- No sorting (unlike export)
- Includes all variables, even internal ones

### 7. `exit` - Exit Shell

**Purpose**: Terminates the shell with an optional exit code.

**Usage**:
- `exit` → exit with status 0
- `exit 42` → exit with status 42
- `exit invalid` → error, don't exit

#### How Exit Works

```c
int builtin_exit(t_shell *shell, t_command *cmd)
{
    int exit_code = 0;
    
    ft_putstr_fd("exit\n", STDERR_FILENO);
    
    if (cmd->args[1])
    {
        if (cmd->args[2])
        {
            error("exit", "too many arguments", NULL);
            g_exit_status = 1;
            return (1);  // Don't exit with multiple arguments
        }
        
        if (!is_valid_number(cmd->args[1]))
        {
            error_quoted("exit", cmd->args[1], "numeric argument required");
            clean_and_exit_shell(shell, 2);
        }
        
        exit_code = ft_atoi(cmd->args[1]) % 256;  // Bash-like behavior
    }
    else
    {
        exit_code = g_exit_status;  // Use last command's exit status
    }
    
    clean_and_exit_shell(shell, exit_code);
    return (0);  // Never reached
}
```

**Key Features**:
- Validates numeric arguments
- Handles multiple arguments error
- Cleans up before exiting
- Uses last command's exit status if no argument given

## Built-in Detection and Routing

### Built-in Detection

```c
int is_builtin(t_command *cmd)
{
    if (!cmd || !cmd->args || !cmd->args[0])
        return (0);
    
    char *command = cmd->args[0];
    
    if (ft_strcmp(command, "echo") == 0)
        return (1);
    if (ft_strcmp(command, "cd") == 0)
        return (1);
    if (ft_strcmp(command, "pwd") == 0)
        return (1);
    if (ft_strcmp(command, "export") == 0)
        return (1);
    if (ft_strcmp(command, "unset") == 0)
        return (1);
    if (ft_strcmp(command, "env") == 0)
        return (1);
    if (ft_strcmp(command, "exit") == 0)
        return (1);
    
    return (0);
}
```

### Built-in Execution Router

```c
int run_builtin(t_shell *shell, t_command *cmd)
{
    char *command = cmd->args[0];
    
    if (ft_strcmp(command, "echo") == 0)
        return builtin_echo(cmd);
    if (ft_strcmp(command, "cd") == 0)
        return builtin_cd(shell, cmd);
    if (ft_strcmp(command, "pwd") == 0)
        return builtin_pwd(shell, cmd);
    if (ft_strcmp(command, "export") == 0)
        return builtin_export(shell, cmd);
    if (ft_strcmp(command, "unset") == 0)
        return builtin_unset(shell, cmd);
    if (ft_strcmp(command, "env") == 0)
        return builtin_env(shell, cmd);
    if (ft_strcmp(command, "exit") == 0)
        return builtin_exit(shell, cmd);
    
    return (1);  // Should never reach here
}
```

## Built-in Redirection Handling

Built-ins support redirections just like external commands:

```c
void run_builtin_command(t_shell *shell, t_command *cmd)
{
    // 1. Process redirections
    if (process_command_redirections(cmd, shell) == -1)
        return;
    
    // 2. Apply redirections
    apply_command_redirections(cmd);
    
    // 3. Execute built-in
    g_exit_status = run_builtin(shell, cmd);
    
    // 4. Restore original file descriptors
    restore_standard_fds(shell);
}
```

**Examples**:
- `echo hello > file.txt` → writes to file
- `pwd >> log.txt` → appends to file
- `export < /dev/null` → ignores stdin

## Built-ins in Pipelines

Built-ins can be part of pipelines:

```bash
echo "hello world" | grep hello
pwd | cat -n
export | grep PATH
```

When a built-in is in a pipeline, it runs in a child process (not the shell process) to maintain pipeline semantics.

## Error Handling

Each built-in handles errors appropriately:

### Input Validation
```c
if (!is_valid_identifier(var_name))
{
    error_quoted("export", var_name, "not a valid identifier");
    return (1);
}
```

### System Call Errors
```c
if (chdir(target_path) != 0)
{
    error("cd", target_path, strerror(errno));
    return (1);
}
```

### Memory Errors
```c
char *path = getcwd(NULL, 0);
if (!path)
{
    error("pwd", NULL, strerror(errno));
    return (1);
}
```

## Integration with Shell Systems

### Environment System
- `export`, `unset`, `env` directly modify `shell->env`
- `cd` updates `PWD` and `OLDPWD`
- All built-ins can read environment variables

### Memory Management
- Built-ins use the shell's memory manager
- Proper cleanup on errors
- No memory leaks

### Error System
- Consistent error reporting
- Proper exit codes
- Integration with `g_exit_status`

## Built-in vs External Command Examples

### Built-in Behavior
```bash
export PATH=/new/path    # Affects the shell itself
cd /tmp                  # Changes shell's directory
exit                     # Terminates the shell
```

### External Command Behavior
```bash
/bin/ls                  # Runs in child process
grep pattern file        # Isolated from shell
python script.py         # Can't affect shell environment
```

This built-in system gives your minishell the essential commands needed to function as a real shell, while maintaining proper separation between shell operations and external programs!
