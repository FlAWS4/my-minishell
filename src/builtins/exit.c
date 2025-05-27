/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:38:57 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if string is a valid number
 */
static int	is_numeric(char *str)
{
    int	i;

    i = 0;
    if (str[i] == '+' || str[i] == '-')
        i++;
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Convert string to long integer
 */
static long	ft_atol(const char *str)
{
    long	result;
    int		sign;
    int		i;

    result = 0;
    sign = 1;
    i = 0;
    while (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13))
        i++;
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
    while (str[i] >= '0' && str[i] <= '9')
    {
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result * sign);
}

/**
 * Built-in exit command
 */
int	builtin_exit(t_shell *shell, t_cmd *cmd)
{
    int	exit_code;

    exit_code = 0;
    ft_putendl_fd("exit", STDOUT_FILENO);
    if (cmd->args[1])
    {
        if (!is_numeric(cmd->args[1]))
        {
            print_error("exit", "numeric argument required");
            shell->exit_status = 2;
        }
        else if (cmd->args[2])
        {
            print_error("exit", "too many arguments");
            return (1);
        }
        else
            exit_code = (unsigned char)ft_atol(cmd->args[1]);
    }
    shell->exit_status = exit_code;
    shell->should_exit = 1;
    return (0);
}
