/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/10 23:27:50 by mshariar         ###   ########.fr       */
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
 * Set up redirections for builtin command
 */
static int	setup_builtin_redirections(t_cmd *cmd, int *saved_stdin, 
                                    int *saved_stdout, t_shell *shell)
{
    *saved_stdin = dup(STDIN_FILENO);
    *saved_stdout = dup(STDOUT_FILENO);
    if (*saved_stdin == -1 || *saved_stdout == -1)
        return (1);
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
    {
        printf("DEBUG: Calling builtin_cd function\n");
        int result = builtin_cd(shell, cmd);
        printf("DEBUG: builtin_cd returned %d\n", result);
        return result;
    }
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
    printf("DEBUG: No matching builtin found for %s\n", cmd_name);
    return (1);    
    
}

/**
 * Execute builtin command with redirection handling
 */
int execute_builtin(t_shell *shell, t_cmd *cmd)
{
    char *cmd_name;
    int saved_stdin;
    int saved_stdout;
    int result;

    printf("DEBUG: Starting to execute builtin: %s\n", cmd->args[0]);
    
    if (!cmd->args || !cmd->args[0])
        return (1);
    if (setup_builtin_redirections(cmd, &saved_stdin, &saved_stdout, shell) != 0)
    {
        printf("DEBUG: Failed to set up redirections\n");
        return (1);
    }
    cmd_name = cmd->args[0];
    printf("DEBUG: About to run builtin command: %s\n", cmd_name);
    result = run_builtin_command(shell, cmd, cmd_name);
    printf("DEBUG: Builtin result: %d\n", result);
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
static void	prepare_child_exec(t_shell *shell, t_cmd *cmd, 
                            char **cmd_path, char ***env_array)
{
    if (setup_redirections(cmd, shell) != 0)
        exit(1);
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
void	execute_child(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_path;
    char	**env_array;

    prepare_child_exec(shell, cmd, &cmd_path, &env_array);
    execve(cmd_path, cmd->args, env_array);
    display_error(ERR_EXEC, cmd_path, strerror(errno));
    free(cmd_path);
    free_env_array(env_array);
    exit(126);
}

/**
 * Handle fork creation for external command
 */

static int	handle_command_fork(t_shell *shell, t_cmd *cmd, char *path)
{
    pid_t	pid;
    int		status;

    pid = fork();
    if (pid == -1)
    {
        display_error(ERR_FORK, "fork", strerror(errno));
        free(path);
        return (1);
    }
    if (pid == 0)
    {
        setup_redirections(cmd, shell);
        execve(path, cmd->args, env_to_array(shell->env));
        display_error(ERR_EXEC, cmd->args[0], strerror(errno));
        exit(126);
    }
    free(path);
    waitpid(pid, &status, 0);
    process_cmd_status(shell, status);
    return (shell->exit_status);
}

/**
 * Execute a command with proper error handling
 */
int execute_command(t_shell *shell, t_cmd *cmd)
{
    char *path;
    int result;

    if (!cmd || !cmd->args || !cmd->args[0])
        return (0);
        
    if (is_builtin(cmd->args[0]))
    {
        result = execute_builtin(shell, cmd);
        shell->exit_status = result;
        return (result);  
    }
    
    // Only for external commands
    path = find_command(shell, cmd->args[0]);
    if (!path)
    {
        // REPLACE THIS SECTION:
        display_error(ERR_NOT_FOUND, cmd->args[0], "command not found");
        shell->exit_status = 127;
        return (127);
    }
    
    return (handle_command_fork(shell, cmd, path));
}
/**
 * Execute a command or pipeline with proper handling
 */
int	execute(t_shell *shell, t_cmd *cmd)
{
    if (!shell || !cmd)
        return (1);
    if (cmd->heredoc_delim || has_heredoc_redirection(cmd))
        return (process_and_execute_heredoc_command(shell, cmd));
    if (cmd->next)
        return (execute_pipeline(shell, cmd));
    else
        return (execute_command(shell, cmd));
}
