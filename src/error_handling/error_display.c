/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_display.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:41:13 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:41:13 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * error - Display formatted error message with color highlighting
 * @cmd: Command that caused the error (can be NULL)
 * @error_item: Item causing the error (can be NULL)
 * @msg: Error message to display
 *
 * Displays a formatted error message with color-coded components to improve
 * readability and error visibility.
 */
void	error(const char *cmd, const char *error_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    if (error_item)
    {
        ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
        ft_putstr_fd(error_item, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(BOLD_RED, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}

/**
 * error_quoted - Display error message with quoted item and color highlighting
 * @cmd: Command that caused the error (can be NULL)
 * @error_item: Item causing the error to be displayed in quotes (can be NULL)
 * @msg: Error message to display
 *
 * Similar to error() but puts the error_item in quotes for better visibility
 * of whitespace or special characters in the problematic item.
 */
void	error_quoted(const char *cmd, const char *error_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    if (error_item)
    {
        ft_putstr_fd(BOLD_YELLOW, STDERR_FILENO);
        ft_putchar_fd('\'', STDERR_FILENO);
        ft_putstr_fd(error_item, STDERR_FILENO);
        ft_putchar_fd('\'', STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(BOLD_RED, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}


/**
 * warning - Display warning message with color formatting
 * @cmd: Command related to the warning (can be NULL)
 * @warning_item: Item that triggered the warning (can be NULL)
 * @msg: Warning message to display
 *
 * Displays a formatted warning message with yellow highlighting
 * to indicate non-critical issues.
 */
void	warning(const char *cmd, const char *warning_item, const char *msg)
{
    ft_putstr_fd(BOLD_WHITE "minishell: " RESET, STDERR_FILENO);
    if (cmd)
    {
        ft_putstr_fd(BOLD_BLUE, STDERR_FILENO);
        ft_putstr_fd(cmd, STDERR_FILENO);
        ft_putstr_fd(RESET ": ", STDERR_FILENO);
    }
    ft_putstr_fd(YELLOW "warning: " RESET, STDERR_FILENO);
    if (warning_item)
    {
        ft_putstr_fd(YELLOW, STDERR_FILENO);
        ft_putstr_fd(warning_item, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
        if (msg)
            ft_putstr_fd(": ", STDERR_FILENO);
    }
    if (msg)
    {
        ft_putstr_fd(YELLOW, STDERR_FILENO);
        ft_putstr_fd((char *)msg, STDERR_FILENO);
        ft_putstr_fd(RESET, STDERR_FILENO);
    }
    ft_putchar_fd('\n', STDERR_FILENO);
}


void	handle_cmd_error(t_shell *shell, const char *cmd, const char *msg,
	int exit_code)
{
	error(NULL, cmd, msg);
	clean_and_exit_shell(shell, exit_code);
}

int	writable(int fd, const char *cmd_name)
{
	if (write(fd, "", 0) == -1)
	{
		error(cmd_name, NULL, strerror(errno));
		return (0);
	}
	return (1);
}