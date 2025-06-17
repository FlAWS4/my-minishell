/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/18 00:06:40 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a full path by joining directory and command
 */
char *create_path(char *dir, char *cmd)
{
    char *path;
    char *with_slash;

    if (!dir || !cmd)
        return (NULL);
    with_slash = ft_strjoin(dir, "/");
    if (!with_slash)
        return (NULL);
    path = ft_strjoin(with_slash, cmd);
    free(with_slash);
    return (path);
}

/**
 * Check if a command exists and is executable in the given path
 */
int is_executable(char *path)
{
    struct stat file_stat;

    if (!path)
        return (0);
    if (stat(path, &file_stat) == -1)
        return (0);
    return (S_ISREG(file_stat.st_mode) && (file_stat.st_mode & S_IXUSR));
}

/**
 * Validate command character set
 */
static int validate_cmd_chars(char *cmd)
{
    int i;

    i = 0;
    while (cmd[i])
    {
        if (!ft_isalnum(cmd[i]) && cmd[i] != '/' && cmd[i] != '.' && 
            cmd[i] != '_' && cmd[i] != '-')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Find a command in the PATH environment
 */
char *find_command(t_shell *shell, char *cmd)
{
    char    *path_var;
    char    **paths;
    char    *cmd_path;
    int     i;

    if (!cmd || !cmd[0] || !shell || !shell->env)
        return (NULL);
    if (!validate_cmd_chars(cmd))
        return (NULL);
    if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/'))
    {
        if (access(cmd, X_OK) == 0)
            return (ft_strdup(cmd));
        return (NULL);
    }
    path_var = get_env_value(shell->env, "PATH");
    if (!path_var)
        return (NULL);
    paths = ft_split(path_var, ':');
    free(path_var);
    if (!paths)
        return (NULL);
    i = 0;
    while (paths[i])
    {
        cmd_path = create_path(paths[i], cmd);
        if (cmd_path && access(cmd_path, X_OK) == 0)
        {
            free_env_array(paths);
            return (cmd_path);
        }
        free(cmd_path);
        i++;
    }
    free_env_array(paths);
    return (NULL);
}
/**
 * Execute a command directly (shared between executor and pipes)
 */
void execute_cmd(t_shell *shell, t_cmd *cmd)
{
    char *cmd_path;
    char **env_array;
    
    // Handle builtin commands
    if (is_builtin(cmd->args[0]))
        handle_builtin_child(shell, cmd);
    cmd_path = find_command(shell, cmd->args[0]);
    if (!cmd_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], "command not found");
        exit(127);
    }
    env_array = env_to_array(shell->env);
    if (!env_array)
    {
        free(cmd_path);
        display_error(ERROR_MEMORY, "execute", "Memory allocation failed");
        exit(126);
    }
    execve(cmd_path, cmd->args, env_array);
    display_error(ERR_EXEC, cmd_path, strerror(errno));
    free(cmd_path);
    free_env_array(env_array);
    exit(126);
}

/**
 * Duplicate arguments from split result into a new array
 */
static char **duplicate_arguments(char **split_args, int count)
{
    char    **new_args;
    int     i;

    new_args = malloc(sizeof(char *) * (count + 1));
    if (!new_args)
        return (NULL);
    i = 0;
    while (i < count)
    {
        new_args[i] = ft_strdup(split_args[i]);
        if (!new_args[i])
        {
            while (i > 0)
            {
                i--;
                free(new_args[i]);
            }
            free(new_args);
            return (NULL);
        }
        i++;
    }
    new_args[count] = NULL;
    return (new_args);
}

/**
 * Process command arguments with special handling for colon prefixes
 */
int process_command_args(t_shell *shell, t_cmd *cmd)
{
    char    **split_args;
    char    **new_args;
    char    *potential_cmd;
    int     count;
    
    if (!cmd->args[0] || cmd->args[0][0] != ':')
        return (1);
    
    // Remove colon prefix
    char *new_cmd = ft_strdup(cmd->args[0] + 1);
    if (!new_cmd)
        return (0);
    
    free(cmd->args[0]);
    cmd->args[0] = new_cmd;
    
    // Only process if there are spaces
    if (!ft_strchr(cmd->args[0], ' '))
        return (1);
        
    split_args = ft_split(cmd->args[0], ' ');
    if (!split_args || !split_args[0])
    {
        free_str_array(split_args);
        return (0);
    }
    
    potential_cmd = find_command(shell, split_args[0]);
    if (!potential_cmd && !is_builtin(split_args[0]))
    {
        free_str_array(split_args);
        return (1);
    }
    
    // Count arguments
    count = 0;
    while (split_args[count])
        count++;
    
    // Create new argument array
    new_args = duplicate_arguments(split_args, count);
    if (!new_args)
    {
        free(potential_cmd);
        free_str_array(split_args);
        return (0);
    }
    
    // Replace command arguments
    free_str_array(cmd->args);
    cmd->args = new_args;
    free(potential_cmd);
    free_str_array(split_args);
    return (1);
}

/**
 * Execute builtin command with redirection handling
 */
