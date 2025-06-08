/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:12 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 20:10:24 by my42             ###   ########.fr       */
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
    else if (error_type == ERR_PIPE)
        return (BOLD_BLUE "📤 Pipe Error" RESET ": ");
    else if (error_type == ERR_FORK)
        return (BOLD_YELLOW "🔀 Fork Error" RESET ": ");
    else if (error_type == ERR_EXEC)
        return (BOLD_RED "⚙️ Execution Error" RESET ": ");
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
    if (!command && !message)
        return;
    prefix = get_error_prefix(error_type);
    ft_putstr_fd(prefix, 2);
    if (command && command[0] != '\0')
    {
        ft_putstr_fd(command, 2);
        ft_putstr_fd(": ", 2);
    }
    if (message)
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
    if (error_type == ERROR_COMMAND || error_type == ERR_NOT_FOUND)
        return (127);  // Command not found
    else if (error_type == ERROR_PERMISSION)
        return (126);  // Permission denied
    else if (error_type == ERROR_SYNTAX)
        return (2);    // Syntax error
    else if (error_type == ERR_REDIR)
        return (1);    // General error
    else if (error_type == ERROR_MEMORY)
        return (12);   // Out of memory
    else if (error_type == ERR_EXEC)
        return (126);  // Execution error
    else if (error_type == ERR_PIPE || error_type == ERR_FORK)
        return (1);    // General error for pipe/fork
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

/**
 * Print syntax error for tokens
 */
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

/**
 * Handle execution errors and set appropriate exit status
 * Returns the appropriate exit status
 */
int handle_execution_error(t_shell *shell, char *cmd, char *message, int error_type)
{
    display_error(error_type, cmd, message);
    
    if (shell)
        shell->exit_status = get_error_exit_status(error_type);
        
    return (get_error_exit_status(error_type));
}

/**
 * Handle redirection errors specifically
 * Returns 1 to indicate error occurred
 */
int handle_redirection_error(t_shell *shell, char *filename, char *message)
{
    char err_msg[256];
    
    if (!message)
    {
        if (errno == ENOENT)
            message = "No such file or directory";
        else if (errno == EACCES)
            message = "Permission denied";
        else if (errno == EISDIR)
            message = "Is a directory";
        else
            message = strerror(errno);
    }
    
    if (filename && message)
    {
        snprintf(err_msg, sizeof(err_msg), "%s", message);
        display_error(ERR_REDIR, filename, err_msg);
    }
    
    if (shell)
        shell->exit_status = 1;
    
    return (1);
}

/**
 * Handle memory allocation errors
 * Prints error message and exits with error code 12
 */
void handle_memory_error(t_shell *shell, char *location)
{
    if (location)
        display_error(ERROR_MEMORY, location, "Memory allocation failed");
    else
        display_error(ERROR_MEMORY, "malloc", "Memory allocation failed");
    
    if (shell)
        shell->exit_status = 12;
        
    exit(12);  // Standard practice is to exit on critical memory errors
}

/**
 * Print error and exit with appropriate status
 * Useful for fatal errors
 */
void print_error_and_exit(t_shell *shell, int error_type, char *cmd, char *message)
{
    int exit_status;
    
    display_error(error_type, cmd, message);
    exit_status = get_error_exit_status(error_type);
    
    if (shell)
        shell->exit_status = exit_status;
        
    exit(exit_status);
}

/**
 * Handle pipe errors specifically
 * Returns 1 to indicate error occurred
 */
int handle_pipe_error(t_shell *shell, char *context)
{
    if (context)
        display_error(ERR_PIPE, context, strerror(errno));
    else
        display_error(ERR_PIPE, "pipe", strerror(errno));
    
    if (shell)
        shell->exit_status = 1;
        
    return (1);
}

/**
 * Handle fork errors specifically
 * Returns 1 to indicate error occurred
 */
int handle_fork_error(t_shell *shell, char *context)
{
    if (context)
        display_error(ERR_FORK, context, strerror(errno));
    else
        display_error(ERR_FORK, "fork", strerror(errno));
    
    if (shell)
        shell->exit_status = 1;
        
    return (1);
}
