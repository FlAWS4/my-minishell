/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/15 06:44:06 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	setup_child_input(int prev_pipe, t_cmd *current)
{
    if (prev_pipe != -1)
    {
        if (current->input_fd == -1)
        {
            if (dup2(prev_pipe, STDIN_FILENO) == -1)
                exit(1);
        }
        else
        {
            if (dup2(current->input_fd, STDIN_FILENO) == -1)
                exit(1);
        }
        close(prev_pipe);
    }
    else if (current->input_fd != -1)
    {
        if (dup2(current->input_fd, STDIN_FILENO) == -1)
            exit(1);
        close(current->input_fd);
    }
}

static void	setup_child_output(int *pipe_fds, t_cmd *current)
{
    if (current->next)
    {
        close(pipe_fds[0]);
        if (dup2(pipe_fds[1], STDOUT_FILENO) == -1)
            exit(1);
        close(pipe_fds[1]);
    }
}

static void	setup_child_pipes(int prev_pipe, int *pipe_fds, t_cmd *current)
{
    setup_child_input(prev_pipe, current);
    setup_child_output(pipe_fds, current);
}

static int	handle_pipe_failure(int prev_pipe)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    display_error(ERR_PIPE, "pipe", strerror(errno));
    return (1);
}

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

static int execute_piped_command(t_shell *shell, t_cmd *cmd, 
                  int prev_pipe, int *pipe_fds)
{
    pid_t pid;

    if (cmd->next && pipe(pipe_fds) == -1)
        return (handle_pipe_failure(prev_pipe));
    
    pid = fork();
    if (pid == -1)
        return (handle_fork_failure(prev_pipe, pipe_fds, cmd));
    
    if (pid == 0)
    {
        // Child process
        setup_signals_noninteractive();
        setup_child_pipes(prev_pipe, pipe_fds, cmd);
        execute_child(shell, cmd);
        exit(1);
    }
    else
    {
        cmd->pid = pid;
    }
    return (0);
}

int wait_for_children(t_shell *shell)
{
    int status;
    int pid;
    int last_status = 0;
    
    if (!shell)
        return (1);
    
    // Wait for all child processes
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        if (pid <= 0)
        {
            if (errno == EINTR)
                continue;  // Retry if interrupted
            break;
        }
        
        // Track exit status
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
        {
            if (WTERMSIG(status) == SIGINT)
                write(STDOUT_FILENO, "\n", 1);
            last_status = 128 + WTERMSIG(status);
        }
    }
    
    shell->exit_status = last_status;
    return (last_status);
}

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

static void	cleanup_heredoc_files(t_cmd *cmd, t_cmd *current)
{
    t_cmd	*cleanup;

    cleanup = cmd;
    while (cleanup != current)
    {
        if (cleanup->heredoc_file)
        {
            unlink(cleanup->heredoc_file);
            free(cleanup->heredoc_file);
            cleanup->heredoc_file = NULL;
        }
        cleanup = cleanup->next;
    }
}

static int	process_cmd_heredoc(t_shell *shell, t_cmd *cmd, t_cmd *current)
{
    if (!process_heredoc(current, shell))
    {
        cleanup_heredoc_files(cmd, current);
        return (0);
    }
    return (1);
}

static int	process_pipeline_heredocs(t_shell *shell, t_cmd *cmd)
{
    t_cmd	*current;

    current = cmd;
    while (current)
    {
        if (current->heredoc_delim)
        {
            if (!process_cmd_heredoc(shell, cmd, current))
                return (0);
        }
        current = current->next;
    }
    return (1);
}

static void	cleanup_after_execution(t_cmd *cmd)
{
    t_cmd	*current;

    current = cmd;
    while (current)
    {
        if (current->heredoc_file)
        {
            unlink(current->heredoc_file);
            free(current->heredoc_file);
            current->heredoc_file = NULL;
        }
        if (current->input_fd != -1)
        {
            close(current->input_fd);
            current->input_fd = -1;
        }
        current = current->next;
    }
}

static int	run_pipeline_commands(t_shell *shell, t_cmd *cmd)
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
    return (0);
}

int execute_pipeline(t_shell *shell, t_cmd *cmd)
{
    int result;
    
    // Process all heredocs first
    if (!process_pipeline_heredocs(shell, cmd))
        return (1);
    
    // Run all commands in the pipeline
    if (run_pipeline_commands(shell, cmd))
        return (1);
    
    result = wait_for_children(shell);
    
    cleanup_after_execution(cmd);
    
    // Reset signal handlers
    setup_signals();
    
    return (result);
}
