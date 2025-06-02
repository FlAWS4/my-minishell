/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:32:19 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 02:47:31 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process input redirection from list
 */
int	process_input_redir(t_redirection *redir)
{
    int	fd;

    fd = open(redir->word, O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    close(fd);
    return (0);
}

/**
 * Process output redirection from list
 */
int	process_output_redir(t_redirection *redir)
{
    int	fd;
    int	flags;

    flags = O_WRONLY | O_CREAT;
    if (redir->type == TOKEN_REDIR_APPEND)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    fd = open(redir->word, flags, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        display_error(ERR_REDIR, redir->word, strerror(errno));
        return (1);
    }
    close(fd);
    return (0);
}
/**
 * Process heredoc redirection
 */
int process_heredoc_redir(t_redirection *redir)
{
    int fd;
    
    // Create temporary file
    fd = open(".heredoc.tmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Collect heredoc content into file
    if (!collect_heredoc_input(redir->word, fd))
    {
        close(fd);
        unlink(".heredoc.tmp");
        return (1);
    }
    
    close(fd);
    
    // Reopen file for reading
    fd = open(".heredoc.tmp", O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        unlink(".heredoc.tmp");
        return (1);
    }
    
    // Connect to stdin
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        unlink(".heredoc.tmp");
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    close(fd);
    unlink(".heredoc.tmp");  // Remove temp file
    return (0);
}
/**
 * Process a single redirection based on type
 */
int	process_single_redir(t_redirection *redir)
{
    if (redir->type == TOKEN_REDIR_IN)
        return (process_input_redir(redir));
    else if (redir->type == TOKEN_REDIR_OUT || 
            redir->type == TOKEN_REDIR_APPEND)
        return (process_output_redir(redir));
    else if (redir->type == TOKEN_HEREDOC)
        return (process_heredoc_redir(redir));
    return (0);
}
