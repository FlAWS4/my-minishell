/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 21:46:03 by mshariar         ###   ########.fr       */
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
        dup2(prev_pipe, STDIN_FILENO);
        close(prev_pipe);
    }
    if (current->next)
    {
        close(pipe_fds[0]);
        dup2(pipe_fds[1], STDOUT_FILENO);
        close(pipe_fds[1]);
    }
}

/**
 * Execute a single command in a pipeline
 */
static void	execute_piped_command(t_shell *shell, t_cmd *cmd,
        int prev_pipe, int *pipe_fds)
{
    pid_t	pid;

    if (cmd->next)
    {
        if (pipe(pipe_fds) == -1)
        {
            ft_putstr_fd("minishell: pipe: failed\n", 2);
            return ;
        }
    }
    pid = fork();
    if (pid == -1)
    {
        ft_putstr_fd("minishell: fork: failed\n", 2);
        return ;
    }
    if (pid == 0)
    {
        setup_signals_noninteractive();
        setup_child_pipes(prev_pipe, pipe_fds, cmd);
        execute_child(shell, cmd);
        exit(shell->exit_status);
    }
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
 * Wait for all child processes in a pipeline
 */
static int	wait_for_children(t_shell *shell)
{
    int	status;
    int	last_status;

    last_status = 0;
    while (wait(&status) > 0)
    {
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
        execute_piped_command(shell, current, prev_pipe, pipe_fds);
        prev_pipe = manage_parent_pipes(prev_pipe, pipe_fds, current);
        current = current->next;
    }
    return (wait_for_children(shell));
}
