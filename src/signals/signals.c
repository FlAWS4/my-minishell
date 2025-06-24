/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 01:46:14 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Disables the display of control characters in terminal
 * 
 * Modifies terminal settings to hide control characters like ^C
 * when signals are received, providing a cleaner user experience.
 */
void	disable_control_char_echo(void)
{
    struct termios	term;

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return ;
    term.c_lflag &= ~ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/**
 * Enables the display of control characters in terminal
 * 
 * Modifies terminal settings to show control characters like ^C
 * when signals are received, restoring default behavior.
 */
void	enable_control_char_echo(void)
{
    struct termios	term;

    if (tcgetattr(STDIN_FILENO, &term) == -1)
        return ;
    term.c_lflag |= ECHOCTL;
    tcsetattr(STDIN_FILENO, TCSANOW, &term);
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
    write(2, "^C\n", 3);
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
 * Executes commands with signal state management
 * 
 * @param shell  The shell context
 * @return       1 if interrupted, 0 otherwise
 * 
 * Sets a special exit status (999) to mark command execution,
 * executes commands, then handles signals and restores state.
 */
int	safely_execute_command(t_shell *shell)
{
    int	old_exit_status;

    old_exit_status = g_exit_status;
    g_exit_status = 999;
    dispatch_commands(shell);
    if (g_exit_status == 130)
    {
        restore_standard_fds(shell);
        return (1);
    }
    if (g_exit_status == 999)
        g_exit_status = old_exit_status;
    return (0);
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