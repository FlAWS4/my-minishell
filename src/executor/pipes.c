/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/18 00:46:55 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle pipe creation failure - non-static to match header declaration
 */
int handle_pipe_error(t_shell *shell, char *context)
{
    (void)shell;
    display_error(ERR_PIPE, context, strerror(errno));
    return (1);
}

/**
 * Handle fork failure - non-static to match header declaration
 */
int handle_fork_error(t_shell *shell, char *context)
{
    (void)shell;
    display_error(ERR_FORK, context, strerror(errno));
    return (1);
}

/**
 * Clean up pipe resources on error
 */
static void cleanup_pipe_resources(int prev_pipe, int *pipe_fds, t_cmd *cmd)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    
    if (cmd && cmd->next)
    {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
    }
}

/**
 * Execute a command in the pipeline
 */
static int execute_pipe_command(t_shell *shell, t_cmd *cmd, 
                                int prev_pipe, int *pipe_fds)
{
    pid_t pid;

    // Process heredocs for this command first
    if (!cmd->heredocs_processed)
    {
        if (!process_heredoc(cmd, shell) && g_signal == SIGINT)
            return (1);
    }

    // Create pipe for next command if needed
    if (cmd->next && pipe(pipe_fds) == -1)
    {
        cleanup_pipe_resources(prev_pipe, NULL, NULL);
        return (handle_pipe_error(shell, "pipe"));
    }
    
    // Fork process
    pid = fork();
    if (pid == -1)
    {
        cleanup_pipe_resources(prev_pipe, pipe_fds, cmd);
        return (handle_fork_error(shell, "fork"));
    }
    
    if (pid == 0)
    {
        // Child process
        setup_signals_noninteractive();
        
        // 1. Handle pipe input
        if (cmd->input_fd != -1)
        {
            if (dup2(cmd->input_fd, STDIN_FILENO) == -1)
            {
                display_error(ERR_REDIR, "input", strerror(errno));
                exit(1);
            }
            close(cmd->input_fd);
            if (prev_pipe != -1)
                close(prev_pipe);
        }
        else if (prev_pipe != -1)
        {
            if (dup2(prev_pipe, STDIN_FILENO) == -1)
            {
                display_error(ERR_REDIR, "input", strerror(errno));
                exit(1);
            }
            close(prev_pipe);
        }
        
        // 2. Handle pipe output
        if (cmd->next)
        {
            close(pipe_fds[0]);
            if (cmd->output_fd == -1) // Only redirect to pipe if no file redirection
            {
                if (dup2(pipe_fds[1], STDOUT_FILENO) == -1)
                {
                    display_error(ERR_REDIR, "output", strerror(errno));
                    exit(1);
                }
            }
            close(pipe_fds[1]);
        }
        
        // 3. Process regular file redirections (skip heredocs already processed)
        process_regular_redirections(cmd);
        
        // 4. Process command arguments (handle colon prefixes)
        if (!process_command_args(shell, cmd))
            exit(1);
        
        // 5. Execute the command using shared function
        execute_cmd(shell, cmd);
        // execute_cmd never returns
    }
    else
    {
        // Parent - store child PID for later
        cmd->pid = pid;
    }
    return (0);
}

/**
 * Wait for all child processes and handle their exit status
 */
int wait_for_children(t_shell *shell)
{
    int status;
    int pid;
    int last_status;
    
    if (!shell)
        return (1);
    
    last_status = 0;
    while (1)
    {
        pid = waitpid(-1, &status, 0);
        if (pid <= 0)
        {
            if (errno == EINTR)
                continue;
            break;
        }
        
        if (WIFEXITED(status))
            last_status = WEXITSTATUS(status);
        else if (WIFSIGNALED(status))
        {
            // No need to print newline here since signal handler already did
            if (WTERMSIG(status) == SIGQUIT)
                ft_putendl_fd("Quit (core dumped)", STDERR_FILENO);
                
            last_status = 128 + WTERMSIG(status);
        }
    }
    
    shell->exit_status = last_status;
    return (last_status);
}

/**
 * Manage pipe file descriptors between commands
 */
static int manage_parent_pipes(int prev_pipe, int *pipe_fds, t_cmd *cmd)
{
    if (prev_pipe != -1)
        close(prev_pipe);
    
    if (cmd->next)
    {
        close(pipe_fds[1]);  // Close write end
        return (pipe_fds[0]);  // Return read end for next command
    }
    return (-1);  // No more pipes needed
}

/**
 * Run all commands in a pipeline
 */
static int run_pipeline_commands(t_shell *shell, t_cmd *cmd)
{
    int     pipe_fds[2];
    t_cmd   *current;
    int     prev_pipe;

    prev_pipe = -1;
    current = cmd;
    
    // Process each command in the pipeline
    while (current)
    {
        // Execute the command (heredocs now processed inside execute_pipe_command)
        if (execute_pipe_command(shell, current, prev_pipe, pipe_fds))
        {
            // Error already handled by execute_pipe_command
            return (1);
        }
        
        // Update pipe for next command
        prev_pipe = manage_parent_pipes(prev_pipe, pipe_fds, current);
        current = current->next;
    }
    return (0);
}

/**
 * Execute a pipeline of commands
 */
int execute_pipeline(t_shell *shell, t_cmd *cmd)
{
    int result;
    
    if (!shell || !cmd || !cmd->next)
        return (1);
    if (run_pipeline_commands(shell, cmd))
        return (1);
    
    // Wait for all processes to complete
    result = wait_for_children(shell);
    
    // Clean up any remaining resources
    cleanup_redirections(cmd);
    
    // Reset signal handlers
    setup_signals();
    
    return (result);
}
