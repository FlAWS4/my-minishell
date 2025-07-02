/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_heredoc.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:03:01 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:05:38 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Sets up signal handlers for heredoc input
 * 
 * @param old_int   Pointer to store previous SIGINT handler
 * @param old_quit  Pointer to store previous SIGQUIT handler
 * 
 * Configures SIGINT to use heredoc-specific handler and
 * ignores SIGQUIT during heredoc input.
 */
void	setup_heredoc_signal_handlers(struct sigaction *old_int,
	struct sigaction *old_quit)
{
	struct sigaction	act_int;
	struct sigaction	act_quit;

	sigemptyset(&act_int.sa_mask);
	act_int.sa_handler = handle_heredoc_interrupt;
	act_int.sa_flags = 0;
	sigaction(SIGINT, &act_int, old_int);
	sigemptyset(&act_quit.sa_mask);
	act_quit.sa_handler = SIG_IGN;
	act_quit.sa_flags = 0;
	sigaction(SIGQUIT, &act_quit, old_quit);
}

/**
 * Signal handler for SIGINT during heredoc input
 * 
 * @param sig  Signal number (unused but required by signal API)
 * 
 * Sets a special exit status and displays ^C to indicate interruption.
 * Uses status 19 as a marker for heredoc interruption.
 */
void	handle_heredoc_interrupt(int sig)
{
	(void)sig;
	g_exit_status = 19;
	write(2, "\n", 1);
}

/**
 * Restores previous signal handlers and clears input buffer
 * 
 * @param old_int   Previous SIGINT handler to restore
 * @param old_quit  Previous SIGQUIT handler to restore
 * 
 * Used after completing a command read operation to restore
 * normal signal handling and clear any buffered input.
 */
void	restore_signals_clear_buffer(struct sigaction *old_int,
					struct sigaction *old_quit)
{
	sigaction(SIGINT, old_int, NULL);
	sigaction(SIGQUIT, old_quit, NULL);
	get_next_line(STDIN_FILENO, 1);
}

/**
 * Displays heredoc EOF warning with delimiter
 * 
 * @param delim  The heredoc delimiter that was expected
 * 
 * Warns user when a heredoc was terminated by EOF rather than
 * the expected delimiter.
 */
void	display_heredoc_eof_warning(char *delim)
{
	if (!delim)
		return ;
	ft_putstr_fd(ERROR_HEREDOC_EOF, 2);
	ft_putstr_fd(delim, 2);
	ft_putstr_fd("')\n", 2);
}
