/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_handling.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:12 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 00:35:51 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Remove colon prefix from command name (for quoted variables)
 */
static char	*clean_command_name(char *cmd)
{
    if (!cmd)
        return (NULL);
    if (cmd[0] == ':')
        return (cmd + 1);
    return (cmd);
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
    else if (error_type == ERROR_CD)
        return (BOLD_CYAN "📁 Directory Error" RESET ": ");
    else
        return (BOLD_RED "Error" RESET ": ");
}

/**
 * Get command color and icon based on command type
 */
static void	get_cmd_style(char *cmd, char **color, char **icon)
{
    if (!cmd)
    {
        *color = BOLD_YELLOW;
        *icon = "📦";
        return ;
    }
    if (ft_strcmp(cmd, "cd") == 0)
    {
        *color = BOLD_CYAN;
        *icon = "📂";
    }
    else if (ft_strcmp(cmd, "echo") == 0)
    {
        *color = BOLD_GREEN;
        *icon = "📢";
    }
    else if (ft_strcmp(cmd, "export") == 0)
    {
        *color = BOLD_BLUE;
        *icon = "🔄";
    }
    else if (ft_strcmp(cmd, "unset") == 0)
    {
        *color = BOLD_MAGENTA;
        *icon = "🗑️";
    }
    else if (ft_strcmp(cmd, "env") == 0)
    {
        *color = BOLD_CYAN;
        *icon = "🌍";
    }
    else if (ft_strcmp(cmd, "exit") == 0)
    {
        *color = BOLD_RED;
        *icon = "🚪";
    }
    else if (ft_strcmp(cmd, "pwd") == 0)
    {
        *color = BOLD_BLUE;
        *icon = "📂";
    }
    else
    {
        *color = BOLD_YELLOW;
        *icon = "📦";
    }
}

/**
 * Display builtin command error with styling
 */
static void	display_builtin_error(char *cmd, char *arg, char *message)
{
    char	*color;
    char	*icon;
    char	*clean_arg;

    clean_arg = clean_command_name(arg);
    get_cmd_style(cmd, &color, &icon);
    ft_putstr_fd(color, 2);
    ft_putstr_fd(icon, 2);
    ft_putstr_fd(" minishell" RESET ": ", 2);
    ft_putstr_fd(color, 2);
    ft_putstr_fd(cmd, 2);
    ft_putstr_fd(RESET, 2);
    if (clean_arg && clean_arg[0] != '\0')
    {
        ft_putstr_fd(": " BOLD_WHITE, 2);
        ft_putstr_fd(clean_arg, 2);
        ft_putstr_fd(RESET ": ", 2);
    }
    else
        ft_putstr_fd(": ", 2);
    if (message)
    {
        ft_putstr_fd(BOLD_RED, 2);
        ft_putstr_fd(message, 2);
        ft_putstr_fd(RESET, 2);
    }
    ft_putstr_fd("\n", 2);
}

/**
 * Display command not found error
 */
static void display_not_found_error(char *command)
{
    char *prefix;
    char *display_cmd;

    // Make sure we have a valid command
    if (!command)
    {
        ft_putstr_fd(BOLD_RED "🔍 Command Not Found" RESET ": command not found\n", 2);
        return;
    }
    
    // Skip any colon or quote at the start
    display_cmd = command;
    while (*display_cmd && (*display_cmd == ':' || *display_cmd == '"' || *display_cmd == '\''))
        display_cmd++;
        
    prefix = BOLD_RED "🔍 Command Not Found" RESET ": ";
    ft_putstr_fd(prefix, 2);
    ft_putstr_fd(display_cmd, 2);
    ft_putstr_fd(": command not found\n", 2);
}

/**
 * Display error message with formatting
 */
void	display_error(int error_type, char *command, char *message)
{
    char	*prefix;
    char	*clean_cmd;

    clean_cmd = clean_command_name(command);
    if (error_type == ERROR_CD)
        return (display_builtin_error("cd", clean_cmd, message));
    else if (error_type == ERROR_ECHO)
        return (display_builtin_error("echo", clean_cmd, message));
    else if (error_type == ERROR_EXPORT)
        return (display_builtin_error("export", clean_cmd, message));
    else if (error_type == ERROR_UNSET)
        return (display_builtin_error("unset", clean_cmd, message));
    else if (error_type == ERROR_ENV)
        return (display_builtin_error("env", clean_cmd, message));
    else if (error_type == ERROR_EXIT)
        return (display_builtin_error("exit", clean_cmd, message));
    else if (error_type == ERROR_PWD)
        return (display_builtin_error("pwd", clean_cmd, message));
    if (error_type == ERROR_NOT_FOUND || error_type == ERR_NOT_FOUND)
        return (display_not_found_error(clean_cmd));
    prefix = get_error_prefix(error_type);
    ft_putstr_fd(prefix, 2);
    if (clean_cmd && clean_cmd[0] != '\0')
    {
        ft_putstr_fd(clean_cmd, 2);
        ft_putstr_fd(": ", 2);
    }
    if (message)
        ft_putstr_fd(message, 2);
    ft_putstr_fd("\n", 2);
}

/**
 * Print error message with automatic error type detection
 */
