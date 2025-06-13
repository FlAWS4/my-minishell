/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/13 20:57:28 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Only one global variable as required by subject
 * Used to track signal state between processes
 */
extern int g_signal;

/**
 * Event hook for readline - called between key presses
 * Handles displaying a new prompt after SIGINT
 */
int check_for_signals(void)
{
    if (g_signal == SIGINT)
    {
        write(1, "\n", 1);
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
        g_signal = 0;  // Reset signal flag after handling
        return (1);
    }
    return (0);
}

/**
 * Signal handler for interactive mode (main shell prompt)
 * Sets global signal flag and breaks readline
 */
void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        rl_done = 1;
        // Don't call rl_replace_line/rl_redisplay here - can cause issues
        // if readline is in the middle of reading input
    }
}

/**
 * Signal handler for heredoc mode
 * Exits heredoc process with status 130 on SIGINT
 */
void sigint_heredoc_handler(int signum)
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
 * Configure a sigaction structure with given handler
 */
static void setup_signal_action(struct sigaction *sa, void (*handler)(int))
{
    ft_bzero(sa, sizeof(struct sigaction));
    sa->sa_handler = handler;
    sigemptyset(&sa->sa_mask);
    sa->sa_flags = SA_RESTART;  // Add SA_RESTART flag for better stability
}

/**
 * Setup signals for interactive mode (main shell)
 */
void setup_signals(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;

    // Reset readline event hook (in case it was cleared)
    rl_event_hook = check_for_signals;
    
    setup_signal_action(&sa_int, sigint_handler);
    sigaddset(&sa_int.sa_mask, SIGQUIT);
    sigaction(SIGINT, &sa_int, NULL);
    
    setup_signal_action(&sa_quit, SIG_IGN);
    sigaddset(&sa_quit.sa_mask, SIGINT);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for child processes (commands)
 */
void setup_signals_noninteractive(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    
    // Also disable the readline event hook in non-interactive mode
    rl_event_hook = NULL;
    
    setup_signal_action(&sa_int, SIG_DFL);
    sigaction(SIGINT, &sa_int, NULL);
    
    setup_signal_action(&sa_quit, SIG_DFL);
    sigaction(SIGQUIT, &sa_quit, NULL);
    
    g_signal = 0;
}

/**
 * Setup signals for heredoc input
 */
void setup_signals_heredoc(void)
{
    struct sigaction sa_int;
    struct sigaction sa_quit;
    
    // Disable readline event hook during heredoc
    rl_event_hook = NULL;
    
    setup_signal_action(&sa_int, sigint_heredoc_handler);
    sigaction(SIGINT, &sa_int, NULL);
    
    setup_signal_action(&sa_quit, SIG_IGN);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Reset all signals to default behavior
 * Useful when terminal is restarted
 */
void reset_signals(void)
{
    struct sigaction sa;
    
    setup_signal_action(&sa, SIG_DFL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    
    // Clear readline event hook
    rl_event_hook = NULL;
    
    // Reset global signal state
    g_signal = 0;
}
