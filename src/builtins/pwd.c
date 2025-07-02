/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:47:07 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:47:07 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in command that prints the current working directory
 * Uses PWD environment variable when available to preserve symlinks
 */
int	builtin_pwd(t_shell *shell)
{
    char	*cwd;
    char	*env_pwd;

    env_pwd = get_env_value(shell, "PWD");
    if (env_pwd)
    {
        ft_putendl_fd(env_pwd, STDOUT_FILENO);
        g_exit_status = 0;
        return (0);
    }

    cwd = getcwd(NULL, 0);
    if (!cwd)
    {
        error("pwd", NULL, strerror(errno));
        g_exit_status = 1;
        return (1);
    }
    ft_putendl_fd(cwd, STDOUT_FILENO);
    free(cwd);
    g_exit_status = 0;
    return (0);
}
