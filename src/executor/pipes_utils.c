/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:42:22 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:42:22 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * handle_parent_pipe_descriptors - Manages file descriptors in the parent process
 * @input_fd: Pointer to the current input file descriptor
 * @pipe_fds: Array containing read (0) and write (1) ends of the pipe
 * @cmd: Current command being processed
 * 
 * This function properly manages file descriptors in the parent process after
 * forking a child for command execution in a pipeline. It:
 * 1. Closes the previous input fd if it's not standard input
 * 2. Sets up the read end of the current pipe as input for the next command
 * 3. Closes unused pipe ends to prevent fd leaks
 * 
 * Critical for maintaining proper pipe chain and preventing fd exhaustion.
 */
static void	handle_parent_pipe_descriptors(int *input_fd, int pipe_fds[2],
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

/**
 * process_single_piped_command - Processes a single command within a pipeline
 * @shell: Shell context containing environment and settings
 * @cmd: Current command to process in the pipeline
 * @data: Pipeline execution data including pids and counters
 * 
 * This function is the core of pipeline processing. For each command, it:
 * 1. Creates pipes as needed for command output
 * 2. Periodically waits for processes to avoid excessive forking
 * 3. Forks and executes the command
 * 4. Updates process tracking information
 * 5. Sets up file descriptors for the next command
 * 
 * Returns: 1 on success, 0 on failure (with proper cleanup)
 */
int	process_single_piped_command(t_shell *shell, t_command *cmd,
    t_pipe_data *data)
{
    int		pipe_fds[2];
    pid_t	pid;

    if (!create_pipe_if_needed(cmd, pipe_fds))
    {
        if (*(data->fork_count) > 0)
            collect_pipeline_exit_status(data->pids, *(data->fork_count),
                *(data->last_pid));
        return (0);
    }
    if (*(data->fork_count) > 0 && *(data->fork_count) % 10 == 0)
        cleanup_finished_processes(data->pids, *(data->fork_count));
    pid = fork_pipe_child(shell, cmd, *(data->input_fd), pipe_fds);
    if (pid == -1)
    {
        if (*(data->fork_count) > 0)
            collect_pipeline_exit_status(data->pids, *(data->fork_count),
                *(data->last_pid));
        return (0);
    }
    data->pids[*(data->fork_count)] = pid;
    (*(data->fork_count))++;
    *(data->last_pid) = pid;
    handle_parent_pipe_descriptors(data->input_fd, pipe_fds, cmd);
    return (1);
}
/**
 * create_pipe_if_needed - Creates a pipe only if there's another command in chain
 * @cmd: Current command to check if it needs a pipe
 * @pipe_fds: Array to store pipe file descriptors (read/write ends)
 * 
 * This function optimizes pipe creation by only creating pipes when
 * another command follows in the pipeline. It initializes pipe file
 * descriptors to safe values and handles pipe creation errors.
 * 
 * Returns: 1 on success (pipe created or not needed), 0 on pipe creation failure
 */
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

/**
 * fork_pipe_child - Creates a child process for executing a command in a pipeline
 * @shell: Shell context containing environment and settings
 * @cmd: Command to execute in the child process
 * @input_fd: Input file descriptor (from previous pipe or stdin)
 * @pipe_fds: Current pipe file descriptors for output redirection
 * 
 * This function forks a child process to execute a command in a pipeline,
 * handles fork errors with proper cleanup, and calls the appropriate
 * handler for the child process. The parent process simply returns the
 * child's pid for tracking.
 * 
 * Returns: Child process PID on success, -1 on fork failure
 */
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

/**
 * setup_pipeline_execution - Prepares and executes a sequence of piped commands
 * @shell: Shell context containing environment and settings
 * @cmd: First command in the pipeline sequence
 * 
 * This function is the entry point for pipeline execution. It:
 * 1. Sets up signal handling specific to pipelines (ignoring SIGPIPE)
 * 2. Counts the number of commands in the pipeline
 * 3. Allocates memory for tracking process IDs
 * 4. Executes the pipeline commands
 * 5. Ensures proper cleanup of file descriptors
 * 
 * Called when the shell detects multiple commands connected by pipes.
 */
void	setup_pipeline_execution(t_shell *shell, t_command *cmd)
{
    int			cmd_count;
    t_command	*tmp;
    pid_t		*pids;
    size_t		size;

    signal(SIGPIPE, SIG_IGN);
    cmd_count = 0;
    tmp = cmd;
    while (tmp != NULL)
    {
        cmd_count++;
        tmp = tmp->next;
    }
    size = sizeof(pid_t) * cmd_count;
    pids = allocate_managed_memory(&shell->memory_manager, size, MEM_ERROR_FATAL, NULL);
    if (pids == NULL)
    {
        error("malloc", NULL, strerror(errno));
        return ;
    }
    ft_memset(pids, 0, size);
    execute_pipe(shell, cmd, pids);
    cleanup_shell_file_descriptors(shell);
}
