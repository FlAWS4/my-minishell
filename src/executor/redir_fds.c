/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_fds.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 19:38:41 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/26 19:40:54 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	setup_child_input_fd(t_command *cmd, int input_fd)
{
	if (cmd->fd_in != -1 && cmd->fd_in != STDIN_FILENO)
	{
		if (dup2(cmd->fd_in, STDIN_FILENO) == -1)
			error("dup2", "stdin", strerror(errno));
		close(cmd->fd_in);
	}
	else if (input_fd != -1 && input_fd != STDIN_FILENO)
	{
		if (dup2(input_fd, STDIN_FILENO) == -1)
			error("dup2", "stdin", strerror(errno));
		close(input_fd);
	}
}

void	setup_child_output_fd(t_command *cmd, int pipe_fds[2])
{
	if (cmd->fd_out != -1 && cmd->fd_out != STDOUT_FILENO)
	{
		if (dup2(cmd->fd_out, STDOUT_FILENO) == -1)
			error("dup2", "stdout", strerror(errno));
		close(cmd->fd_out);
	}
	else if (cmd->next && pipe_fds[1] != -1)
	{
		if (dup2(pipe_fds[1], STDOUT_FILENO) == -1)
			error("dup2", "stdout", strerror(errno));
	}
	if (pipe_fds[0] != -1)
		close(pipe_fds[0]);
	if (pipe_fds[1] != -1 && pipe_fds[1] != STDOUT_FILENO)
		close(pipe_fds[1]);
}

void	setup_child_fds(t_command *cmd, int input_fd, int pipe_fds[2])
{
	setup_child_input_fd(cmd, input_fd);
	setup_child_output_fd(cmd, pipe_fds);
	close_all_fds_except_stdio();
}

void	set_redir_fds(t_command *cmd, int type, int fd)
{
	if (type == REDIR_IN)
	{
		if (cmd->fd_in != STDIN_FILENO && cmd->fd_in != -1)
			close(cmd->fd_in);
		cmd->fd_in = fd;
	}
	else if (type == REDIR_OUT || type == APPEND)
	{
		if (cmd->fd_out != STDOUT_FILENO && cmd->fd_out != -1)
			close(cmd->fd_out);
		cmd->fd_out = fd;
	}
}

void	redirect_stdio(t_command *cmd)
{
	if (cmd->fd_in != STDIN_FILENO && cmd->fd_in != -1)
	{
		if (dup2(cmd->fd_in, STDIN_FILENO) == -1)
			error("dup2", "stdin", strerror(errno));
		close(cmd->fd_in);
		cmd->fd_in = STDIN_FILENO;
	}
	if (cmd->fd_out != STDOUT_FILENO && cmd->fd_out != -1)
	{
		if (dup2(cmd->fd_out, STDOUT_FILENO) == -1)
			error("dup2", "stdout", strerror(errno));
		close(cmd->fd_out);
		cmd->fd_out = STDOUT_FILENO;
	}
}