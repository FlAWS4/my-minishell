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
 * Display an error message for cd command failures
 */
void	print_cd_error(char *target)
{
	error("cd", target, strerror(errno));
	g_exit_status = 1;
}

/**
 * Retrieves HOME or OLDPWD environment variable
 */
char	*get_home_or_oldpwd(t_shell *shell, int is_oldpwd)
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

/**
 * Determines the target directory for cd
 */
char	*get_cd_destination(t_shell *shell, char *arg)
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

/**
 * Creates full path for a relative path
 */
char	*create_full_path(char *oldpwd, char *target)
{
	char	*temp;
	char	*full_path;

	if (oldpwd[ft_strlen(oldpwd) - 1] == '/')
		full_path = ft_strjoin(oldpwd, target);
	else
	{
		temp = ft_strjoin(oldpwd, "/");
		full_path = ft_strjoin(temp, target);
		free(temp);
	}
	return (full_path);
}

/**
 * Checks if a path is a symlink
 */
int	check_symlink(char *path, char *oldpwd, char *target)
{
	struct stat		path_stat;
	char			*full_path;
	int				is_symlink;

	is_symlink = 0;
	(void)path;
	if (target[0] != '/' && oldpwd)
	{
		full_path = create_full_path(oldpwd, target);
		if (full_path)
		{
			if (lstat(full_path, &path_stat) == 0)
				is_symlink = S_ISLNK(path_stat.st_mode);
			free(full_path);
		}
	}
	else if (lstat(target, &path_stat) == 0)
		is_symlink = S_ISLNK(path_stat.st_mode);
	return (is_symlink);
}
