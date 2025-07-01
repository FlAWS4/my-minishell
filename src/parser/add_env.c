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
	new_env[size] = create_managed_string_copy(&shell->memory_manager, new_var);
	if (!new_env[size])
		return (env_error("minishell: failed to duplicate new variable\n",
				&shell->memory_manager));
	new_env[size + 1] = NULL;
	shell->env = new_env;
	return (0);
}

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
		new_env[i] = create_managed_string_copy(&shell->memory_manager, \
			shell->env[i]);
		if (!new_env[i])
			return (NULL);
		i++;
	}
	return (new_env);
}

static int	add_new_var(t_shell *shell, char **new_env, char *new_var, int pos)
{
	new_env[pos] = create_managed_string_copy(&shell->memory_manager, new_var);
	if (!new_env[pos])
	{
		env_error("minishell: failed to duplicate new variable\n",
			&shell->memory_manager);
		return (0);
	}
	new_env[pos + 1] = NULL;
	return (1);
}

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
