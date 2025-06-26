/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:10:45 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * Restores previous signal handlers without clearing buffer
 * 
 * @param old_int   Previous SIGINT handler to restore
 * @param old_quit  Previous SIGQUIT handler to restore
 */
void	restore_signal_handlers(struct sigaction *old_int,
                    struct sigaction *old_quit)
{
    sigaction(SIGINT, old_int, NULL);
    sigaction(SIGQUIT, old_quit, NULL);
}


/**
 * Handles SIGINT (Ctrl+C) during main shell operation
 * 
 * @param sig  Signal number (unused but required by signal API)
 * 
 * Handles interrupt differently based on shell state:
 * - During command execution (status 999): Sets status and prints newline
 * - During command input: Resets line, displays prompt on new line
 */
void	handle_interrupt(int sig)
{
    (void)sig;
    if (g_exit_status == 999)
    {
        g_exit_status = 130;
        write(STDOUT_FILENO, "\n", 1);
        return ;
    }
    g_exit_status = 130;
    write(STDOUT_FILENO, "\n", 1);
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

/**
 * Sets up signal handlers for main shell operation
 * 
 * Configures custom interrupt handler and ignores quit and suspend signals.
 * This provides a more user-friendly shell experience similar to bash.
 */

void	setup_signals(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;
	struct sigaction	sa_tstp;

	sa_int.sa_handler = handle_interrupt;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_int, NULL);
	sa_quit.sa_handler = SIG_IGN;
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = 0;
	sigaction(SIGQUIT, &sa_quit, NULL);
	sa_tstp.sa_handler = SIG_IGN;
	sigemptyset(&sa_tstp.sa_mask);
	sa_tstp.sa_flags = 0;
	sigaction(SIGTSTP, &sa_tstp, NULL);
}
/**
 * Resets all signal handlers to system defaults
 * 
 * Used before executing external programs to ensure they
 * receive default signal behavior.
 */
void	reset_signals_to_default(void)
{
    struct sigaction	sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGTSTP, &sa, NULL);
}


/**
 * Restores standard input and output file descriptors
 * 
 * @param shell  The shell context containing saved descriptors
 * 
 * Used to restore stdin and stdout after command execution
 * or when recovering from errors.
 */
void	restore_standard_fds(t_shell *shell)
{
    dup2(shell->saved_stdin, STDIN_FILENO);
    dup2(shell->saved_stdout, STDOUT_FILENO);
}