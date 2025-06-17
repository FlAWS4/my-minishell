/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   echo.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 02:04:10 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if the argument is the -n flag
 * Valid flags are -n, -nn, -nnn, etc.
 */
static int	is_n_flag(char *arg)
{
    int	i;

    if (!arg || arg[0] != '-')
        return (0);
    i = 1;
    if (arg[i] == '\0')
        return (0);
    while (arg[i])
    {
        if (arg[i] != 'n')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Print arguments with spaces between them
 */
static void	print_args(char **args, int start_idx)
{
    int	i;

    i = start_idx;
    while (args[i])
    {
        ft_putstr_fd(args[i], STDOUT_FILENO);
        if (args[i + 1])
            ft_putchar_fd(' ', STDOUT_FILENO);
        i++;
    }
}

/**
 * Built-in echo command
 * Handles the -n option to suppress trailing newline
 */
int	builtin_echo(t_cmd *cmd)
{
    int	i;
    int	n_flag;

    if (!cmd || !cmd->args)
    {
        display_error(ERROR_ECHO, NULL, "Invalid command structure");
        return (1);
    }
    i = 1;
    n_flag = 0;
    while (cmd->args[i] && is_n_flag(cmd->args[i]))
    {
        n_flag = 1;
        i++;
    }
    print_args(cmd->args, i);
    if (!n_flag)
        ft_putchar_fd('\n', STDOUT_FILENO);
    return (0);
}
