/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/13 21:33:30 by mshariar         ###   ########.fr       */
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
        // Child process - remove setpgid calls
        setup_signals_noninteractive();
        setup_child_pipes(prev_pipe, pipe_fds, cmd);
        write(STDOUT_FILENO, "", 0);
        write(STDERR_FILENO, "", 0);
        execute_child(shell, cmd);
        exit(1);
    }
    else
    {
        cmd->pid = pid;
        
       // Change this code in execute_piped_command function
        if (cmd == shell->cmd && isatty(STDIN_FILENO)) 
        {
            // Only give terminal control if:
            // 1. It's not in a pipeline, OR
            // 2. It has input redirection (not reading from terminal)
            if (!cmd->next || cmd->input_fd != -1 || cmd->input_file || cmd->heredoc_file)
            {
                ioctl(STDIN_FILENO, TIOCSPGRP, &pid);
            }
        }
    }
    return (0);
}


int wait_for_children(t_shell *shell)
{
    int status;
    int pid;
    int last_status;
    struct sigaction sa_int, sa_quit, sa_old_int, sa_old_quit;
    
    if (!shell)
        return (1);
    
    // Use sigaction instead of signal() for more reliability
    sa_int.sa_handler = SIG_IGN;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, &sa_old_int);
    
    sa_quit.sa_handler = SIG_IGN;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = 0;
    sigaction(SIGQUIT, &sa_quit, &sa_old_quit);
    
    last_status = 0;
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
    
    // Restore previous signal handlers properly
    sigaction(SIGINT, &sa_old_int, NULL);
    sigaction(SIGQUIT, &sa_old_quit, NULL);
    
    // Ensure buffers are flushed
    write(STDOUT_FILENO, "", 0);
    write(STDERR_FILENO, "", 0);
    
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
    pid_t shell_pgid;
    struct sigaction sa_ttou;
    
    // Save shell's process group ID using allowed functions
    shell_pgid = getpid();  // Using getpid() which is allowed
    
    // Ignore SIGTTOU before changing terminal control
    sa_ttou.sa_handler = SIG_IGN;
    sigemptyset(&sa_ttou.sa_mask);
    sa_ttou.sa_flags = 0;
    sigaction(SIGTTOU, &sa_ttou, NULL);
    
    // Process all heredocs first
    if (!process_pipeline_heredocs(shell, cmd))
        return (1);
    
    // Run all commands in the pipeline
    if (run_pipeline_commands(shell, cmd))
        return (1);
    
    result = wait_for_children(shell);
    
    // Restore terminal control to the shell - more robust approach
    if (isatty(STDIN_FILENO))
    {
        // Use ioctl to set terminal process group (equivalent to tcsetpgrp)
        ioctl(STDIN_FILENO, TIOCSPGRP, &shell_pgid);
        
        // Always restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
    }
    
    cleanup_after_execution(cmd);
    
    // Reset signal handlers
    setup_signals();
    
    return (result);
}
