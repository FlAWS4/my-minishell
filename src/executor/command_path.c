/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_path.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:42:04 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:42:04 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Validates that a command exists and has arguments
 * 
 * @param shell  Shell context containing environment and settings
 * @param cmd    Command to validate
 * 
 * Exits the shell with appropriate error message and status code if:
 * - Command is NULL (internal error)
 * - Command has no arguments or empty first argument
 */

void	validate_command_exists(t_shell *shell, t_command *cmd)
{
	if (!cmd)
		display_error_and_exit(shell, NULL, "internal error (null command)",
			EXIT_FAILURE);
	if (!cmd->args || !cmd->args[0] || cmd->args[0][0] == '\0')
		display_error_and_exit(shell, "", "command not found", 127);
}

/**
 * Validates an absolute path for execution
 * 
 * @param shell     Shell context
 * @param cmd_path  Absolute path to check
 * 
 * @return The validated path if accessible
 * 
 * Exits the shell with appropriate error message if:
 * - Path doesn't exist (127)
 * - Path exists but execution permission denied (126)
 */
static char	*validate_executable_path(t_shell *shell, char *cmd_path)
{
	if (access(cmd_path, F_OK) != 0)
		display_error_and_exit(shell, cmd_path, "No such file or directory\n", 127);
	if (access(cmd_path, X_OK) != 0)
		display_error_and_exit(shell, cmd_path, "Permission denied", 126);
	return (cmd_path);
}
/**
 * Searches for executable in PATH directories
 * 
 * @param cmd    Command name to find
 * @param shell  Shell context containing environment variables
 * 
 * @return Full path to executable if found, NULL otherwise
 * 
 * Prioritizes PATH from environment, falls back to default path if needed.
 * Returns NULL if command not found in any PATH directory.
 */

char	*search_path_for_exec(char *cmd, t_shell *shell)
{
	char	**paths;
	char	*full_path;
	char	*temp;
	int		i;
	char	*path_env;

	i = 0;
	path_env = get_env_value(shell, "PATH");
	if (!path_env && shell->default_path && !shell->path_was_unset)
		path_env = shell->default_path;
	if (!path_env)
		return (NULL);
	paths = ft_split(path_env, ':');
	if (!paths)
		return (NULL);
	while (paths[i])
	{
		temp = join_managed_strings(&shell->memory_manager, paths[i], "/");
		full_path = join_managed_strings(&shell->memory_manager, temp, cmd);
		if (access(full_path, X_OK) == 0)
			return (free_array(paths), full_path);
		i++;
	}
	free_array(paths);
	return (NULL);
}

/**
 * Searches PATH for executable and handles command not found error
 * 
 * @param shell     Shell context
 * @param cmd_name  Command name to search for
 * 
 * @return Full path to executable
 * 
 * Exits the shell with "command not found" error if executable not found
 */
static char	*resolve_command_path(t_shell *shell, char *cmd_name)
{
	char	*executable_path;

	executable_path = search_path_for_exec(cmd_name, shell);
	if (!executable_path)
		display_error_and_exit(shell, cmd_name, "command not found", 127);
	return (executable_path);
}

/**
 * Resolves full command path for execution
 * 
 * @param shell  Shell context
 * @param cmd    Command to resolve path for
 * 
 * @return Full path to executable
 * 
 * Handles both absolute paths (containing '/') and commands to find in PATH.
 * Validates command arguments and output writability before resolution.
 * Exits the shell on validation failure or if command not found.
 */

char	*get_command_path(t_shell *shell, t_command *cmd)
{
	char	*executable_path;

	validate_command_exists(shell, cmd);
	apply_command_redirections(cmd);
	if (!cmd->args || !cmd->args[0] || !is_fd_writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (ft_strchr(cmd->args[0], '/'))
		executable_path = validate_executable_path(shell, cmd->args[0]);
	else
		executable_path = resolve_command_path(shell, cmd->args[0]);
	return (executable_path);
}