int execute_builtin(t_shell *shell, t_cmd *cmd)
{
    int     saved_stdin;
    int     saved_stdout;
    int     result;
    char    *cmd_name;
    
    if (!cmd->args || !cmd->args[0])
        return (1);
    
    // Save current standard I/O
    saved_stdin = dup(STDIN_FILENO);
    saved_stdout = dup(STDOUT_FILENO);
    if (saved_stdin == -1 || saved_stdout == -1)
    {
        if (saved_stdin != -1)
            close(saved_stdin);
        if (saved_stdout != -1)
            close(saved_stdout);
        return (1);
    }
    
    // Setup redirections
    if (setup_redirections(cmd, shell) != 0)
    {
        close(saved_stdin);
        close(saved_stdout);
        return (1);
    }
    
    // Execute the builtin command
    cmd_name = cmd->args[0];
    if (ft_strcmp(cmd_name, "echo") == 0)
        result = builtin_echo(cmd);
    else if (ft_strcmp(cmd_name, "cd") == 0)
        result = builtin_cd(shell, cmd);
    else if (ft_strcmp(cmd_name, "pwd") == 0)
        result = builtin_pwd(shell, cmd);
    else if (ft_strcmp(cmd_name, "export") == 0)
        result = builtin_export(shell, cmd);
    else if (ft_strcmp(cmd_name, "unset") == 0)
        result = builtin_unset(shell, cmd);
    else if (ft_strcmp(cmd_name, "env") == 0)
        result = builtin_env(shell);
    else if (ft_strcmp(cmd_name, "exit") == 0)
        result = builtin_exit(shell, cmd);
    else if (ft_strcmp(cmd_name, "help") == 0)
        result = builtin_help(shell);
    else
        result = 1;
    
    // Restore standard I/O
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
    
    // Clean up
    cleanup_redirections(cmd);
    
    return (result);
}

/**
 * Check if command is a builtin
 */
int is_builtin(char *cmd)
{
    if (!cmd)
        return (0);
    
    return (
        ft_strcmp(cmd, "echo") == 0 ||
        ft_strcmp(cmd, "cd") == 0 ||
        ft_strcmp(cmd, "pwd") == 0 ||
        ft_strcmp(cmd, "export") == 0 ||
        ft_strcmp(cmd, "unset") == 0 ||
        ft_strcmp(cmd, "env") == 0 ||
        ft_strcmp(cmd, "exit") == 0 ||
        ft_strcmp(cmd, "help") == 0
    );
}

/**
 * Handle builtin command execution in child process
 * Exported to be shared with pipes.c
 */
void handle_builtin_child(t_shell *shell, t_cmd *cmd)
{
    char *cmd_name;

    cmd_name = cmd->args[0];
    if (ft_strcmp(cmd_name, "echo") == 0)
        exit(builtin_echo(cmd));
    else if (ft_strcmp(cmd_name, "cd") == 0)
        exit(builtin_cd(shell, cmd));
    else if (ft_strcmp(cmd_name, "pwd") == 0)
        exit(builtin_pwd(shell, cmd));
    else if (ft_strcmp(cmd_name, "export") == 0)
        exit(builtin_export(shell, cmd));
    else if (ft_strcmp(cmd_name, "unset") == 0)
        exit(builtin_unset(shell, cmd));
    else if (ft_strcmp(cmd_name, "env") == 0)
        exit(builtin_env(shell));
    else if (ft_strcmp(cmd_name, "exit") == 0)
        exit(builtin_exit(shell, cmd));
    else if (ft_strcmp(cmd_name, "help") == 0)
        exit(builtin_help(shell));
    exit(1);
}

/**
 * Execute a child process
 */
void execute_child(t_shell *shell, t_cmd *cmd)
{
    // Set up redirections
    if (setup_redirections(cmd, shell) != 0)
        exit(1);
    
    // Process command arguments
    if (!process_command_args(shell, cmd))
        exit(1);
    
    // Execute the command (shared logic)
    execute_cmd(shell, cmd);
    // execute_cmd never returns
}

/**
 * Execute a command with proper error handling
 */
int execute_command(t_shell *shell, t_cmd *cmd)
{
    pid_t   pid;
    int     status;
    int     result;
    
    if (!cmd || !cmd->args || !cmd->args[0])
        return (0);
    
    // Process command arguments
    process_command_args(shell, cmd);
    
    // Check if command is a builtin and not in a pipeline
    if (is_builtin(cmd->args[0]) && !cmd->next)
    {
        result = execute_builtin(shell, cmd);
        shell->exit_status = result;
        return (result);
    }
    pid = fork();
    if (pid == -1)
    {
        display_error(ERR_FORK, "fork", strerror(errno));
        return (1);
    }
    
    if (pid == 0)
    {
        // Child process
        setup_signals_noninteractive();
        execute_child(shell, cmd);
        exit(126);  // Should never reach this
    }
    
    // Parent process - make sure to close any redirected fds
    cleanup_redirections(cmd);
    
    // Wait for command to complete
    waitpid(pid, &status, 0);
    process_cmd_status(shell, status);
    
    return (shell->exit_status);
}

/**
 * Execute a command or pipeline with proper handling
 */
int execute(t_shell *shell, t_cmd *cmd)
{
    if (!shell || !cmd)
        return (1);
    
    // Execute according to whether it's a pipeline or single command
    if (cmd->next)
        return (execute_pipeline(shell, cmd));
    else
        return (execute_command(shell, cmd));
}
