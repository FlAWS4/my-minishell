/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 00:46:05 by mshariar         ###   ########.fr       */
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
 * Find a command in the PATH environment
 */
char *find_command(t_shell *shell, char *cmd)
{
    char *path_var;
    char **paths;
    char *cmd_path;
    int i;
    
    if (!cmd || !cmd[0] || !shell || !shell->env)
        return (NULL);
    
    // Validate command characters
    i = 0;
    while (cmd[i])
    {
        if (!ft_isalnum(cmd[i]) && cmd[i] != '/' && cmd[i] != '.' && 
            cmd[i] != '_' && cmd[i] != '-')
            return (NULL);
        i++;
    }
    
    // Check if cmd is already a path
    if (cmd[0] == '/' || (cmd[0] == '.' && cmd[1] == '/'))
    {
        if (access(cmd, X_OK) == 0)
            return (ft_strdup(cmd));
        return (NULL);
    }
    
    // Get PATH variable
    path_var = get_env_value(shell->env, "PATH");
    if (!path_var)
        return (NULL);
    
    // Debug output
    printf("DEBUG: Searching for '%s' in PATH\n", cmd);
    
    // Split PATH into directories
    paths = ft_split(path_var, ':');
    free(path_var);
    if (!paths)
        return (NULL);
    
    // Check each directory
    i = 0;
    while (paths[i])
    {
        cmd_path = create_path(paths[i], cmd);
        if (cmd_path && access(cmd_path, X_OK) == 0)
        {
            printf("DEBUG: Found command at: %s\n", cmd_path);
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
 * Set up redirections for builtin command
 */
static int setup_builtin_redirections(t_cmd *cmd, int *saved_stdin, 
                                   int *saved_stdout, t_shell *shell)
{
    *saved_stdin = dup(STDIN_FILENO);
    *saved_stdout = dup(STDOUT_FILENO);
    if (*saved_stdin == -1 || *saved_stdout == -1)
        return (1);
    
    // Add debug output to track heredoc processing
    printf("DEBUG: Setting up builtin redirections, input_fd=%d\n", cmd->input_fd);
    
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

int execute_builtin(t_shell *shell, t_cmd *cmd)
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
    
    // Clean up redirections
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
 * Execute a child process for an external command
 */
void execute_child(t_shell *shell, t_cmd *cmd)
{
    char *cmd_path;
    char **env_array;
    
    // Set up redirections
    if (setup_redirections(cmd, shell) != 0)
        exit(1);
    
    // Handle builtin commands in child process
    if (is_builtin(cmd->args[0]))
    {
        exit(run_builtin_command(shell, cmd, cmd->args[0]));
    }
    
    // Find command path
    cmd_path = find_command(shell, cmd->args[0]);
    if (!cmd_path)
    {
        display_error(ERR_NOT_FOUND, cmd->args[0], NULL);
        exit(127);  // Command not found
    }
    
    // Convert environment to array for execve
    env_array = env_to_array(shell->env);
    if (!env_array)
    {
        free(cmd_path);
        exit(126);  // Memory allocation failure
    }
    
    // Execute the command
    execve(cmd_path, cmd->args, env_array);
    
    // If we get here, execve failed
    display_error(ERR_EXEC, cmd_path, strerror(errno));
    free(cmd_path);
    free_env_array(env_array);
    exit(126);  // Command exists but could not be executed
}
/**
 * Execute a command with proper error handling
 */
int execute_command(t_shell *shell, t_cmd *cmd)
{
    char *path;
    int status;
    pid_t pid;
    
    if (!cmd || !cmd->args || !cmd->args[0])
    {
        printf("DEBUG: No command to execute\n");
        return (0);
    }
    
    printf("DEBUG: Executing command: %s\n", cmd->args[0]);
    
    // Check for built-in commands first
    if (is_builtin(cmd->args[0]))
    {
        printf("DEBUG: Executing builtin: %s\n", cmd->args[0]);
        return (execute_builtin(shell, cmd));
    }
    
    // Find the command in PATH
    path = find_command(shell, cmd->args[0]);
    if (!path)
    {
        // Command not found error
        printf("minishell: %s: command not found\n", cmd->args[0]);
        shell->exit_status = 127;  // Standard error for command not found
        return (127);
    }
    
    printf("DEBUG: Found command at path: %s\n", path);
    
    // Fork and execute
    pid = fork();
    if (pid == -1)
    {
        printf("minishell: fork error: %s\n", strerror(errno));
        free(path);
        return (1);
    }
    
    if (pid == 0)
    {
        // Child process
        setup_redirections(cmd, shell);
        // Add this after setup_redirections(cmd):
        printf("DEBUG: In child process, about to execve\n");
        fflush(stdout); // Force output before execv
        execve(path, cmd->args, env_to_array(shell->env));
        // If execve returns, it failed
        printf("minishell: %s: %s\n", cmd->args[0], strerror(errno));
        exit(126); // Permission denied or other exec error
    }
    
    // Parent process
    free(path);
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
    
    // Debug output to see what's happening
    printf("DEBUG: In execute function\n");
    if (cmd->args && cmd->args[0])
        printf("DEBUG: Attempting to execute: '%s'\n", cmd->args[0]);
    else
        printf("DEBUG: No command to execute (null args)\n");
    
    // Handle heredocs specially if present
    if (cmd->heredoc_delim || has_heredoc_redirection(cmd))
    {
        printf("DEBUG: Executing with heredoc\n");
        return (process_and_execute_heredoc_command(shell, cmd));
    }
    
    // Route to proper execution function based on presence of pipes
    if (cmd->next)
    {
        printf("DEBUG: Executing pipeline\n");
        return (execute_pipeline(shell, cmd));
    }
    else
    {
        printf("DEBUG: Executing single command\n");
        return (execute_command(shell, cmd));
    }
}
