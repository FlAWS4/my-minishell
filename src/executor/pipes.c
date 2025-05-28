/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:36:10 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Set up pipes for a child process
 */
static void	setup_child_pipes(int prev_pipe, int *pipe_fds, t_cmd *current)
{
    if (prev_pipe != -1)
    {
        if (dup2(prev_pipe, STDIN_FILENO) == -1)
            exit(1);
        close(prev_pipe);
    }
    if (current->next)
    {
        close(pipe_fds[0]);
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1)
            exit(1);
        close(pipe_fds[1]);
    }
}

/**
 * Handle pipe creation failure
 */
static int	handle_pipe_failure(int prev_pipe)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    display_error(ERR_PIPE, "pipe", strerror(errno));
    return (1);
}

/**
 * Handle fork failure
 */
static int	handle_fork_failure(int prev_pipe, int *pipe_fds, t_cmd *cmd)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    if (cmd->next)
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }
    display_error(ERR_FORK, "fork", strerror(errno));
    return (1);
}

/**
 * Execute a single command in a pipeline
 */
static int	execute_piped_command(t_shell *shell, t_cmd *cmd,
        int prev_pipe, int *pipe_fds)
{
    pid_t	pid;

    if (cmd->next && pipe(pipe_fds) == -1)
        return (handle_pipe_failure(prev_pipe));
    pid = fork();
    if (pid == -1)
        return (handle_fork_failure(prev_pipe, pipe_fds, cmd));
    if (pid == 0)
    {
        setup_signals_noninteractive();
        setup_child_pipes(prev_pipe, pipe_fds, cmd);
        execute_child(shell, cmd);
        exit(1);
    }
    return (0);
}

/**
 * Manage parent process pipe handling
 */
static int	manage_parent_pipes(int prev_pipe, int *pipe_fds, t_cmd *current)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    if (current->next)
    {
        close(pipe_fds[1]);
        return (pipe_fds[0]);
    }
    return (-1);
}

/**
 * Wait for all child processes to complete
 */
static int	wait_for_children(t_shell *shell)
{
    int	status;
    int	pid;
    int	last_status;

    last_status = 0;
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        if (pid <= 0)
            break ;
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
            last_status = 128 + WTERMSIG(status);
    }
    shell->exit_status = last_status;
    return (last_status);
}

/**
 * Execute a pipeline of commands
 */
int	execute_pipeline(t_shell *shell, t_cmd *cmd)
{
    int		pipe_fds[2];
    t_cmd	*current;
    int		prev_pipe;

    prev_pipe = -1;
    current = cmd;
    while (current)
    {
        if (execute_piped_command(shell, current, prev_pipe, pipe_fds))
            return (1);
        prev_pipe = manage_parent_pipes(prev_pipe, pipe_fds, current);
        current = current->next;
    }
    return (wait_for_children(shell));
}
