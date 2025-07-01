/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_close.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 19:38:41 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/26 19:40:54 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * cleanup_shell_file_descriptors - Close all non-standard file descriptors
 * @shell: Shell structure containing saved standard descriptors
 * 
 * This function closes all open file descriptors from 3 to OPEN_MAX,
 * except for the shell's saved stdin and stdout. After closing
 * non-standard FDs, it also closes and resets the saved standard FDs.
 * 
 * Called during shell cleanup or after pipeline execution to prevent
 * file descriptor leaks.
 */
void	cleanup_shell_file_descriptors(t_shell *shell)
{
    int	fd;

    fd = 3;
    while (fd < OPEN_MAX)
    {
        if (fd != shell->saved_stdin && fd != shell->saved_stdout)
            close(fd);
        fd++;
    }
    if (shell->saved_stdin >= 0)
    {
        close(shell->saved_stdin);
        shell->saved_stdin = -1;
    }
    if (shell->saved_stdout >= 0)
    {
        close(shell->saved_stdout);
        shell->saved_stdout = -1;
    }
}

/**
 * close_all_non_standard_fds - Close all file descriptors above standard ones
 * 
 * This function closes all file descriptors from 3 to OPEN_MAX,
 * preserving only standard input (0), output (1), and error (2).
 * 
 * Used before exec calls to prevent leaking file descriptors to
 * child processes that don't need them.
 */
void	close_all_non_standard_fds(void)
{
    int	fd;

    fd = 3;
    while (fd < OPEN_MAX)
    {
        close(fd);
        fd++;
    }
}

/**
 * close_unused_command_fds - Close file descriptors for all other commands
 * @all_cmds: Linked list of all commands in the pipeline
 * @current_cmd: The current command being executed
 * 
 * This function walks through the command list and closes input/output
 * file descriptors for all commands except the current one. This prevents
 * descriptor leaks and ensures each command only has access to its own
 * redirections.
 * 
 * Critical for proper pipeline execution to ensure file descriptors
 * aren't exhausted during complex command sequences.
 */
void	close_unused_command_fds(t_command *all_cmds, t_command *current_cmd)
{
    t_command	*cmd;

    cmd = all_cmds;
    while (cmd)
    {
        if (cmd != current_cmd)
        {
            if (cmd->fd_in != -1 && cmd->fd_in > 2)
                close(cmd->fd_in);
            if (cmd->fd_out != -1 && cmd->fd_out > 2)
                close(cmd->fd_out);
        }
        cmd = cmd->next;
    }
}
/**
 * setup_heredoc_pipe - Creates a pipe containing heredoc content
 * @cmd: Command structure to attach the heredoc input to
 * @redir: Redirection structure containing heredoc content
 * 
 * This function creates a pipe, writes the heredoc content to the
 * write end, then closes it. The read end is assigned as the command's
 * input file descriptor, allowing the command to read the heredoc
 * content as if it were coming from a file.
 * 
 * Returns: 0 on success, -1 on pipe creation failure
 */
int	setup_heredoc_pipe(t_command *cmd, t_redir *redir)
{
    int	pipe_fd[2];

    if (pipe(pipe_fd) == -1)
        return (-1);
    write(pipe_fd[1], redir->heredoc_content,
        ft_strlen(redir->heredoc_content));
    close(pipe_fd[1]);
    if (cmd->fd_in != STDIN_FILENO)
        close(cmd->fd_in);
    cmd->fd_in = pipe_fd[0];
    return (0);
}
