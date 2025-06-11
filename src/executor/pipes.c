/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 03:13:59 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle input redirection for child process
 */
static void	setup_child_input(int prev_pipe, t_cmd *current)
{
    // CRITICAL CHANGE: Only use the pipe for input if there's no heredoc input
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

/**
 * Handle output redirection for child process
 */
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


/**
 * Set up pipes for a child process
 */
static void	setup_child_pipes(int prev_pipe, int *pipe_fds, t_cmd *current)
{
    setup_child_input(prev_pipe, current);
    setup_child_output(pipe_fds, current);
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
 * Execute child process in pipeline
 */
static void	execute_pipeline_child(t_shell *shell, t_cmd *cmd, 
                                int prev_pipe, int *pipe_fds)
{
    setup_signals_noninteractive();
    setup_child_pipes(prev_pipe, pipe_fds, cmd);
    execute_child(shell, cmd);
    exit(1);
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
        execute_pipeline_child(shell, cmd, prev_pipe, pipe_fds);
    else
        cmd->pid = pid;
    return (0);
}

/**
 * Process signals when a child exits
 */
static void	process_child_signal(int signal_num)
{
    if (signal_num == SIGINT)
        write(STDERR_FILENO, "\n", 1);
}

/**
 * Handle waitpid return values
 */
static int	handle_waitpid_result(int pid, int *status, int *last_status)
{
    if (pid < 0)
    {
        if (errno == ECHILD)
            return (0);
        else if (errno == EINTR)
            return (2);
        else
            return (-1);
    }
    else if (pid == 0)
        return (0);
    
    if (WIFEXITED(*status))
        *last_status = WEXITSTATUS(*status);
    else if (WIFSIGNALED(*status))
    {
        process_child_signal(WTERMSIG(*status));
        *last_status = 128 + WTERMSIG(*status);
    }
    return (1);
}

/**
 * Wait for all child processes to complete
 */
int	wait_for_children(t_shell *shell)
{
    int	status;
    int	pid;
    int	last_status;
    int	result;

    if (!shell)
        return (1);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    last_status = 0;
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        result = handle_waitpid_result(pid, &status, &last_status);
        if (result <= 0)
            break;
        if (result == 2)
            continue;
    }
    setup_signals();
    write(STDOUT_FILENO, "", 0);
    fflush(stdout);
    fflush(stderr);
    shell->exit_status = last_status;
    return (last_status);
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
 * Clean up heredoc files
 */
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

/**
 * Process heredocs for a single command
 */
static int	process_cmd_heredoc(t_shell *shell, t_cmd *cmd, t_cmd *current)
{
    if (!process_heredoc(current, shell))
    {
        cleanup_heredoc_files(cmd, current);
        return (0);
    }
    return (1);
}

/**
 * Process heredocs for all commands in pipeline
 */
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

/**
 * Final cleanup after pipeline execution
 */
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

/**
 * Execute all commands in the pipeline
 */
static int	run_pipeline_commands(t_shell *shell, t_cmd *cmd)
{
    int     pipe_fds[2];
    t_cmd   *current;
    int     prev_pipe;

    // PRE-PROCESS ALL HEREDOCS BEFORE EXECUTING THE PIPELINE
    current = cmd;
    while (current)
    {
        if (current->heredoc_delim)
        {
            printf("DEBUG: Processing heredoc with delimiter '%s' for command: %s\n", 
                   current->heredoc_delim, 
                   current->args ? current->args[0] : "NULL");
                   
            if (!process_heredoc(current, shell))
            {
                // If process_heredoc failed, clean up any previous heredoc files
                t_cmd *cleanup = cmd;
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
                return (1);
            }
            {
                // Cleanup any previous heredoc files
                t_cmd *cleanup = cmd;
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
                return (1);
            }
            
            // Verify that process_heredoc set up the input_fd correctly
            printf("DEBUG: Heredoc processed, input_fd=%d, file=%s\n", 
                   current->input_fd, 
                   current->heredoc_file ? current->heredoc_file : "NULL");
        }
        current = current->next;
    }

    // NOW EXECUTE THE PIPELINE AS BEFORE
    prev_pipe = -1;
    current = cmd;
    while (current)
    {
        // Debug the command before execution
        printf("DEBUG: Executing piped command: %s (input_fd=%d)\n", 
               current->args ? current->args[0] : "NULL", 
               current->input_fd);
               
        if (execute_piped_command(shell, current, prev_pipe, pipe_fds))
            return (1);
        prev_pipe = manage_parent_pipes(prev_pipe, pipe_fds, current);
        current = current->next;
    }
    return (0);
}

/**
 * Execute a pipeline of commands
 */
int	execute_pipeline(t_shell *shell, t_cmd *cmd)
{
    int	result;

    if (!process_pipeline_heredocs(shell, cmd))
        return (1);
    
    if (run_pipeline_commands(shell, cmd))
        return (1);
    
    result = wait_for_children(shell);
    cleanup_after_execution(cmd);
    return (result);
}
