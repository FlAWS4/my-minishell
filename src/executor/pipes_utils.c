/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 19:38:41 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/26 19:40:54 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	parent_setup_next_input_fd(int *input_fd, int pipe_fds[2],
	t_command *cmd)
{
	if (*input_fd != STDIN_FILENO && *input_fd != -1)
	{
		close(*input_fd);
		*input_fd = -1;
	}
	if (cmd->next)
		*input_fd = pipe_fds[0];
	else if (pipe_fds[0] != -1)
	{
		close(pipe_fds[0]);
		pipe_fds[0] = -1;
	}
	if (pipe_fds[1] != -1)
	{
		close(pipe_fds[1]);
		pipe_fds[1] = -1;
	}
}

int	pipe_and_fork_one_cmd(t_shell *shell, t_command *cmd,
	t_pipe_data *data)
{
	int		pipe_fds[2];
	pid_t	pid;

	if (!create_pipe_if_needed(cmd, pipe_fds))
	{
		if (*(data->fork_count) > 0)
			wait_for_all_children(data->pids, *(data->fork_count),
				*(data->last_pid));
		return (0);
	}
	if (*(data->fork_count) > 0 && *(data->fork_count) % 100 == 0)
		wait_for_some_children(data->pids, *(data->fork_count));
	pid = fork_pipe_child(shell, cmd, *(data->input_fd), pipe_fds);
	if (pid == -1)
	{
		if (*(data->fork_count) > 0)
			wait_for_all_children(data->pids, *(data->fork_count),
				*(data->last_pid));
		return (0);
	}
	data->pids[*(data->fork_count)] = pid;
	(*(data->fork_count))++;
	*(data->last_pid) = pid;
	parent_setup_next_input_fd(data->input_fd, pipe_fds, cmd);
	return (1);
}
int	create_pipe_if_needed(t_command *cmd, int pipe_fds[2])
{
	pipe_fds[0] = -1;
	pipe_fds[1] = -1;
	if (cmd->next)
	{
		if (pipe(pipe_fds) == -1)
		{
			error("pipe", NULL, strerror(errno));
			return (0);
		}
	}
	return (1);
}

int	fork_pipe_child(t_shell *shell, t_command *cmd, int input_fd,
	int pipe_fds[2])
{
	pid_t	pid;

	pid = fork();
	if (pid == -1)
	{
		error("fork", NULL, strerror(errno));
		if (pipe_fds[0] != -1)
			close(pipe_fds[0]);
		if (pipe_fds[1] != -1)
			close(pipe_fds[1]);
		return (-1);
	}
	if (pid == 0)
		handle_pipe_child(shell, cmd, input_fd, pipe_fds);
	return (pid);
}
void	prepare_pipe_execution(t_shell *shell, t_command *cmd)
{
	int			cmd_count;
	t_command	*tmp;
	pid_t		*pids;
	size_t		size;

	cmd_count = 0;
	tmp = cmd;
	while (tmp != NULL)
	{
		cmd_count++;
		tmp = tmp->next;
	}
	size = sizeof(pid_t) * cmd_count;
	pids = gc_malloc(&shell->gc, size, GC_FATAL, NULL);
	if (pids == NULL)
	{
		error("malloc", NULL, strerror(errno));
		return ;
	}
	ft_memset(pids, 0, size);
	execute_pipe(shell, cmd, pids);
	close_fds(shell);
}
