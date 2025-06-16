/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 02:43:30 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Create a full path by joining directory and command
 */
char	*create_path(char *dir, char *cmd)
{
    char	*path;
    char	*with_slash;

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
int	is_executable(char *path)
{
    struct stat	file_stat;

    if (!path)
        return (0);
    if (stat(path, &file_stat) == -1)
        return (0);
    return (S_ISREG(file_stat.st_mode) && (file_stat.st_mode & S_IXUSR));
}

/**
 * Validate command character set
 */
static int	validate_cmd_chars(char *cmd)
{
    int	i;

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
 * Check if command is an absolute or relative path
 */
static char	*check_direct_path(char *cmd)
{
    if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/'))
    {
        if (access(cmd, X_OK) == 0)
            return (ft_strdup(cmd));
    }
    return (NULL);
}

/**
 * Search for command in PATH directories
 */
static char	*search_in_path_dirs(char **paths, char *cmd)
{
    char	*cmd_path;
    int		i;

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
 * Find a command in the PATH environment
 */
char	*find_command(t_shell *shell, char *cmd)
{
    char	*path_var;
    char	**paths;
    char	*direct_path;

    if (!cmd || !cmd[0] || !shell || !shell->env)
        return (NULL);
    if (!validate_cmd_chars(cmd))
        return (NULL);
    direct_path = check_direct_path(cmd);
    if (direct_path)
        return (direct_path);
    path_var = get_env_value(shell->env, "PATH");
    if (!path_var)
        return (NULL);
    paths = ft_split(path_var, ':');
    free(path_var);
    if (!paths)
        return (NULL);
    return (search_in_path_dirs(paths, cmd));
}

/**
 * Remove colon prefix from command if present
 * Returns 1 if modified, 0 if not
 */
static int	remove_colon_prefix(char **cmd)
{
    char	*new_cmd;

    if (!cmd || !*cmd || (*cmd)[0] != ':')
        return (0);
    
    new_cmd = ft_strdup(*cmd + 1);
    if (!new_cmd)
        return (0);
    free(*cmd);
    *cmd = new_cmd;
    return (1);
}

/**
 * Copy split arguments to new array
 */
static char	**copy_split_args(char **split_args, int count)
{
    char	**new_args;
    int		i;

    new_args = malloc(sizeof(char *) * (count + 1));
    if (!new_args)
        return (NULL);
    i = 0;
    while (i < count)
    {
        new_args[i] = ft_strdup(split_args[i]);
        if (!new_args[i])
        {
            while (--i >= 0)
                free(new_args[i]);
            free(new_args);
            return (NULL);
        }
        i++;
    }
    new_args[i] = NULL;
    return (new_args);
}

/**
 * Try to split a quoted variable into command and arguments
 * Returns 1 if successfully split, 0 otherwise
 */
static int try_split_quoted_var(t_shell *shell, t_cmd *cmd)
{
    char    **split_args;
    char    *potential_cmd;
    int     count;
    char    **new_args;

    if (!cmd->args[0] || cmd->args[0][0] != ':')
        return (0);
    remove_colon_prefix(&cmd->args[0]);
    if (!ft_strchr(cmd->args[0], ' '))
        return (0);
    split_args = ft_split(cmd->args[0], ' ');
    if (!split_args || !split_args[0])
    {
        free_str_array(split_args);
        return (0);
    }
    potential_cmd = find_command(shell, split_args[0]);
    if (potential_cmd || is_builtin(split_args[0]))
    {
        free(potential_cmd);
        count = 0;
        while (split_args[count])
            count++;
        new_args = copy_split_args(split_args, count);
        if (new_args)
        {
            free_str_array(cmd->args);
            cmd->args = new_args;
            free_str_array(split_args);
            return (1);
        }
    }
    free(potential_cmd);  // Free potential_cmd in all cases
    free_str_array(split_args);
    return (0);
}

/**
 * Set up redirections for builtin command
 */
static int setup_builtin_redirections(t_cmd *cmd, int *saved_stdin, 
                                    int *saved_stdout, t_shell *shell)
{
    *saved_stdin = dup(STDIN_FILENO);
    if (*saved_stdin == -1)
        return (1);
        
    *saved_stdout = dup(STDOUT_FILENO);
    if (*saved_stdout == -1)
    {
        close(*saved_stdin);  // Close the successful dup
        return (1);
    }
    
    if (setup_redirections(cmd, shell) != 0)
    {
        close(*saved_stdin);
        close(*saved_stdout);
        return (1);
    }
    return (0);
}

/**
 * Restore original stdin/stdout after builtin
 */
static void	restore_redirections(int saved_stdin, int saved_stdout)
{
    dup2(saved_stdin, STDIN_FILENO);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdin);
    close(saved_stdout);
}

/**
 * Execute builtin command with proper redirections
 */
static int	run_builtin_command(t_shell *shell, t_cmd *cmd, char *cmd_name)
{
    if (ft_strcmp(cmd_name, "echo") == 0)
        return (builtin_echo(cmd));
    else if (ft_strcmp(cmd_name, "cd") == 0)
        return (builtin_cd(shell, cmd));
    else if (ft_strcmp(cmd_name, "pwd") == 0)
        return (builtin_pwd(shell, cmd));
    else if (ft_strcmp(cmd_name, "export") == 0)
        return (builtin_export(shell, cmd));
    else if (ft_strcmp(cmd_name, "unset") == 0)
        return (builtin_unset(shell, cmd));
    else if (ft_strcmp(cmd_name, "env") == 0)
        return (builtin_env(shell));
    else if (ft_strcmp(cmd_name, "exit") == 0)
        return (builtin_exit(shell, cmd));
    else if (ft_strcmp(cmd_name, "help") == 0)
        return (builtin_help(shell));
    return (1);
}

/**
 * Execute builtin command with redirection handling
 */
int	execute_builtin(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_name;
    int		saved_stdin;
    int		saved_stdout;
    int		result;
    
    if (!cmd->args || !cmd->args[0])
        return (1);
    if (setup_builtin_redirections(cmd, &saved_stdin, &saved_stdout, shell) != 0)
        return (1);
    cmd_name = cmd->args[0];
    result = run_builtin_command(shell, cmd, cmd_name);
    restore_redirections(saved_stdin, saved_stdout);
    cleanup_redirections(cmd);
    return (result);
}

/**
 * Check if command is a builtin
 */
int	is_builtin(char *cmd)
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
 * Set up environment for child process
 */
static void prepare_child_exec(t_shell *shell, t_cmd *cmd, 
                           char **cmd_path, char ***env_array)
{
    if (cmd->redirections || cmd->input_file || cmd->input_fd != -1)
    {
        if (setup_redirections(cmd, shell) != 0)
            exit(1);
    }
    if (cmd->args[0] && cmd->args[0][0] == ':')
    {
        if (!try_split_quoted_var(shell, cmd))
            remove_colon_prefix(&cmd->args[0]);
    }
    if (is_builtin(cmd->args[0]))
        exit(run_builtin_command(shell, cmd, cmd->args[0]));   
    *cmd_path = find_command(shell, cmd->args[0]);
    if (!*cmd_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], "command not found");
        exit(127);
    }
    *env_array = env_to_array(shell->env);
    if (!*env_array)
    {
        free(*cmd_path);
        display_error(ERROR_MEMORY, "execute", "Memory allocation failed");
        exit(126);
    }
}

