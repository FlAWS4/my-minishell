/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:08 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 03:20:12 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <signal.h>
#include <string.h>
#include "minishell.h"

void	disable_prints(void)
{
	struct termios	term;

	if (tcgetattr(STDIN_FILENO, &term) == -1)
		return ;
	term.c_lflag &= ~ECHOCTL;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

void	enable_prints(void)
{
	struct termios	term;

	if (tcgetattr(STDIN_FILENO, &term) == -1)
		return ;
	term.c_lflag |= ECHOCTL;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
}
void	sigint_heredoc(int sig)
{
	(void)sig;
	g_exit_status = 19;
	write(2, "^C\n", 3);
}

void	restore_signals_clear_buffer(struct sigaction *old_int,
					struct sigaction *old_quit)
{
	sigaction(SIGINT, old_int, NULL);
	sigaction(SIGQUIT, old_quit, NULL);
	get_next_line(STDIN_FILENO, 1);
}

void	write_warning(char *delim)
{
	if (!delim)
		return ;
	ft_putstr_fd(HEREDOCW, 2);
	ft_putstr_fd(delim, 2);
	ft_putstr_fd("')\n", 2);
}

void	restore_signal(struct sigaction *old_int,
					struct sigaction *old_quit)
{
	sigaction(SIGINT, old_int, NULL);
	sigaction(SIGQUIT, old_quit, NULL);
}

void	init_heredoc_signals(struct sigaction *old_int,
	struct sigaction *old_quit)
{
	struct sigaction	act_int;
	struct sigaction	act_quit;

	sigemptyset(&act_int.sa_mask);
	act_int.sa_handler = sigint_heredoc;
	act_int.sa_flags = 0;
	sigaction(SIGINT, &act_int, old_int);
	sigemptyset(&act_quit.sa_mask);
	act_quit.sa_handler = SIG_IGN;
	act_quit.sa_flags = 0;
	sigaction(SIGQUIT, &act_quit, old_quit);
}
void	handle_sigint(int sig)
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

void	setup_signals(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;
	struct sigaction	sa_tstp;

	sa_int.sa_handler = handle_sigint;
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

int	execute_with_signal_recovery(t_shell *shell)
{
	int	old_exit_status;

	old_exit_status = g_exit_status;
	g_exit_status = 999;
	dispatch_commands(shell);
	if (g_exit_status == 130)
	{
		dup2(shell->saved_stdin, STDIN_FILENO);
		dup2(shell->saved_stdout, STDOUT_FILENO);
		return (1);
	}
	if (g_exit_status == 999)
		g_exit_status = old_exit_status;
	return (0);
}
void	restore_std_fds(t_shell *shell)
{
	dup2(shell->saved_stdin, STDIN_FILENO);
	dup2(shell->saved_stdout, STDOUT_FILENO);
}