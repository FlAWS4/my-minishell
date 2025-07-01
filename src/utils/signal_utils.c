/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signal_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:07:50 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:10:56 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Updates the global exit status after waiting for a child process
 * 
 * @param pid  Process ID to wait for
 * 
 * Handles both normal exit status and signals that terminated the process.
 * For signals, adds special handling for SIGINT and SIGQUIT.
 */
static void	process_command_exit_status(pid_t pid)
{
	int	status;
	int	sig;

	if (waitpid(pid, &status, 0) == -1)
		return ;
	if (WIFEXITED(status))
		g_exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
	{
		sig = WTERMSIG(status);
		if (sig == SIGINT)
			ft_putendl_fd("", STDOUT_FILENO);
		else if (sig == SIGQUIT)
			ft_putendl_fd("Quit", STDOUT_FILENO);
		g_exit_status = 128 + sig;
	}
}

/**
 * Ignores SIGINT signal while waiting for child process
 * 
 * @param child_pid  Process ID to wait for
 * 
 * Temporarily sets SIGINT to be ignored while waiting for child process,
 * then restores the original signal handler.
 */
void	ignore_sigint_and_wait(pid_t child_pid)
{
	void	(*original_handler)(int);

	original_handler = signal(SIGINT, SIG_IGN);
	process_command_exit_status(child_pid);
	signal(SIGINT, original_handler);
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
    execute_command_sequence(shell);
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

