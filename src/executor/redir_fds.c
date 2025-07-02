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

/**
 * setup_command_input - Configures input redirection for a command
 * @cmd: Command structure containing fd_in to use if available
 * @input_fd: Input file descriptor from previous pipe or stdin
 * 
 * This function sets up stdin for a command in a pipeline by:
 * 1. Using command's own input redirection (fd_in) if available
 * 2. Using the pipeline input (from previous command) if no redirection
 * 3. Properly closing file descriptors after duplication
 * 
 * Critical for implementing both stdin redirections (< file)
 * and proper pipeline connections (cmd1 | cmd2)
 */
void	setup_command_input(t_command *cmd, int input_fd)
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

/**
 * setup_command_output - Configures output redirection for a command
 * @cmd: Command structure containing fd_out to use if available
 * @pipe_fds: Pipe file descriptors for connecting to next command
 * 
 * This function sets up stdout for a command in a pipeline by:
 * 1. Using command's own output redirection (fd_out) if available
 * 2. Using the pipe write end if there's another command in pipeline
 * 3. Properly closing unused pipe file descriptors
 * 
 * Essential for implementing both stdout redirections (> file)
 * and proper pipeline connections (cmd1 | cmd2)
 */
void	setup_command_output(t_command *cmd, int pipe_fds[2])
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

/**
 * setup_command_io - Sets up all I/O for a pipeline command
 * @cmd: Command structure to configure
 * @input_fd: Input file descriptor from previous command
 * @pipe_fds: Pipe file descriptors for connecting to next command
 * 
 * This function prepares all file descriptors for a command by:
 * 1. Setting up input redirection
 * 2. Setting up output redirection
 * 3. Closing all other file descriptors to prevent leaks
 */
void	setup_command_io(t_command *cmd, int input_fd, int pipe_fds[2])
{
	setup_command_input(cmd, input_fd);
	setup_command_output(cmd, pipe_fds);
	close_all_non_standard_fds();
}

/**
 * update_command_redirections - 
 * Updates command with new redirection file descriptors
 * @cmd: Command structure to update
 * @type: Redirection type (REDIR_IN, REDIR_OUT, APPEND)
 * @fd: New file descriptor to set
 * 
 * This function updates a command's input or output file descriptors
 * based on redirection type, ensuring previous file descriptors are
 * properly closed to prevent leaks.
 * 
 * Called when processing redirection operators (<, >, >>)
 * during command execution setup.
 */
void	update_command_redirections(t_command *cmd, int type, int fd)
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

/**
 * apply_command_redirections - Applies command's redirections to stdin/stdout
 * @cmd: Command structure containing redirection file descriptors
 * 
 * This function applies a command's input and output redirections
 * to the actual stdin and stdout file descriptors, ensuring proper
 * cleanup after redirection is complete.
 * 
 * Used for both builtin commands and external processes to ensure
 * redirections affect the command's execution environment.
 */
void	apply_command_redirections(t_command *cmd)
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
