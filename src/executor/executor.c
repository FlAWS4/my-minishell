/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/23 22:20:01 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Find command in PATH
 */
char *find_command(t_shell *shell, char *cmd)
{
    char *path_env;
    char **paths;
    char *full_path;
    int i;
    
    if (cmd[0] == '/' || cmd[0] == '.')
        return (ft_strdup(cmd));
    if (!(path_env = get_env_value(shell->env, "PATH")))
        return (NULL);
    paths = ft_split(path_env, ':');
    free(path_env);
    i = 0;
    while (paths[i])
    {
        if ((full_path = create_path(paths[i], cmd)) && 
            access(full_path, X_OK) == 0)
        {
            free_str_array(paths);
            return (full_path);
        }
        free(full_path);
        i++;
    }
    free_str_array(paths);
    return (NULL);
}

int	execute_builtin(t_shell *shell, t_cmd *cmd)
{
    if (ft_strcmp(cmd->args[0], "echo") == 0)
        return (builtin_echo(cmd));
    else if (ft_strcmp(cmd->args[0], "cd") == 0)
        return (builtin_cd(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "pwd") == 0)
        return (builtin_pwd(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "export") == 0)
        return (builtin_export(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "unset") == 0)
        return (builtin_unset(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "env") == 0)
        return (builtin_env(shell));
    else if (ft_strcmp(cmd->args[0], "exit") == 0)
        return (builtin_exit(shell, cmd));
    else if (ft_strcmp(cmd->args[0], "clear") == 0)
        return (builtin_clear());
     else if (ft_strcmp(cmd->args[0], "help") == 0)
        return (builtin_help(shell));
    return (1);
}

/**
 * Execute child process for external command
 */
void execute_child(t_shell *shell, t_cmd *cmd)
{
    char *cmd_path;
    char **env_array;

    // Setup signals for child process
    setup_signals_noninteractive();
    if (setup_redirections(cmd) != 0)
        exit(1);
    // IMPORTANT: Create environment array BEFORE finding command
    // This ensures we access environment before any memory is freed
    env_array = env_to_array(shell->env);
    if (!env_array)
        exit(1);
    // Now find command (which might modify memory)
    cmd_path = find_command(shell, cmd->args[0]);
    if (!cmd_path)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": command not found\n", 2);
        free_str_array(env_array);
        exit(127);
    }   
    execve(cmd_path, cmd->args, env_array);
    free(cmd_path);
    free_str_array(env_array);
    perror("minishell");
    exit(1);
}

/**
 * Execute a single command
 */
int	execute_command(t_shell *shell, t_cmd *cmd)
{
    pid_t	pid;
    int		status;

    status = 0;
    if (cmd->next)
        return (execute_pipeline(shell, cmd));
    // For built-ins, execute directly and return
    if (cmd->args && cmd->args[0] && is_builtin(cmd->args[0]))
        return (execute_builtin(shell, cmd));
    // Only external commands should reach this point
    pid = fork();
    if (pid == 0)
    {
        setup_signals_noninteractive();
        // Child process - execute external command
        execute_child(shell, cmd);
    }
    else if (pid < 0)
    {
        perror("minishell: fork");
        return (1);
    }
    waitpid(pid, &status, 0);
    process_cmd_status(shell, status);
    return (shell->exit_status);
}

int is_builtin(char *cmd)
{
    return (
        ft_strcmp(cmd, "echo") == 0 ||
        ft_strcmp(cmd, "cd") == 0 ||
        ft_strcmp(cmd, "pwd") == 0 ||
        ft_strcmp(cmd, "export") == 0 ||
        ft_strcmp(cmd, "unset") == 0 ||
        ft_strcmp(cmd, "env") == 0 ||
        ft_strcmp(cmd, "exit") == 0 ||
        ft_strcmp(cmd, "help") == 0 ||
        ft_strcmp(cmd, "clear") == 0
    );
}

/**
 * Execute a single command
 
int	execute_command(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_path;
    pid_t	pid;
    int		status;

    if (execute_builtin(shell, cmd))
        return (shell->exit_status);
    pid = fork();
    if (pid == 0)
    {
        if (setup_redirections(cmd) != 0)
            exit(1);
        cmd_path = find_command(shell, cmd->args[0]);
        if (!cmd_path)
        {
            fprintf(stderr, "minishell: %s: command not found\n", cmd->args[0]);
            exit(127);
        }
        execve(cmd_path, cmd->args, NULL);
        perror("minishell");
        exit(1);
    }
    else if (pid < 0)
    {
        perror("minishell: fork");
        return (1);
    }
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        shell->exit_status = WEXITSTATUS(status);
    return (shell->exit_status);
}
*/