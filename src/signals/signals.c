/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 21:26:58 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Only one global variable as required by subject
 * Used to track signal state between processes
 */
extern int	g_signal;

/**
 * Event hook for readline - helps to handle signals in a proper way
 * This is called by readline between key presses
 */
int check_for_signals(void)
{
    if (g_signal == SIGINT)
    {
        // Print a single newline here instead of in the signal handler
        write(1, "\n", 1);
        
        // Reset readline state properly
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
        return (1);
    }
    return (0);
}

/**
 * Signal handler for interactive mode
 * Handles Ctrl+C by displaying a new prompt
 */
void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        rl_done = 1;
        
        // This is crucial for properly breaking out of readline
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

/**
 * Signal handler for heredoc input
 * Exits the heredoc process with status 130 on SIGINT
 */
void	sigint_heredoc_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        ft_putstr_fd("\n", 1);
        gnl_cleanup(STDIN_FILENO);
        exit(130);
    }
}

/**
 * Setup signals for interactive mode
 * Sets custom handler for SIGINT and ignores SIGQUIT
 */
void	setup_signals(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    // Register the event hook for readline
    rl_event_hook = check_for_signals;
    
    // Use ft_bzero instead of memset
    ft_bzero(&sa_int, sizeof(sa_int));
    ft_bzero(&sa_quit, sizeof(sa_quit));
    
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sigaddset(&sa_int.sa_mask, SIGQUIT);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sigaddset(&sa_quit.sa_mask, SIGINT);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for non-interactive mode (child processes)
 * Restores default handling for both SIGINT and SIGQUIT
 */
void	setup_signals_noninteractive(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    // Use ft_bzero instead of memset
    ft_bzero(&sa_int, sizeof(sa_int));
    ft_bzero(&sa_quit, sizeof(sa_quit));
    
    sa_int.sa_handler = SIG_DFL;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_DFL;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
    g_signal = 0;
}

/**
 * Setup signals specifically for heredoc input
 * Sets custom handler for SIGINT and ignores SIGQUIT
 */
void	setup_signals_heredoc(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    // Use ft_bzero instead of memset
    ft_bzero(&sa_int, sizeof(sa_int));
    ft_bzero(&sa_quit, sizeof(sa_quit));
    
    sa_int.sa_handler = sigint_heredoc_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, NULL);
}
