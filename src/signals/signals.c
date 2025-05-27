/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 21:31:28 by mshariar         ###   ########.fr       */
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
        g_signal = SIGINT;  // Store actual signal value
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
        g_signal = SIGINT;  // Store actual signal value
        write(1, "\n", 1);
        close(0);  // Close stdin to interrupt the heredoc
    }
}

/**
 * Setup signals for interactive mode
 */
void	setup_signals(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sigaddset(&sa_int.sa_mask, SIGQUIT);  // Block SIGQUIT during SIGINT handler
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sigaddset(&sa_quit.sa_mask, SIGINT);  // Block SIGINT during SIGQUIT handler
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for non-interactive mode (child processes)
 */
void	setup_signals_noninteractive(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    sa_int.sa_handler = SIG_DFL;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = SIG_DFL;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    g_signal = 0;  // Reset global signal state
}

/**
 * Setup signals for heredoc
 */
void	setup_signals_heredoc(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    sa_int.sa_handler = sigint_heredoc_handler;
    sigemptyset(&sa_int.sa_mask);
    sigaddset(&sa_int.sa_mask, SIGQUIT);  // Block SIGQUIT during handler
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
