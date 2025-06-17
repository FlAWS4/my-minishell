/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/18 00:47:18 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Used to track signal state between processes
 */
extern int	g_signal;

/**
 * Event hook for readline - called between key presses
 */
int	check_for_signals(void)
{
    if (g_signal == SIGINT)
    {
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
        g_signal = 0;
        return (1);
    }
    return (0);
}

/**
 * Signal handler for interactive mode
 */
void	sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        write(1, "\n", 1);
    }
}

/**
 * Signal handler for heredoc input mode
 */
void	sigint_heredoc_handler(int signum)
{
    if (signum == SIGINT)
    {
        g_signal = SIGINT;
        write(STDOUT_FILENO, "\n", 1);
        close(STDIN_FILENO);
        exit(130);
    }
}

/**
 * Configure a sigaction structure with given handler
 */
static void	setup_signal_action(struct sigaction *sa, void (*handler)(int))
{
    ft_bzero(sa, sizeof(struct sigaction));
    sa->sa_handler = handler;
    sigemptyset(&sa->sa_mask);
    sa->sa_flags = SA_RESTART;
}

/**
 * Setup signals for interactive shell mode
 */
void	setup_signals(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    rl_event_hook = check_for_signals;
    setup_signal_action(&sa_int, sigint_handler);
    sigaction(SIGINT, &sa_int, NULL);
    setup_signal_action(&sa_quit, SIG_IGN);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for child processes running commands
 */
void	setup_signals_noninteractive(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    rl_event_hook = NULL;
    setup_signal_action(&sa_int, SIG_DFL);
    sigaction(SIGINT, &sa_int, NULL);
    setup_signal_action(&sa_quit, SIG_DFL);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Setup signals for heredoc input handling
 */
void	setup_signals_heredoc(void)
{
    struct sigaction	sa_int;
    struct sigaction	sa_quit;

    rl_event_hook = NULL;
    setup_signal_action(&sa_int, sigint_heredoc_handler);
    sigaction(SIGINT, &sa_int, NULL);
    setup_signal_action(&sa_quit, SIG_IGN);
    sigaction(SIGQUIT, &sa_quit, NULL);
}

/**
 * Reset all signals to default behavior
 */
void	reset_signals(void)
{
    struct sigaction	sa;

    setup_signal_action(&sa, SIG_DFL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    rl_event_hook = NULL;
    g_signal = 0;
}

/**
 * Ignore terminal control signals in child processes
 */
void	ignore_tty_signals(void)
{
    struct sigaction	sa;

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTTIN, &sa, NULL);
    sigaction(SIGTTOU, &sa, NULL);
}

/**
 * Restore original terminal settings
 */
void	restore_terminal_settings(t_shell *shell)
{
    if (!isatty(STDIN_FILENO))
        return ;
    tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
}

/**
 * Restore shell terminal state after heredoc interruption
 */
void	restore_shell_terminal(t_shell *shell)
{
    if (!isatty(STDIN_FILENO))
        return ;
    tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
    setup_signals();
}

/**
 * Clean up readline resources before exit
 */
void	cleanup_readline_resources(void)
{
    save_history();
    rl_clear_history();
}

