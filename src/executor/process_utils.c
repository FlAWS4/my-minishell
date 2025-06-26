/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 19:45:12 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/26 19:57:24 by hchowdhu         ###   ########.fr       */
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
			ft_putendl_fd("^\\Quit", STDOUT_FILENO);
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

