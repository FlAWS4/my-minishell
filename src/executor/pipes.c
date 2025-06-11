/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 21:54:12 by mshariar         ###   ########.fr       */
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

static void execute_pipeline_child(t_shell *shell, t_cmd *cmd, 
                             int prev_pipe, int *pipe_fds)
{
    // Using cmd->pid as the pgid for all processes in the pipeline
    pid_t pgid = shell->cmd->pid;
    
    // Set process group ID for this child
    if (setpgid(0, pgid) == -1)
        ; // Ignore errors - might happen if parent already set it
    
    // Setup signals for child process
    setup_signals_noninteractive();
    
    // Setup pipes and redirections
    setup_child_pipes(prev_pipe, pipe_fds, cmd);
    
    // Ensure output buffers are flushed
    write(STDOUT_FILENO, "", 0);
    write(STDERR_FILENO, "", 0);
    
    // Execute the command
    execute_child(shell, cmd);
    exit(1);
}

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
    {
        cmd->pid = pid;
        
        // Set process group in parent to avoid race conditions
        if (cmd == shell->cmd) {
            // First command sets the pgid for the pipeline
            if (setpgid(pid, pid) == -1 && errno != EACCES)
                ; // Ignore errors - child might have already called setpgid
            
            // Give terminal control to the pipeline with error handling
            if (isatty(STDIN_FILENO)) {
                if (tcsetpgrp(STDIN_FILENO, pid) == -1)
                    ; // Ignore errors here too - might fail if child exits quickly
            }
        } else {
            // Other commands join the same process group
            if (setpgid(pid, shell->cmd->pid) == -1 && errno != EACCES)
                ; // Ignore errors
        }
    }
    return (0);
}

static void process_child_signal(int signal_num)
{
    if (signal_num == SIGINT)
        write(STDOUT_FILENO, "\n", 1);
}

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

int wait_for_children(t_shell *shell)
{
    int status;
    int pid;
    int last_status;
    int result;

    if (!shell)
        return (1);
    
    // Ignore signals in parent while waiting
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
    
    // Reset shell's signal handlers
    setup_signals();
    
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
    pid_t shell_pgid = getpgrp();
    struct sigaction sa_ttou;
    
    // Ignore SIGTTOU before changing terminal control
    sa_ttou.sa_handler = SIG_IGN;
    sigemptyset(&sa_ttou.sa_mask);
    sa_ttou.sa_flags = 0;
    sigaction(SIGTTOU, &sa_ttou, NULL);
    
    if (!process_pipeline_heredocs(shell, cmd))
        return (1);
    
    if (run_pipeline_commands(shell, cmd))
        return (1);
    
    result = wait_for_children(shell);
    
    // Restore terminal control to the shell
    if (isatty(STDIN_FILENO))
    {
        tcsetpgrp(STDIN_FILENO, shell_pgid);
        // Make sure we have proper terminal attributes
        tcsetattr(STDIN_FILENO, TCSANOW, &shell->orig_termios);
    }
    
    cleanup_after_execution(cmd);
    
    // Reset the signal handler
    setup_signals();
    
    return (result);
}
