/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 04:48:04 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int		builtin_pwd(t_shell *shell)
{
	char	*cwd;
	char	*env_pwd;

	(void)shell;
	cwd = getcwd(NULL, 0);
	if (!cwd)
	{
		error("pwd", NULL, strerror(errno));
		g_exit_status = 1;
		return (1);
	}
	env_pwd = get_env_value(shell, "PWD");
	if (env_pwd && ft_strcmp(env_pwd, "//") == 0 && ft_strcmp(cwd, "/") == 0)
		ft_putendl_fd(env_pwd, STDOUT_FILENO);
	else
		ft_putendl_fd(cwd, STDOUT_FILENO);
	free(cwd);
	g_exit_status = 0;
	return (0);
}
