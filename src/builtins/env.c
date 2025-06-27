/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:33:31 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


int	env_has_path(char **envp)
{
	int	i;

	i = 0;
	if (!envp || !*envp)
		return (0);
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "PATH=", 5) == 0)
			return (1);
		i++;
	}
	return (0);
}

char	*get_env_value(t_shell *shell, const char *var_name)
{
	int		i;
	size_t	var_len;

	i = 0;
	if (!shell || !shell->env || !var_name || !*var_name)
		return (NULL);
	var_len = ft_strlen(var_name);
	while (shell->env[i])
	{
		if (ft_strncmp(shell->env[i], var_name, var_len) == 0
			&& shell->env[i][var_len] == '=')
			return (shell->env[i] + var_len + 1);
		i++;
	}
	return (NULL);
}

int	find_var_pos(char *var_name, t_shell *shell)
{
	int		i;
	size_t	len;

	i = 0;
	if (!shell || !shell->env || !var_name || !*var_name)
		return (-1);
	len = ft_strlen(var_name);
	while (shell->env[i])
	{
		if (!ft_strncmp(shell->env[i], var_name, len)
			&& (shell->env[i][len] == '=' || shell->env[i][len] == '\0'))
			return (i);
		i++;
	}
	return (-1);
}

void	sort_env_for_export(char **env_copy)
{
	int		i;
	int		j;
	char	*temp;

	i = 0;
	if (!env_copy)
		return ;
	while (env_copy[i + 1])
	{
		j = 0;
		while (env_copy[j + 1])
		{
			if (ft_strcmp(env_copy[j], env_copy[j + 1]) > 0)
			{
				temp = env_copy[j];
				env_copy[j] = env_copy[j + 1];
				env_copy[j + 1] = temp;
			}
			j++;
		}
		i++;
	}
}

int	builtin_env(t_shell *shell, t_command *cmd)
{
	int	i;

	i = 0;
	if (!shell || !cmd)
		return (error("env", NULL, "internal error"), 1);
	if (!shell->env)
		return (error("env", NULL, "environment not found"), 1);
	if (cmd->args[1])
	{
		error("env", NULL, "options not supported");
		return (1);
	}
	while (shell->env[i])
	{
		if (ft_strchr(shell->env[i], '='))
		{
			ft_putstr_fd(shell->env[i], STDOUT_FILENO);
			ft_putchar_fd('\n', STDOUT_FILENO);
		}
		i++;
	}
	return (0);
}
