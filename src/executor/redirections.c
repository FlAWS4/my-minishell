/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 03:20:47 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	setup_child_input_fd(t_command *cmd, int input_fd)
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

static void	setup_child_output_fd(t_command *cmd, int pipe_fds[2])
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

char	*join_args(t_shell *shell, char **args)
{
	char	*result;
	char	*tmp;
	int		i;

	i = 0;
	if (!args || !args[0])
		return (gc_strdup(&shell->gc, ""));
	result = gc_strdup(&shell->gc, args[i]);
	i++;
	while (args[i])
	{
		tmp = gc_strjoin(&shell->gc, result, "");
		result = gc_strjoin(&shell->gc, tmp, args[i]);
		i++;
	}
	return (result);
}
int	open_error(const char *filename)
{
	if (errno == EISDIR)
		error(NULL, filename, "Is a directory");
	else if (errno == EACCES)
		error(NULL, filename, "Permission denied");
	else
		error(NULL, filename, strerror(errno));
	return (-1);
}

int	open_input_file(const char *path)
{
	int	fd;

	fd = open(path, O_RDONLY);
	if (fd == -1)
		return (open_error(path));
	return (fd);
}

int	open_output_file(const char *path)
{
	int	fd;

	fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0666);
	if (fd == -1)
		return (open_error(path));
	return (fd);
}

int	open_append_file(const char *path)
{
	int	fd;

	fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd == -1)
		return (open_error(path));
	return (fd);
}

int	open_redir_file(t_redir *redir, t_command *cmd)
{
	if (redir->ar)
		return (-1);
	if (!redir->file_or_del || !*redir->file_or_del)
	{
		error(NULL, "", "No such file or directory\n");
		return (-1);
	}
	if (redir->type == REDIR_IN)
		return (open_input_file(redir->file_or_del));
	else if (redir->type == REDIR_OUT)
		return (open_output_file(redir->file_or_del));
	else if (redir->type == APPEND)
		return (open_append_file(redir->file_or_del));
	else if (redir->type == HEREDOC)
	{
		if (handle_heredoc(cmd, redir) == -1)
			return (-1);
		return (0);
	}
	return (-1);
}
static void	set_redir_fds(t_command *cmd, int type, int fd)
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

int	handle_redirections(t_command *cmd, t_shell *shell)
{
	t_redir	*redir;
	int		fd;

	redir = cmd->redirs;
	while (redir)
	{
		fd = open_redir_file(redir, cmd);
		if (fd == -1)
		{
			close_fds(shell);
			g_exit_status = 1;
			return (-1);
		}
		if (redir->type != HEREDOC || fd > 0)
			set_redir_fds(cmd, redir->type, fd);
		redir = redir->next;
	}
	return (0);
}

int	handle_builtin_redirections(t_shell *shell, t_command *cmd)
{
	int	saved_stdin;
	int	saved_stdout;
	int	result;

	saved_stdin = -1;
	saved_stdout = -1;
	if (cmd->fd_in != STDIN_FILENO && cmd->fd_in != -1)
		saved_stdin = dup(STDIN_FILENO);
	if (cmd->fd_out != STDOUT_FILENO && cmd->fd_out != -1)
		saved_stdout = dup(STDOUT_FILENO);
	redirect_stdio(cmd);
	result = run_builtin(shell, cmd);
	if (saved_stdin != -1)
	{
		dup2(saved_stdin, STDIN_FILENO);
		close(saved_stdin);
	}
	if (saved_stdout != -1)
	{
		dup2(saved_stdout, STDOUT_FILENO);
		close(saved_stdout);
	}
	return (result);
}
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
