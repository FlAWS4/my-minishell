/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 02:01:51 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if character is a whitespace
 */
static int	is_space(char c)
{
    return (c == ' ' || (c >= 9 && c <= 13));
}

/**
 * Skip whitespace in a string
 */
static int	skip_whitespace(const char *str)
{
    int	i;

    i = 0;
    while (str[i] && is_space(str[i]))
        i++;
    return (i);
}

/**
 * Check if string is a valid number
 * Handles NULL input and leading/trailing spaces
 */
static int	is_numeric(char *str)
{
    int	i;

    if (!str || !*str)
        return (0);
    i = skip_whitespace(str);
    if (str[i] == '+' || str[i] == '-')
        i++;
    if (!str[i])
        return (0);
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Check for integer overflow during conversion
 */
static int	check_overflow(long result, char digit, int sign)
{
    if ((result > LONG_MAX / 10) || 
        (result == LONG_MAX / 10 && (digit - '0') > LONG_MAX % 10))
    {
        return (sign == 1) ? 1 : -1;
    }
    return (0);
}

/**
 * Process digits for atol conversion
 */
static long	process_digits(const char *str, int i, int sign)
{
    long	result;
    int		overflow;

    result = 0;
    while (str[i] >= '0' && str[i] <= '9')
    {
        overflow = check_overflow(result, str[i], sign);
        if (overflow == 1)
            return (LONG_MAX);
        if (overflow == -1)
            return (LONG_MIN);
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result * sign);
}

/**
 * Convert string to long integer with overflow detection
 */
static long	ft_atol(const char *str)
{
    int	sign;
    int	i;

    if (!str)
        return (0);
    sign = 1;
    i = skip_whitespace(str);
    if (str[i] == '-' || str[i] == '+')
    {
        if (str[i] == '-')
            sign = -1;
        i++;
    }
    return (process_digits(str, i, sign));
}

/**
 * Handle numeric argument for exit
 */
static int	handle_exit_arg(t_shell *shell, t_cmd *cmd)
{
    int	exit_code;

    if (!is_numeric(cmd->args[1]))
    {
        display_error(ERROR_EXIT, cmd->args[1], "numeric argument required");
        shell->exit_status = 2;
        shell->should_exit = 1;
        return (0);
    }
    else if (cmd->args[2])
    {
        display_error(ERROR_EXIT, NULL, "too many arguments");
        return (1);
    }
    else
    {
        exit_code = (unsigned char)ft_atol(cmd->args[1]);
        shell->exit_status = exit_code;
        shell->should_exit = 1;
    }
    return (0);
}

/**
 * Built-in exit command
 * Exits the shell with specified status code
 */
int	builtin_exit(t_shell *shell, t_cmd *cmd)
{
    int	exit_code;

    if (!shell || !cmd || !cmd->args)
        return (1);
    exit_code = shell->exit_status;
    ft_putendl_fd("exit", STDOUT_FILENO);
    if (cmd->args[1])
        return (handle_exit_arg(shell, cmd));
    shell->exit_status = exit_code;
    shell->should_exit = 1;
    return (0);
}
