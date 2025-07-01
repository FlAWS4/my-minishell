/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_shell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 22:42:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/07/02 00:54:18 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * backup_standard_fds - Save copies of standard file descriptors
 * @shell: Shell structure to store file descriptor copies
 *
 * Duplicates STDIN and STDOUT for restoration after redirections
 * Exits with error if duplication fails
 */
void	init_shell_fds(t_shell *shell)
{
	shell->saved_stdin = dup(STDIN_FILENO);
	shell->saved_stdout = dup(STDOUT_FILENO);
	if (shell->saved_stdin == -1 || shell->saved_stdout == -1)
	{
		error("dup", NULL, strerror(errno));
		clean_and_exit_shell(shell, 1);
	}
}

/**
 * Built-in help command with beautiful formatted output
 */
int	builtin_help(t_shell *shell)
{
	(void)shell;
	printf("\n%s✨ %sMINISHELL HELP%s ✨%s\n\n", \
		BOLD_WHITE, BOLD_YELLOW, BOLD_WHITE, RESET);
	display_commands();
	printf("\n");
	display_env_commands();
	printf("\n");
	display_operators();
	printf("\n");
	return (0);
}
