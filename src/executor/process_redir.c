/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_redir.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 23:45:45 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:38:28 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process non-heredoc redirections
 */
static int	process_normal_redirs(t_redirection *redir)
{
    while (redir)
    {
        if (redir->type != TOKEN_HEREDOC)
        {
            if (process_single_redir(redir) != 0)
                return (1);
        }
        redir = redir->next;
    }
    return (0);
}

/**
 * Create pipe for heredocs
 */
static int	create_heredoc_pipe(int pipe_fds[2])
{
    if (pipe(pipe_fds) == -1)
    {
        display_error(ERR_PIPE, "heredoc", strerror(errno));
        return (1);
    }
    return (0);
}

/**
 * Process all heredocs into a single pipe
 */
static int	process_all_heredocs(t_redirection *redir, int pipe_fd)
{
    int	count;
    int	result;

    count = 0;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            result = collect_heredoc_input(redir->word, pipe_fd);
            if (result != 0)
                return (-1);
            count++;
        }
        redir = redir->next;
    }
    return (count);
}

/**
 * Process all redirections from the list
 */
int	process_redirections(t_cmd *cmd)
{
    t_redirection	*redir;
    int				pipe_fds[2];
    int				heredoc_count;

    redir = cmd->redirections;
    if (!redir)
        return (0);
    /* Process non-heredoc redirections first */
    if (process_normal_redirs(cmd->redirections) != 0)
        return (1);
    /* Process heredocs if needed */
    if (create_heredoc_pipe(pipe_fds) != 0)
        return (1);
    heredoc_count = process_all_heredocs(cmd->redirections, pipe_fds[1]);
    close(pipe_fds[1]);
    if (heredoc_count == -1)
    {
        close(pipe_fds[0]);
        return (1);
    }
    /* Connect heredoc pipe to stdin if any heredocs were processed */
    if (heredoc_count > 0)
    {
        if (dup2(pipe_fds[0], STDIN_FILENO) == -1)
            return (1);
    }
    close(pipe_fds[0]);
    return (0);
}
