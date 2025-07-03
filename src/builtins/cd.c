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

/**
 * Creates logical path for symlinks
 */
static	char	*create_symlink_path(char *oldpwd, char *target)
{
	char	*logical_path;
	char	*temp;

	if (target[0] == '/')
		logical_path = ft_strdup(target);
	else if (oldpwd)
	{
		if (oldpwd[ft_strlen(oldpwd) - 1] == '/')
			logical_path = ft_strjoin(oldpwd, target);
		else
		{
			temp = ft_strjoin(oldpwd, "/");
			logical_path = ft_strjoin(temp, target);
			free(temp);
		}
	}
	else
		logical_path = getcwd(NULL, 0);
	return (logical_path);
}

/** * Updates PWD and OLDPWD environment variables with logical path
 * If the target is a symlink, it creates a logical path based on the old PWD.
 * If not, it uses getcwd to get the current working directory.
 */
static	int	update_pwd_vars_with_logical_path(t_shell *shell, char *target)
{
	char	*oldpwd;
	char	*logical_path;
	int		is_symlink;
	int		pwd_was_set;

	pwd_was_set = (get_env_value(shell, "PWD") != NULL);
	oldpwd = get_env_value(shell, "PWD");
	is_symlink = check_symlink(target, oldpwd, target);
	if (is_symlink)
		logical_path = create_symlink_path(oldpwd, target);
	else
		logical_path = getcwd(NULL, 0);
	if (!logical_path)
	{
		error("getcwd", NULL, strerror(errno));
		g_exit_status = 1;
		return (1);
	}
	if (oldpwd)
		update_env(shell, "OLDPWD", oldpwd);
	if (pwd_was_set)
		update_env(shell, "PWD", logical_path);
	free(logical_path);
	return (0);
}

/**
 * Validates cd command arguments
 */
static int	validate_cd_args(t_shell *shell, t_command *cmd, int *count_args)
{
	*count_args = 0;
	if (!shell || !cmd || !cmd->args)
	{
		error("cd", NULL, "internal error");
		g_exit_status = 1;
		return (1);
	}
	while (cmd->args[*count_args])
		(*count_args)++;
	if (*count_args > 2)
	{
		error("cd", NULL, "too many arguments");
		g_exit_status = 1;
		return (1);
	}
	return (0);
}

/**
 * Handles special case for // paths
 */
static int	handle_double_slash(t_shell *shell, char *oldpwd, char *target)
{
	if (target && ft_strcmp(target, "//") == 0)
	{
		if (oldpwd)
			update_env(shell, "OLDPWD", oldpwd);
		update_env(shell, "PWD", "//");
		return (1);
	}
	return (0);
}

/**
 * Built-in command to change the current directory
 */
int	builtin_cd(t_shell *shell, t_command *cmd)
{
	char	*target;
	int		count_args;
	char	*oldpwd;

	if (validate_cd_args(shell, cmd, &count_args))
		return (1);
	target = get_cd_destination(shell, cmd->args[1]);
	if (!target)
		return (1);
	oldpwd = get_env_value(shell, "PWD");
	if (chdir(target) == -1)
	{
		print_cd_error(target);
		return (1);
	}
	if (handle_double_slash(shell, oldpwd, target))
		return (0);
	return (update_pwd_vars_with_logical_path(shell, target));
}
