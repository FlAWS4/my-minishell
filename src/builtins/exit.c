/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:48 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:43:20 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if string is a valid number
 * Handles NULL input and leading/trailing spaces
 */
static int	is_numeric(char *str)
{
    int	i;

    if (!str || !*str)
        return (0);
        
    i = 0;
    // Skip leading whitespace
    while (str[i] && (str[i] == ' ' || (str[i] >= 9 && str[i] <= 13)))
        i++;
        
    // Handle sign
    if (str[i] == '+' || str[i] == '-')
        i++;
        
    // Must have at least one digit
    if (!str[i])
        return (0);
        
    // Check that all remaining characters are digits
    while (str[i])
    {
        if (str[i] < '0' || str[i] > '9')
            return (0);
        i++;
    }
    return (1);
}

/**
 * Convert string to long integer with overflow detection
 */
static long	ft_atol(const char *str)
{
    long	result;
    int		sign;
    int		i;

    if (!str)
        return (0);
        
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
        // Check for overflow
        if ((result > LONG_MAX / 10) || 
            (result == LONG_MAX / 10 && (str[i] - '0') > LONG_MAX % 10))
        {
            if (sign == 1)
                return (LONG_MAX);
            else
                return (LONG_MIN);
        }
        result = result * 10 + (str[i] - '0');
        i++;
    }
    return (result * sign);
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
    exit_code = shell->exit_status;  // Default to current exit status
    ft_putendl_fd("exit", STDOUT_FILENO);
    if (cmd->args[1])
    {
        if (!is_numeric(cmd->args[1]))
        {
            print_error("exit", "numeric argument required");
            shell->exit_status = 2;
            shell->should_exit = 1;
            return (0);
        }
        else if (cmd->args[2])
        {
            print_error("exit", "too many arguments");
            return (1);  // Don't exit if too many arguments
        }
        else
        {
            // Cast to unsigned char to match bash behavior (0-255 range)
            exit_code = (unsigned char)ft_atol(cmd->args[1]);
        }
    }
    shell->exit_status = exit_code;
    shell->should_exit = 1;
    return (0);
}
