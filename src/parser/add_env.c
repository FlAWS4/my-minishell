/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   add_env.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:32:25 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:33:10 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * update_env - Updates an existing environment variable 
 * or adds it if it doesn't exist
 * @shell: Pointer to the shell context containing the environment
 * @var: Name of the variable to update
 * @new_value: New value to set for the variable
 *
 * This function searches for an existing environment 
 * variable by name and updates
 * its value. If the variable does not exist, it adds a new variable 
 * with the given name
 * and value.
 */

void	update_env(t_shell *shell, char *var, char *new_value)
{
	int		i;
	size_t	var_len;
	char	*new_var;
	char	*tmp;

	i = 0;
	if (!shell || !shell->env || !var || !new_value)
		return ;
	var_len = ft_strlen(var);
	tmp = join_managed_strings(&shell->memory_manager, var, "=");
	if (!tmp)
		return ;
	new_var = join_managed_strings(&shell->memory_manager, tmp, new_value);
	if (!new_var)
		return ;
	while (shell->env[i])
	{
		if (ft_strncmp(shell->env[i], var, var_len) == 0
			&& shell->env[i][var_len] == '=')
			return ((void)(shell->env[i] = new_var));
		i++;
	}
	shell->env = add_env_var(shell, new_var);
}

/**
 * add_to_env - Adds a new variable to the shell's environment
 * @shell: Pointer to the shell context containing the environment
 * @new_var: New variable to add in the format "VAR=VALUE"
 *
 * This function allocates memory for a new environment variable and adds it to
 * the shell's environment. It handles memory management and error reporting.
 *
 * Returns: 0 on success, 1 on failure
 */

int	add_to_env(t_shell *shell, char *new_var)
{
	char	**new_env;
	int		size;

	size = 0;
	if (!new_var)
		return (env_error("minishell: invalid variable\n", NULL));
	while (shell->env && shell->env[size])
		size++;
	new_env = allocate_managed_memory(&shell->memory_manager, \
		sizeof(char *) * (size + 2), MEM_ERROR_RECOVERABLE, NULL);
	if (!new_env || !duplicate_existing_vars(shell, new_env, size))
		return (1);
	new_env[size] = managed_string_copy(&shell->memory_manager, new_var);
	if (!new_env[size])
		return (env_error("minishell: failed to duplicate new variable\n",
				&shell->memory_manager));
	new_env[size + 1] = NULL;
	shell->env = new_env;
	return (0);
}
/**
 * duplicate_env - Duplicates the current environment variables into a new array
 * @shell: Pointer to the shell context containing the environment
 * @new_size: Size of the new environment array to allocate
 *
 * This function allocates memory for a new environment array and copies
 * existing environment variables into it, up to the specified size.
 *
 * Returns: Pointer to the new environment array, or NULL on failure
 */

static char	**duplicate_env(t_shell *shell, int new_size)
{
	char	**new_env;
	int		i;

	i = 0;
	new_env = allocate_managed_memory(&shell->memory_manager, \
		sizeof(char *) * new_size, MEM_ERROR_RECOVERABLE, NULL);
	if (!new_env)
		return (NULL);
	while (shell->env[i] && i < new_size - 1)
	{
		new_env[i] = managed_string_copy(&shell->memory_manager, \
			shell->env[i]);
		if (!new_env[i])
			return (NULL);
		i++;
	}
	return (new_env);
}

/**
 * add_new_var - Adds a new variable to the new environment array
 * @shell: Pointer to the shell context containing the environment
 * @new_env: New environment array to which the variable will be added
 * @new_var: New variable to add in the format "VAR=VALUE"
 * @pos: Position in the new environment array where the variable will be added
 *
 * This function duplicates the new variable and
 *  adds it to the specified position
 * in the new environment array. It handles memory
 * management and error reporting.
 *
 * Returns: 1 on success, 0 on failure
 */

static int	add_new_var(t_shell *shell, char **new_env, char *new_var, int pos)
{
	new_env[pos] = managed_string_copy(&shell->memory_manager, new_var);
	if (!new_env[pos])
	{
		env_error("minishell: failed to duplicate new variable\n",
			&shell->memory_manager);
		return (0);
	}
	new_env[pos + 1] = NULL;
	return (1);
}

/**
 * add_env_var - Adds a new environment variable to the shell's environment
 * @shell: Pointer to the shell context containing the environment
 * @new_var: New variable to add in the format "VAR=VALUE"
 *
 * This function allocates memory for a new 
 * environment array, duplicates existing
 * variables, and adds the new variable to the end of the array. 
 * It handles memory
 * management and error reporting.
 *
 * Returns: Pointer to the new environment array, or NULL on failure
 */

char	**add_env_var(t_shell *shell, char *new_var)
{
	char	**new_env;
	int		env_size;

	if (!shell || !shell->env || !new_var)
	{
		env_error("minishell: invalid parameters to add_env_var\n", NULL);
		return (NULL);
	}
	env_size = 0;
	while (shell->env[env_size])
		env_size++;
	new_env = duplicate_env(shell, env_size + 2);
	if (!new_env)
	{
		env_error("minishell: failed to allocate new environment\n",
			&shell->memory_manager);
		return (NULL);
	}
	if (!add_new_var(shell, new_env, new_var, env_size))
		return (NULL);
	return (new_env);
}
