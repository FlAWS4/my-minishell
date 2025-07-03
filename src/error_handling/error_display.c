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

/**
 * display_error_and_exit - Display error message and exit the shell
 * @shell: Shell context containing resources to clean up
 * @cmd: Command that caused the error
 * @msg: Error message to display
 * @exit_code: Exit status code to return
 *
 * Displays an error message and then terminates the shell with the specified
 * exit code after cleaning up resources.
 */
void	display_error_and_exit(t_shell *shell, const char *cmd, const char *msg,
	int exit_code)
{
	error(NULL, cmd, msg);
	clean_and_exit_shell(shell, exit_code);
}

/**
 * is_fd_is_fd_writable - Check if a file descriptor is is_fd_writable
 * @fd: File descriptor to check
 * @cmd_name: Command name to include in 
 * error message if fd is not is_fd_writable
 * Attempts to write an empty string to the file descriptor to verify
 * it's is_fd_writable. Displays an error using the system error message if not.
 *
 * Return: 1 if is_fd_writable, 0 otherwise
 */
int	is_fd_writable(int fd, const char *cmd_name)
{
	if (write(fd, "", 0) == -1)
	{
		error(cmd_name, NULL, strerror(errno));
		return (0);
	}
	return (1);
}
