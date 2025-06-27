/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   unset.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:36 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 04:48:17 by my42             ###   ########.fr       */
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
	new_env = gc_malloc(&shell->gc, sizeof(char *) * size, GC_SOFT, NULL);
	if (!new_env)
		return (NULL);
	while (i < size)
	{
		if (i != skip_pos)
			new_env[j++] = gc_strdup(&shell->gc, shell->env[i]);
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
	pos = find_var_pos(var_name, shell);
	if (pos == -1)
		return ;
	shell->env = duplicate_env_without_pos(shell, pos);
}

int		builtin_unset(t_shell *shell, t_command *cmd)
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
