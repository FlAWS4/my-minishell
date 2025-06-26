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

int	duplicate_existing_vars(t_shell *shell, char **new_env, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		new_env[i] = gc_strdup(&shell->gc, shell->env[i]);
		if (!new_env[i])
		{
			gc_free_all(&shell->gc);
			return (0);
		}
		i++;
	}
	return (1);
}

int	env_error(const char *msg, t_gc **gc)
{
	if (msg)
		error("env", NULL, msg);
	if (gc && *gc)
		gc_free_all(gc);
	return (1);
}

static int	duplicate_env_vars(char **envp, t_shell *shell)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		shell->env[i] = gc_strdup(&shell->gc, envp[i]);
		if (!shell->env[i])
			return (env_error("failed to init environment\n", &shell->gc));
		i++;
	}
	shell->env[i] = NULL;
	return (1);
}

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
	shell->env = gc_malloc(&shell->gc, sizeof(char *) * (env_vars + 1),
			GC_FATAL, NULL);
	if (!shell->env)
		return (NULL);
	if (!duplicate_env_vars(envp, shell))
		return (NULL);
	return (shell->env);
}
