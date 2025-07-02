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

/**
 * execute_external_command_or_exit - 
 * Attempts to execute external command or exits
 * @shell: Shell context containing environment variables
 * @cmd: Command structure with arguments to execute
 * 
 * This function locates an executable in the PATH environment variable,
 * then attempts to execute it with execve(). If the command cannot be found
 * or executed, it properly reports an error and exits the child process
 * with the appropriate exit code.
 * 
 * Key behaviors:
 * 1. Handles empty command arguments by exiting successfully
 * 2. Searches PATH for executable using command name
 * 3. Reports "command not found" with exit code 127 if not found
 * 4. Reports execve errors if the command was found but execution failed
 * 
 * Never returns (always exits the process).
 */
static void	execute_external_command_or_exit(t_shell *shell, t_command *cmd)
{
	char	*full_path;
	char	*full_cmd;

	if (!cmd->args || !cmd->args[0])
		clean_and_exit_shell(shell, EXIT_SUCCESS);
	full_path = search_path_for_exec(cmd->args[0], shell);
	if (!full_path)
	{
		full_cmd = combine_command_arguments(shell, cmd->args);
		error(NULL, full_cmd, "command not found");
		clean_and_exit_shell(shell, 127);
	}
	execve(full_path, cmd->args, shell->env);
	error("execve", cmd->args[0], strerror(errno));
	clean_and_exit_shell(shell, EXIT_SUCCESS);
}

/**
 * handle_pipe_child - Sets up and executes command in a child process
 * @shell: Shell context containing environment and settings
 * @cmd: Command to execute in this process
 * @input_fd: Input file descriptor (from previous pipe or stdin)
 * @pipe_fds: Current pipe file descriptors for output redirection
 * 
 * This function prepares a child process 
 * for command execution within a pipeline.
 * It performs all necessary setup steps in sequence:
 * 1. Resets signal handlers to default behavior
 * 2. Closes saved file descriptors from parent
 * 3. Sets up stdin/stdout to connect with pipe endpoints
 * 4. Closes all file descriptors not needed by this command
 * 5. Handles command-specific redirections
 * 6. Executes either a builtin or external command
 * 
 * Never returns (always exits the process with appropriate exit code).
 */
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
	setup_command_io(cmd, input_fd, pipe_fds);
	close_unused_command_fds(shell->commands, cmd);
	if (process_command_redirections(cmd, shell) == -1)
		clean_and_exit_shell(shell, EXIT_FAILURE);
	apply_command_redirections(cmd);
	if (!cmd->args || !cmd->args[0] || !writable(STDOUT_FILENO, cmd->args[0]))
		clean_and_exit_shell(shell, 1);
	if (is_builtin(cmd))
	{
		exit_code = run_builtin(shell, cmd);
		clean_and_exit_shell(shell, exit_code);
	}
	else
		execute_external_command_or_exit(shell, cmd);
}

/**
 * collect_pipeline_exit_status - Waits for all pipeline processes to complete
 * @pids: Array of process IDs to wait for
 * @count: Number of processes in the array
 * @last_pid: PID of the last command in the pipeline (determines exit status)
 * 
 * This function waits for all child processes in a pipeline to complete and
 * sets the global exit status based on the last command's result. This mimics
 * how bash handles exit status for pipelines - only the last command's status
 * is returned.
 * 
 * The exit status handling follows POSIX standards:
 * - Normal exit: Uses the exit code provided by the process
 * - Signal termination: 128 + signal number
 */
void	collect_pipeline_exit_status(pid_t *pids, int count, pid_t last_pid)
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

/**
 * cleanup_finished_processes - Removes completed processes from system
 * @pids: Array of process IDs to check
 * @count: Number of processes in the array
 * 
 * This function performs a non-blocking wait on some child processes to avoid
 * accumulating too many completed-but-not-removed processes when running large
 * pipelines. It checks approximately 1/4 of the total processes, 
 * with a minimum of 1.
 * 
 * This helps manage system resources when executing complex command chains.
 */
void	cleanup_finished_processes(pid_t *pids, int count)
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

/**
 * execute_pipe - Executes a sequence of commands as a pipeline
 * @shell: Shell context containing environment and settings
 * @cmd: First command in the pipeline chain
 * @pids: Pre-allocated array to store process IDs
 * 
 * This is the main pipeline execution function that:
 * 1. Sets up the initial input and tracking variables
 * 2. Creates data structure to track pipeline execution state
 * 3. Iterates through all commands in the chain, processing each one
 * 4. Manages file descriptor connections between processes
 * 5. Waits for all processes to complete and collects exit status
 * 
 * This function implements the core of pipeline handling in the shell,
 * connecting multiple commands with pipes so the output of each command
 * becomes the input of the next.
 */
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
		res = process_single_piped_command(shell, cmd, &data);
		if (res == 0)
			return ;
		cmd = cmd->next;
	}
	if (input_fd != STDIN_FILENO && input_fd != -1)
		close(input_fd);
	collect_pipeline_exit_status(pids, fork_count, last_pid);
}
