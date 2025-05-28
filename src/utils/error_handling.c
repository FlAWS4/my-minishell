/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:12 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:50:05 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if string contains substring
 */
static int	str_contains(const char *haystack, const char *needle)
{
    int	i;
    int	j;

    if (!haystack || !needle)
        return (0);
    i = 0;
    while (haystack[i])
    {
        j = 0;
        while (needle[j] && haystack[i + j] && haystack[i + j] == needle[j])
            j++;
        if (!needle[j])
            return (1);
        i++;
    }
    return (0);
}

/**
 * Get error prefix based on error type
 */
static char	*get_error_prefix(int error_type)
{
    if (error_type == ERROR_SYNTAX)
        return (BOLD_RED "✘ Syntax Error" RESET ": ");
    else if (error_type == ERROR_COMMAND)
        return (BOLD_YELLOW "⚠ Command Error" RESET ": ");
    else if (error_type == ERROR_PERMISSION)
        return (BOLD_MAGENTA "🔒 Permission Denied" RESET ": ");
    else
        return (BOLD_RED "Error" RESET ": ");
}

/**
 * Display visually enhanced error message
 */
void	display_error(int error_type, char *command, char *message)
{
    char	*prefix;

    prefix = get_error_prefix(error_type);
    ft_putstr_fd(prefix, 2);
    ft_putstr_fd(command, 2);
    ft_putstr_fd(" - ", 2);
    ft_putstr_fd(message, 2);
    ft_putstr_fd("\n", 2);
}

/**
 * Print error message to stderr
 */
void	print_error(char *cmd, char *msg)
{
    int	error_type;

    error_type = 0;
    if (str_contains(msg, "not found"))
        error_type = ERROR_COMMAND;
    else if (str_contains(msg, "permission"))
        error_type = ERROR_PERMISSION;
    else if (str_contains(msg, "syntax"))
        error_type = ERROR_SYNTAX;
    display_error(error_type, cmd, msg);
}

/**
 * Free array of strings
 */
void	free_str_array(char **array)
{
    int	i;
    
    if (!array)
        return ;
    
    i = 0;
    while (array[i])
    {
        free(array[i]);
        i++;
    }
    free(array);
}
