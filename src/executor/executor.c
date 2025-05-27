/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 00:36:15 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Execute a shell builtin command
 */
int	execute_builtin(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_name;

    if (!cmd->args || !cmd->args[0])
        return (1);
    cmd_name = cmd->args[0];
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
    else if (ft_strcmp(cmd_name, "clear") == 0)
        return (builtin_clear());
    else if (ft_strcmp(cmd_name, "help") == 0)
        return (builtin_help(shell));
    return (1);
}

#include "minishell.h"

/**
 * Handle error when command is not found
 */
static void	handle_command_not_found(char *cmd, char **env_array)
{
    display_error(ERROR_COMMAND, cmd, "command not found");
    free_str_array(env_array);
    exit(127);
}

/**
 * Execute child process for external command
 */
void	execute_child(t_shell *shell, t_cmd *cmd)
{
    char	*cmd_path;
    char	**env_array;

    setup_signals_noninteractive();
    if (!cmd->args || !cmd->args[0] || !*cmd->args[0])
    {
        display_error(ERROR_COMMAND, "minishell", "command not found");
        exit(127);
    }
    if (setup_redirections(cmd) != 0)
        exit(1);
    env_array = env_to_array(shell->env);
    if (!env_array)
        exit(1);
    cmd_path = find_command(shell, cmd->args[0]);
    if (!cmd_path)
        handle_command_not_found(cmd->args[0], env_array);
    execve(cmd_path, cmd->args, env_array);
    free(cmd_path);
    free_str_array(env_array);
    if (errno == ENOEXEC)
        display_error(ERROR_COMMAND, cmd->args[0], "not an executable");
    else if (errno == EACCES)
        display_error(ERROR_PERMISSION, cmd->args[0], "permission denied");
    else
        display_error(0, cmd->args[0], strerror(errno));
    exit(126);
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
        ft_strcmp(cmd, "help") == 0 ||
        ft_strcmp(cmd, "clear") == 0
    );
}
