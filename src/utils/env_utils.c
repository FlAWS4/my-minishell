/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:56 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:31:00 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * duplicate_existing_vars - Duplicates existing environment variables
 * @shell: Pointer to the shell context containing the environment
 * @new_env: Array to store the duplicated environment variables
 * @size: Number of variables to duplicate
 *
 * This function duplicates the existing environment variables into a new array.
 * Returns 1 on success, 0 on failure.
 */

int	duplicate_existing_vars(t_shell *shell, char **new_env, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		new_env[i] = managed_string_copy \
		(&shell->memory_manager, shell->env[i]);
		if (!new_env[i])
		{
			release_all_memory(&shell->memory_manager);
			return (0);
		}
		i++;
	}
	return (1);
}

/**
 * env_error - Handles environment-related errors
 * @msg: Error message to display
 * @gc: Pointer to the memory manager for cleanup
 *
 * This function displays an error message related to the environment and
 * releases all allocated memory if necessary.
 *
 * Returns: 1 to indicate an error occurred
 */

int	env_error(const char *msg, t_memory_node **gc)
{
	if (msg)
		error("env", NULL, msg);
	if (gc && *gc)
		release_all_memory(gc);
	return (1);
}

/**
 * duplicate_env_vars - Duplicates environment variables from envp to shell
 * @envp: Array of environment variables to duplicate
 * @shell: Pointer to the shell context containing the environment
 *
 * This function duplicates the environment variables from the provided envp
 * array into the shell's environment. It handles memory management and error
 * reporting.
 *
 * Returns: 1 on success, 0 on failure
 */

static int	duplicate_env_vars(char **envp, t_shell *shell)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		shell->env[i] = managed_string_copy \
		(&shell->memory_manager, envp[i]);
		if (!shell->env[i])
			return (env_error("failed to init environment\n", \
				&shell->memory_manager));
		i++;
	}
	shell->env[i] = NULL;
	return (1);
}

/**
 * is_valid_identifier - Checks if a string is a valid environment variable name
 * @str: The string to check
 *
 * This function checks if the provided string is a valid identifier for an
 * environment variable. 
 * It must start with an alphabetic character or underscore,
 * and can only contain alphanumeric characters and underscores.
 *
 * Returns: 1 if valid, 0 otherwise
 */

int	is_valid_identifier(const char *str)
{
	int	i;

	i = 1;
	if (!str || !str[0])
		return (0);
	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (0);
	while (str[i])
	{
		if (str[i] == '=' || (!ft_isalnum(str[i]) && str[i] != '_'))
			return (0);
		i++;
	}
	return (1);
}

/**
 * get_env - Initializes the shell's environment from envp
 * @envp: Array of environment variables to initialize from
 * @shell: Pointer to the shell context containing the environment
 *
 * This function checks if the provided envp is valid, allocates memory for the
 * shell's environment, and duplicates the environment variables into it.
 *
 * Returns: Pointer to the initialized environment array, or NULL on failure
 */

char	**get_env(char **envp, t_shell *shell)
{
	int	env_vars;

	env_vars = 0;
	if (!envp || !*envp)
	{
		env_error("invalid environment\n", NULL);
		return (NULL);
	}
	while (envp[env_vars])
		env_vars++;
	shell->env = allocate_managed_memory(&shell->memory_manager, \
		sizeof(char *) * (env_vars + 1),
			MEM_ERROR_FATAL, NULL);
	if (!shell->env)
		return (NULL);
	if (!duplicate_env_vars(envp, shell))
		return (NULL);
	return (shell->env);
}
