/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:12 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 03:15:53 by my42             ###   ########.fr       */
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
    else if (error_type == ERR_REDIR)
        return (BOLD_CYAN "⤵ Redirection Error" RESET ": ");
    else if (error_type == ERROR_MEMORY)
        return (BOLD_RED "🧠 Memory Error" RESET ": ");
    else
        return (BOLD_RED "Error" RESET ": ");
}

/**
 * Display visually enhanced error message
 */
void display_error(int error_type, char *command, char *message)
{
    char *prefix;

    if (error_type == ERR_NOT_FOUND)
    {
        prefix = BOLD_RED "🔍 Command Not Found" RESET ": ";
        ft_putstr_fd(prefix, 2);   
        if (command && command[0] != '\0')
        {
            ft_putstr_fd(command, 2);
            ft_putstr_fd(": command not found\n", 2);
        }
        else
            ft_putstr_fd("command not found\n", 2);
        return;
    }
    if (!command || !message)
        return;
    prefix = get_error_prefix(error_type);
    ft_putstr_fd(prefix, 2);
    if (command && command[0] != '\0')
    {
        ft_putstr_fd(command, 2);
        ft_putstr_fd(": ", 2);
    }
    ft_putstr_fd(message, 2);
    ft_putstr_fd("\n", 2);
}


/**
 * Print error message to stderr with automatic error type detection
 */
void	print_error(char *cmd, char *msg)
{
    int	error_type;

    if (!cmd || !msg)
        return;
        
    error_type = 0;
    if (str_contains(msg, "not found"))
        error_type = ERROR_COMMAND;
    else if (str_contains(msg, "permission"))
        error_type = ERROR_PERMISSION;
    else if (str_contains(msg, "syntax"))
        error_type = ERROR_SYNTAX;
    else if (str_contains(msg, "cannot allocate") || 
             str_contains(msg, "memory") || 
             str_contains(msg, "malloc"))
        error_type = ERROR_MEMORY;
    else if (str_contains(msg, "redirect") || 
             str_contains(msg, "file") ||
             str_contains(msg, "directory"))
        error_type = ERR_REDIR;
        
    display_error(error_type, cmd, msg);
}

/**
 * Get appropriate exit status based on error type
 */
int	get_error_exit_status(int error_type)
{
    if (error_type == ERROR_COMMAND)
        return (127);  // Command not found
    else if (error_type == ERROR_PERMISSION)
        return (126);  // Permission denied
    else if (error_type == ERROR_SYNTAX)
        return (2);    // Syntax error
    else if (error_type == ERR_REDIR)
        return (1);    // General error
    else if (error_type == ERROR_MEMORY)
        return (12);   // Out of memory
    else
        return (1);    // Default error
}

/**
 * Free array of strings
 */
void	free_str_array(char **array)
{
    int	i;
    
    if (!array)
        return;
    
    i = 0;
    while (array[i])
    {
        free(array[i]);
        i++;
    }
    free(array);
}

void print_syntax_error(char *token_value, int token_type)
{
    char error_msg[100];
    
    if (!token_value)
        token_value = "unexpected token";
    
    // Use token_type for more specific error messages if needed
    if (token_type == TOKEN_PIPE)
        snprintf(error_msg, sizeof(error_msg), "unexpected pipe operator '%s'", token_value);
    else if (token_type == TOKEN_REDIR_IN || token_type == TOKEN_REDIR_OUT ||
             token_type == TOKEN_REDIR_APPEND || token_type == TOKEN_HEREDOC)
        snprintf(error_msg, sizeof(error_msg), "unexpected redirection '%s'", token_value);
    else
        snprintf(error_msg, sizeof(error_msg), "unexpected token '%s'", token_value);
        
    display_error(ERROR_SYNTAX, "syntax error", error_msg);
}
