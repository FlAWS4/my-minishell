/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/17 20:09:50 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Find command in PATH
 */
char	*find_command(t_shell *shell, char *cmd)
{
    char	*path;
    char	*full_path;
    char	**paths;
    int		i;

    if (!cmd || !*cmd)
        return (NULL);
    if (cmd[0] == '/' || cmd[0] == '.')
        return (ft_strdup(cmd));
    path = get_env_value(shell->env, "PATH");
    if (!path)
        return (NULL);
    paths = ft_split(path, ':');
    i = 0;
    while (paths[i])
    {
        full_path = ft_strjoin(paths[i], "/");
        full_path = ft_strjoin_free(full_path, cmd);
        if (access(full_path, F_OK | X_OK) == 0)
            return (full_path);
        free(full_path);
        i++;
    }
    return (NULL);
}

/**
 * Execute a built-in command
 * Returns 1 if command was a builtin, 0 otherwise
 */
int	execute_builtin(t_shell *shell, t_cmd *cmd)
{
    if (!cmd->args || !cmd->args[0])
        return (0);
    if (ft_strcmp(cmd->args[0], "echo") == 0)
        return (builtin_echo(cmd));
    if (ft_strcmp(cmd->args[0], "cd") == 0)
        return (builtin_cd(shell, cmd));
    if (ft_strcmp(cmd->args[0], "pwd") == 0)
        return (builtin_pwd(shell));
    if (ft_strcmp(cmd->args[0], "export") == 0)
        return (builtin_export(shell, cmd));
    if (ft_strcmp(cmd->args[0], "unset") == 0)
        return (builtin_unset(shell, cmd));
    if (ft_strcmp(cmd->args[0], "env") == 0)
        return (builtin_env(shell));
    if (ft_strcmp(cmd->args[0], "exit") == 0)
        return (builtin_exit(shell, cmd));
    return (0);
}

/**
 * Execute child process for external command
 */
void	execute_child(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_path;

    if (setup_redirections(cmd) != 0)
        exit(1);
    cmd_path = find_command(shell, cmd->args[0]);
    if (!cmd_path)
    {
        ft_putstr_fd("minishell: ", 2);
        ft_putstr_fd(cmd->args[0], 2);
        ft_putstr_fd(": command not found\n", 2);
        exit(127);
    }
    execve(cmd_path, cmd->args, NULL);
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

    if (execute_builtin(shell, cmd))
        return (shell->exit_status);
    pid = fork();
    if (pid == 0)
        execute_child(shell, cmd);
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