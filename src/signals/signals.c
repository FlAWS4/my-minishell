/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 06:15:02 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

// Only one global variable as required by subject
extern int g_signal;  // Defined in minishell.h

/**
 * Signal handler for interactive mode
 */
void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;  // Store only the signal number
        write(1, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

/**
 * Signal handler for heredoc input
 */
void sigint_heredoc_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        ft_putstr_fd("\n", 1);
        
        // Reset get_next_line buffer
        reset_gnl_buffer();
        
        // Exit the heredoc process with special status
        exit(130);  // 128 + SIGINT (2)
    }
}

/**
 * Setup signals for interactive mode
 */
void setup_signals(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;

    // Clear structures to avoid uninitialized values
    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_quit, 0, sizeof(sa_quit));

    // Set up SIGINT handler (Ctrl+C)
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sigaddset(&sa_int.sa_mask, SIGQUIT);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // Ignore SIGQUIT (Ctrl+\)
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sigaddset(&sa_quit.sa_mask, SIGINT);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for non-interactive mode (child processes)
 */
void setup_signals_noninteractive(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;

    // Clear structures
    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_quit, 0, sizeof(sa_quit));

    // Default handling for SIGINT
    sa_int.sa_handler = SIG_DFL;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // Default handling for SIGQUIT
    sa_quit.sa_handler = SIG_DFL;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    // Reset signal state
    g_signal = 0;
}

/**
 * Setup signals specifically for heredoc input
 */
void setup_signals_heredoc(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;

    // Clear structures
    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_quit, 0, sizeof(sa_quit));

    // Use special heredoc handler for SIGINT
    sa_int.sa_handler = sigint_heredoc_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);

    // Ignore SIGQUIT
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
