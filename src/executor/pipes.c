/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 00:46:58 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Set up pipes for a child process
 */
static void	setup_child_pipes(int prev_pipe, int *pipe_fds, t_cmd *current)
{
    // CRITICAL CHANGE: Only use the pipe for input if there's no heredoc input
    if (prev_pipe != -1)
    {
        // Only use the pipe input if we don't have a heredoc input
        if (current->input_fd == -1)
        {
            printf("DEBUG: Using pipe input (fd=%d) for command\n", prev_pipe);
            if (dup2(prev_pipe, STDIN_FILENO) == -1)
                exit(1);
        }
        else
        {
            printf("DEBUG: Using heredoc input (fd=%d) instead of pipe input\n", 
                   current->input_fd);
            if (dup2(current->input_fd, STDIN_FILENO) == -1)
                exit(1);
        }
        close(prev_pipe);
    }
    else if (current->input_fd != -1)
    {
        // If we have a heredoc but no previous pipe
        printf("DEBUG: Using heredoc input (fd=%d) as stdin\n", current->input_fd);
        if (dup2(current->input_fd, STDIN_FILENO) == -1)
            exit(1);
        close(current->input_fd);
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
    else
    {
        // Store the pid in the command structure for better tracking
        cmd->pid = pid;
    }
    return (0);
}

/**
 * Wait for all child processes to complete
 */
int wait_for_children(t_shell *shell)
{
    int status;
    int pid;
    int last_status;

    if (!shell)
        return (1);
    
    // Temporarily ignore signals while waiting for children
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
        
    last_status = 0;
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        if (pid < 0)
        {
            if (errno == ECHILD)  // No more children
                break;
            else if (errno == EINTR)
                continue;  // Retry if interrupted by signal
            else
                return (1);  // Error occurred
        }
        else if (pid == 0)  // No children have exited
            break;
            
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
        {
            if (WTERMSIG(status) == SIGINT)
                write(STDERR_FILENO, "\n", 1); // Print newline for Ctrl+C
            last_status = 128 + WTERMSIG(status);
        }
    }
    
    // Restore signal handlers
    setup_signals();
    
    // Force flush any pending output
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
 * Execute a pipeline of commands
 */
int execute_pipeline(t_shell *shell, t_cmd *cmd)
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
    
    int result = wait_for_children(shell);
    
    // CLEANUP HEREDOC FILES AFTER EXECUTION
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
    
    return result;
}
