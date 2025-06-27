/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:46:32 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:46:32 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	print_cd_error(char *target)
{
	error("cd", target, strerror(errno));
	g_exit_status = 1;
}

static int	update_pwd_vars(t_shell *shell)
{
	char	*oldpwd;
	char	*newpwd;

	newpwd = getcwd(NULL, 0);
	if (!newpwd)
	{
		error("getcwd", NULL, strerror(errno));
		g_exit_status = 1;
		return (1);
	}
	oldpwd = get_env_value(shell, "PWD");
	if (oldpwd)
		update_env(shell, "OLDPWD", oldpwd);
	update_env(shell, "PWD", newpwd);
	free(newpwd);
	return (0);
}

static char	*get_home_or_oldpwd(t_shell *shell, int is_oldpwd)
{
	char	*value;
	char	*var_name;

	if (is_oldpwd)
		var_name = "OLDPWD";
	else
		var_name = "HOME";
	value = get_env_value(shell, var_name);
	if (!value)
		error("cd", var_name, "not set");
	return (value);
}

static char	*get_cd_destination(t_shell *shell, char *arg)
{
	char	*target;

	if (!arg || arg[0] == '\0')
		return (get_home_or_oldpwd(shell, 0));
	if (ft_strcmp(arg, "-") == 0)
	{
		target = get_home_or_oldpwd(shell, 1);
		if (target)
			ft_putendl_fd(target, STDOUT_FILENO);
		return (target);
	}
	return (arg);
}

int		builtin_cd(t_shell *shell, t_command *cmd)
{
	char	*target;
	int		count_args;
	char	*oldpwd;

	count_args = 0;
	if (!shell || !cmd || !cmd->args)
		return (error("cd", NULL, "internal error"), g_exit_status = 1, 1);
	while (cmd->args[count_args])
		count_args++;
	if (count_args > 2)
		return (error("cd", NULL, "too many arguments"), g_exit_status = 1, 1);
	target = get_cd_destination(shell, cmd->args[1]);
	if (!target)
		return (1);
	oldpwd = get_env_value(shell, "PWD");
	if (chdir(target) == -1)
		return (print_cd_error(target), 1);
	if (target && ft_strcmp(target, "//") == 0)
	{
		if (oldpwd)
			update_env(shell, "OLDPWD", oldpwd);
		update_env(shell, "PWD", "//");
		return (0);
	}
	return (update_pwd_vars(shell));
}