void print_error(char *cmd, char *msg)
{
    int error_type;

    if (!cmd || !msg)
        return;
    error_type = 0;
    if (ft_strstr(msg, "not found"))
        error_type = ERROR_COMMAND;
    else if (ft_strstr(msg, "permission"))
        error_type = ERROR_PERMISSION;
    else if (ft_strstr(msg, "syntax"))
        error_type = ERROR_SYNTAX;
    else if (ft_strstr(msg, "cannot allocate") || 
             ft_strstr(msg, "memory") || 
             ft_strstr(msg, "malloc"))
        error_type = ERROR_MEMORY;
    else if (ft_strstr(msg, "redirect") ||
             ft_strstr(msg, "file") || 
             ft_strstr(msg, "directory"))
        error_type = ERR_REDIR;
    display_error(error_type, cmd, msg);
}

/**
 * Get appropriate exit status based on error type
 */
int	get_error_exit_status(int error_type)
{
    if (error_type == ERROR_COMMAND || error_type == ERR_NOT_FOUND)
        return (127);
    else if (error_type == ERROR_PERMISSION)
        return (126);
    else if (error_type == ERROR_SYNTAX)
        return (2);
    else if (error_type == ERR_REDIR)
        return (1);
    else if (error_type == ERROR_MEMORY)
        return (12);
    else if (error_type == ERR_EXEC)
        return (126);
    else if (error_type == ERR_PIPE || error_type == ERR_FORK)
        return (1);
    else
        return (1);
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

/**
 * Print syntax error for tokens
 */
void	print_syntax_error(char *token_value, int token_type)
{
    char	*prefix;
    char	*suffix;

    prefix = "unexpected ";
    if (!token_value)
        token_value = "unexpected token";
    if (token_type == TOKEN_PIPE)
        suffix = "pipe operator '";
    else if (token_type == TOKEN_REDIR_IN || token_type == TOKEN_REDIR_OUT
        || token_type == TOKEN_REDIR_APPEND || token_type == TOKEN_HEREDOC)
        suffix = "redirection '";
    else
        suffix = "token '";
    display_error(ERROR_SYNTAX, "syntax error", NULL);
    ft_putstr_fd(prefix, 2);
    ft_putstr_fd(suffix, 2);
    ft_putstr_fd(token_value, 2);
    ft_putstr_fd("'\n", 2);
}

/**
 * Set error message based on errno
 */
static char	*get_errno_message(void)
{
    if (errno == ENOENT)
        return ("No such file or directory");
    else if (errno == EACCES)
        return ("Permission denied");
    else if (errno == EISDIR)
        return ("Is a directory");
    else
        return (strerror(errno));
}

/**
 * Handle execution errors and set appropriate exit status
 */
int	handle_execution_error(t_shell *shell, char *cmd, char *message,
        int error_type)
{
    display_error(error_type, cmd, message);
    if (shell)
        shell->exit_status = get_error_exit_status(error_type);
    return (get_error_exit_status(error_type));
}

/**
 * Handle redirection errors specifically
 */
int	handle_redirection_error(t_shell *shell, char *filename, char *message)
{
    if (!message)
        message = get_errno_message();
    if (filename && message)
    {
        display_error(ERR_REDIR, filename, message);
    }
    if (shell)
        shell->exit_status = 1;
    return (1);
}

/**
 * Handle memory allocation errors
 */
void	handle_memory_error(t_shell *shell, char *location)
{
    if (location)
        display_error(ERROR_MEMORY, location, "Memory allocation failed");
    else
        display_error(ERROR_MEMORY, "malloc", "Memory allocation failed");
    if (shell)
        shell->exit_status = 12;
    exit(12);
}

/**
 * Print error and exit with appropriate status
 */
void	print_error_and_exit(t_shell *shell, int error_type, char *cmd,
        char *message)
{
    int	exit_status;

    display_error(error_type, cmd, message);
    exit_status = get_error_exit_status(error_type);
    if (shell)
        shell->exit_status = exit_status;
    exit(exit_status);
}

/**
 * Handle pipe errors specifically
 */
int	handle_pipe_error(t_shell *shell, char *context)
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
 */
int	handle_fork_error(t_shell *shell, char *context)
{
    if (context)
        display_error(ERR_FORK, context, strerror(errno));
    else
        display_error(ERR_FORK, "fork", strerror(errno));
    if (shell)
        shell->exit_status = 1;
    return (1);
}

/**
 * Display heredoc EOF warning
 */
void display_heredoc_eof_warning(char *delimiter)
{
    ft_putstr_fd(BOLD_YELLOW "⚠ Warning" RESET ": here-document delimited by end-of-file (wanted `", STDERR_FILENO);
    ft_putstr_fd(BOLD_WHITE, STDERR_FILENO);
    ft_putstr_fd(delimiter, STDERR_FILENO);
    ft_putstr_fd(RESET "')\n", STDERR_FILENO);
}
/**
 * Display SHLVL warning when value gets too high
 */
void display_shlvl_warning(int level)
{
    char    *level_str;
    
    ft_putstr_fd(BOLD_YELLOW "⚠ Warning" RESET ": shell level (", STDERR_FILENO);
    
    // Convert int to string using ft_itoa
    level_str = ft_itoa(level);
    if (level_str)
    {
        ft_putstr_fd(BOLD_WHITE, STDERR_FILENO);
        ft_putstr_fd(level_str, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        free(level_str);
    }
    else
        ft_putstr_fd("???", STDERR_FILENO); // Fallback if allocation fails
    
    ft_putstr_fd(") too high, resetting to 1\n", STDERR_FILENO);
}