/**
 * Execute a child process for an external command
 */
void execute_child(t_shell *shell, t_cmd *cmd)
{
    char    *cmd_path;
    char    **env_array;

    prepare_child_exec(shell, cmd, &cmd_path, &env_array);
    execve(cmd_path, cmd->args, env_array);
    // If we get here, execve failed
    display_error(ERR_EXEC, cmd_path, strerror(errno));
    free(cmd_path);
    free_env_array(env_array);
    cleanup_redirections(cmd);  // Close any open file descriptors
    exit(126);
}

/**
 * Handle fork creation for external command
 */
static int handle_command_fork(t_shell *shell, t_cmd *cmd, char *path)
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1)
    {
        display_error(ERR_FORK, "fork", strerror(errno));
        free(path);
        return (1);
    }
    if (pid == 0)
    {
        setup_signals_noninteractive();
        execute_child(shell, cmd);
        exit(126);
    }
    
    // Parent process
    free(path);
    waitpid(pid, &status, 0);
    process_cmd_status(shell, status);
    return (shell->exit_status);
}

/**
 * Execute a command with proper error handling
 */
int	execute_command(t_shell *shell, t_cmd *cmd)
{
    char	*path;
    int		result;

    if (!cmd || !cmd->args || !cmd->args[0])
        return (0);
    
    // Try to handle quoted variables intelligently
    if (cmd->args[0][0] == ':')
    {
        if (!try_split_quoted_var(shell, cmd))
            remove_colon_prefix(&cmd->args[0]);
    }
    
    if (is_builtin(cmd->args[0]))
    {
        result = execute_builtin(shell, cmd);
        shell->exit_status = result;
        return (result);
    }
    
    path = find_command(shell, cmd->args[0]);
    if (!path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], "command not found");
        shell->exit_status = 127;
        return (127);
    }
    
    return (handle_command_fork(shell, cmd, path));
}

/**
 * Execute a command or pipeline with proper handling
 */
int execute(t_shell *shell, t_cmd *cmd)
{
    t_cmd   *current;
    
    if (!shell || !cmd)
        return (1);
    
    if (cmd->heredoc_delim || cmd->redirections)
    {
        current = cmd;
        while (current && !g_signal)
        {
            if (!current->heredocs_processed)
                if (!process_heredoc(current, shell))
                    return (1);
            current = current->next;
        }
        
        if (g_signal)
        {
            // Clean up any partially processed heredocs
            current = cmd;
            while (current)
            {
                cleanup_redirections(current);
                current = current->next;
            }
            return (1);
        }
    }
    
    // Then execute command(s)
    if (cmd->next)
        return (execute_pipeline(shell, cmd));
    else
        return (execute_command(shell, cmd));
}
