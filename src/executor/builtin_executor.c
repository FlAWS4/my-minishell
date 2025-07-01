/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_executor.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:41:56 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:41:56 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks if command is a shell builtin
 * 
 * @param cmd  Command to check
 * 
 * @return 1 if command is a builtin, 0 otherwise
 * 
 * Supported builtins: cd, echo, env, exit, export, pwd, unset, help
 */
int	is_builtin(t_command *cmd)
{
	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	if (ft_strcmp(cmd->args[0], "cd") == 0 || \
		ft_strcmp(cmd->args[0], "echo") == 0 || \
		ft_strcmp(cmd->args[0], "env") == 0 || \
		ft_strcmp(cmd->args[0], "exit") == 0 || \
		ft_strcmp(cmd->args[0], "export") == 0 || \
		ft_strcmp(cmd->args[0], "pwd") == 0 || \
		ft_strcmp(cmd->args[0], "unset") == 0 || \
		ft_strcmp(cmd->args[0], "help") == 0)
		return (1);
	return (0);
}

/**
 * Executes a builtin command
 * 
 * @param shell  Shell context
 * @param cmd    Command to execute
 * 
 * @return Exit status of the builtin command
 * 
 * Routes execution to the appropriate builtin handler function
 * based on the command name.
 */
int	run_builtin(t_shell *shell, t_command *cmd)
{
	if (ft_strcmp(cmd->args[0], "cd") == 0)
		return (builtin_cd(shell, cmd));
	else if (ft_strcmp(cmd->args[0], "echo") == 0)
		return (builtin_echo(cmd));
	else if (ft_strcmp(cmd->args[0], "env") == 0)
		return (builtin_env(shell, cmd));
	else if (ft_strcmp(cmd->args[0], "exit") == 0)
		return (builtin_exit(shell, cmd));
	else if (ft_strcmp(cmd->args[0], "export") == 0)
		return (builtin_export(shell, cmd));
	else if (ft_strcmp(cmd->args[0], "pwd") == 0)
		return (builtin_pwd(shell));
	else if (ft_strcmp(cmd->args[0], "unset") == 0)
		return (builtin_unset(shell, cmd));
	else if (ft_strcmp(cmd->args[0], "help") == 0)
		return (builtin_help(shell));
	return (1);
}
