/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/17 20:50:05 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in pwd command
 */
int	builtin_pwd(t_shell *shell)
{
    char	*pwd;
    char	cwd[1024];

    (void)shell;
    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putstr_fd(cwd, 1);
        ft_putstr_fd("\n", 1);
        return (0);
    }
    pwd = get_env_value(shell->env, "PWD");
    if (pwd)
    {
        ft_putstr_fd(pwd, 1);
        ft_putstr_fd("\n", 1);
        return (0);
    }
    ft_putstr_fd("pwd: error retrieving current directory\n", 2);
    return (1);
}
