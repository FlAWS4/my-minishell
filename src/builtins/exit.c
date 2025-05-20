/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 20:41:29 by mshariar         ###   ########.fr       */
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
    int	count;
    int	status;

    ft_putstr_fd("exit\n", 2);
    count = 0;
    while (cmd->args[count])
        count++;
    if (count == 1)
        exit(shell->exit_status);
    if (!is_numeric(cmd->args[1]))
    {
        ft_putstr_fd("minishell: exit: ", 2);
        ft_putstr_fd(cmd->args[1], 2);
        ft_putstr_fd(": numeric argument required\n", 2);
        exit(255);
    }
    if (count > 2)
    {
        ft_putstr_fd("minishell: exit: too many arguments\n", 2);
        return (1);
    }
    status = (unsigned char)ft_atol(cmd->args[1]);
    exit(status);
}
