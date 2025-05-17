/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/17 20:49:48 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if the argument is the -n flag
 */
static int	is_n_flag(char *arg)
{
    int	i;

    if (!arg || arg[0] != '-')
        return (0);
    i = 1;
    while (arg[i])
    {
        if (arg[i] != 'n')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Built-in echo command
 */
int	builtin_echo(t_cmd *cmd)
{
    int	i;
    int	n_flag;

    i = 1;
    n_flag = 0;
    while (cmd->args[i] && is_n_flag(cmd->args[i]))
    {
        n_flag = 1;
        i++;
    }
    while (cmd->args[i])
    {
        ft_putstr_fd(cmd->args[i], 1);
        if (cmd->args[i + 1])
            ft_putstr_fd(" ", 1);
        i++;
    }
    if (!n_flag)
        ft_putstr_fd("\n", 1);
    return (0);
}
