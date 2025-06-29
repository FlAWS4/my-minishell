/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 22:04:15 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

static void	run_execve_or_exit(t_shell *shell, t_command *cmd)
{
	char	*full_path;
	char	*full_cmd;

	if (!cmd->args || !cmd->args[0])
		clean_and_exit_shell(shell, EXIT_SUCCESS);
	full_path = search_path_for_exec(cmd->args[0], shell);
	if (!full_path)
	{
		full_cmd = join_args(shell, cmd->args);
		error(NULL, full_cmd, "command not found");
		clean_and_exit_shell(shell, 127);
	}
	execve(full_path, cmd->args, shell->env);
	error("execve", cmd->args[0], strerror(errno));
	clean_and_exit_shell(shell, EXIT_SUCCESS);
}

void	handle_pipe_child(t_shell *shell, t_command *cmd, int input_fd,
	int pipe_fds[2])
{
	int	exit_code;

	reset_signals_to_default();
	if (shell->saved_stdin >= 0)
		close(shell->saved_stdin);
	if (shell->saved_stdout >= 0)
		close(shell->saved_stdout);
	if (!cmd)
		clean_and_exit_shell(shell, EXIT_SUCCESS);
	setup_child_fds(cmd, input_fd, pipe_fds);
	close_all_command_fds(shell->commands, cmd);
	if (handle_redirections(cmd, shell) == -1)
		clean_and_exit_shell(shell, EXIT_FAILURE);
	redirect_stdio(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (is_builtin(cmd))
	{
		exit_code = run_builtin(shell, cmd);
		clean_and_exit_shell(shell, exit_code);
	}
	else
		run_execve_or_exit(shell, cmd);
}
void	wait_for_all_children(pid_t *pids, int count, pid_t last_pid)
{
	int	i;
	int	status;

	i = 0;
	while (i < count)
	{
		if (waitpid(pids[i], &status, 0) != -1)
		{
			if (pids[i] == last_pid)
			{
				if (WIFEXITED(status))
					g_exit_status = WEXITSTATUS(status);
				else if (WIFSIGNALED(status))
					g_exit_status = 128 + WTERMSIG(status);
			}
		}
		i++;
	}
}

void	wait_for_some_children(pid_t *pids, int count)
{
	int	i;
	int	status;
	int	waited;
	int	to_wait;

	i = 0;
	status = 0;
	waited = 0;
	to_wait = count / 4;
	if (to_wait < 1)
		to_wait = 1;
	while (i < count && waited < to_wait)
	{
		if (waitpid(pids[i], &status, WNOHANG) > 0)
			waited++;
		i++;
	}
}

void	execute_pipe(t_shell *shell, t_command *cmd, pid_t *pids)
{
	int			input_fd;
	int			fork_count;
	pid_t		last_pid;
	int			res;
	t_pipe_data	data;

	input_fd = STDIN_FILENO;
	fork_count = 0;
	last_pid = 0;
	res = 0;
	data.input_fd = &input_fd;
	data.fork_count = &fork_count;
	data.last_pid = &last_pid;
	data.pids = pids;
	while (cmd != NULL)
	{
		res = pipe_and_fork_one_cmd(shell, cmd, &data);
		if (res == 0)
			return ;
		cmd = cmd->next;
	}
	if (input_fd != STDIN_FILENO && input_fd != -1)
		close(input_fd);
	wait_for_all_children(pids, fork_count, last_pid);
}
