/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/23 22:12:53 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Signal handler for interactive mode
 */
void	sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = 1;
        write(1, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

/**
 * Signal handler for heredoc
 */
void	sigint_heredoc_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = 1;
        write(1, "\n", 1);
        close(0);  // Close stdin to interrupt the heredoc
    }
}

/**
 * Setup signals for interactive mode
 */
void	setup_signals(void)
{
    struct sigaction	sa;

    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);
}

/**
 * Setup signals for non-interactive mode (child processes)
 */
void setup_signals_noninteractive(void)
{
    // Reset signals to default behavior in child processes
    signal(SIGINT, SIG_DFL);   // Ctrl+C should terminate the process
    signal(SIGQUIT, SIG_DFL);  // Ctrl+\ should generate core dump
    g_signal = 0;              // Reset global signal state
}
/**
 * Setup signals for heredoc
 */
void	setup_signals_heredoc(void)
{
    struct sigaction	sa;

    sa.sa_handler = sigint_heredoc_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    signal(SIGQUIT, SIG_IGN);
}

