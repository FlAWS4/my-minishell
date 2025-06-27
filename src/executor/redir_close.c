/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_close.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 19:38:41 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/26 19:40:54 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	close_fds(t_shell *shell)
{
	int	fd;

	fd = 3;
	while (fd < OPEN_MAX)
	{
		if (fd != shell->saved_stdin && fd != shell->saved_stdout)
			close(fd);
		fd++;
	}
	if (shell->saved_stdin >= 0)
	{
		close(shell->saved_stdin);
		shell->saved_stdin = -1;
	}
	if (shell->saved_stdout >= 0)
	{
		close(shell->saved_stdout);
		shell->saved_stdout = -1;
	}
}

void	close_all_fds_except_stdio(void)
{
	int	fd;

	fd = 3;
	while (fd < OPEN_MAX)
	{
		close(fd);
		fd++;
	}
}

void	close_all_command_fds(t_command *all_cmds, t_command *current_cmd)
{
	t_command	*cmd;

	cmd = all_cmds;
	while (cmd)
	{
		if (cmd != current_cmd)
		{
			if (cmd->fd_in != -1 && cmd->fd_in > 2)
				close(cmd->fd_in);
			if (cmd->fd_out != -1 && cmd->fd_out > 2)
				close(cmd->fd_out);
		}
		cmd = cmd->next;
	}
}

int	handle_heredoc(t_command *cmd, t_redir *redir)
{
	int	pipe_fd[2];

	if (pipe(pipe_fd) == -1)
		return (-1);
	write(pipe_fd[1], redir->heredoc_content,
		ft_strlen(redir->heredoc_content));
	close(pipe_fd[1]);
	if (cmd->fd_in != STDIN_FILENO)
		close(cmd->fd_in);
	cmd->fd_in = pipe_fd[0];
	return (0);
}