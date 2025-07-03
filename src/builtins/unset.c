/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:47:15 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:47:15 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	env_size(char **env)
{
	int	count;

	count = 0;
	while (env && env[count])
		count++;
	return (count);
}

static char	**duplicate_env_without_pos(t_shell *shell, int skip_pos)
{
	int		size;
	char	**new_env;
	int		i;
	int		j;

	i = 0;
	j = 0;
	size = env_size(shell->env);
	new_env = allocate_managed_memory(&shell->memory_manager, \
		sizeof(char *) * size, MEM_ERROR_RECOVERABLE, NULL);
	if (!new_env)
		return (NULL);
	while (i < size)
	{
		if (i != skip_pos)
			new_env[j++] = managed_string_copy \
			(&shell->memory_manager, shell->env[i]);
		i++;
	}
	new_env[j] = NULL;
	return (new_env);
}

static void	remove_env_var(t_shell *shell, char *var_name)
{
	int	pos;

	if (!shell || !shell->env || !var_name || !*var_name)
		return ;
	if (ft_strcmp(var_name, "PATH") == 0)
		shell->path_was_unset = 1;
	pos = find_var_pos(var_name, shell);
	if (pos == -1)
		return ;
	shell->env = duplicate_env_without_pos(shell, pos);
}

int	builtin_unset(t_shell *shell, t_command *cmd)
{
	int	i;

	i = 1;
	if (!shell || !cmd || !cmd->args)
		return (error("unset", NULL, "internal error"), 1);
	if (!cmd->args[1])
		return (0);
	while (cmd->args[i])
	{
		remove_env_var(shell, cmd->args[i]);
		i++;
	}
	return (0);
}
