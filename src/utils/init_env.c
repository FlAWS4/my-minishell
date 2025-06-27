/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_env.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:29:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:29:55 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**init_env(char **envp, t_shell *shell)
{
	char	**mini_env;

	if (env_has_path(envp))
	{
		shell->default_path = NULL;
		shell->env = get_env(envp, shell);
		return (shell->env);
	}
	shell->default_path = ft_strdup("/usr/local/bin:/usr/bin:/bin");
	mini_env = init_minimal_env();
	if (!mini_env)
	{
		error("env", NULL, "failed to create minimal environment");
		return (NULL);
	}
	shell->env = get_env(mini_env, shell);
	free_array(mini_env);
	if (!shell->env)
		return (NULL);
	return (shell->env);
}
static void	free_tmp_env_vars(char **tmp, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		free(tmp[i]);
		i++;
	}
}

static void	copy_tmp_to_env(char **dest, char **src, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = NULL;
}

char	**init_minimal_env(void)
{
	char	*tmp[3];
	char	*cwd;
	char	**mini_env;

	cwd = getcwd(NULL, 0);
	if (!cwd)
		return (NULL);
	tmp[0] = ft_strjoin("PWD=", cwd);
	tmp[1] = ft_strdup("SHLVL=1");
	tmp[2] = ft_strdup("_=/usr/bin/env");
	free(cwd);
	if (!tmp[0] || !tmp[1] || !tmp[2])
		return (free_tmp_env_vars(tmp, 3), NULL);
	mini_env = malloc(4 * sizeof(char *));
	if (!mini_env)
		return (free_tmp_env_vars(tmp, 3), NULL);
	copy_tmp_to_env(mini_env, tmp, 3);
	return (mini_env);
}
