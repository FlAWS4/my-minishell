/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:32:19 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/29 00:12:15 by mshariar         ###   ########.fr       */
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
 * Set up a temporary file for heredoc content
 
static int	setup_heredoc_file(char *tmp_file, int *tmp_fd)
{
    ft_strlcpy(tmp_file, "/tmp/minishell_heredoc_XXXXXX", 32);
    *tmp_fd = mkstemp(tmp_file);
    unlink(tmp_file);
    
    if (*tmp_fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    return (0);
}
*/

/**
 * Process heredoc redirection
 */
int	process_heredoc_redir(t_redirection *redir)
{
    int		pipe_fds[2];
    int		result;

    ft_putstr_fd("DEBUG: Processing heredoc: ", 2);
    ft_putstr_fd(redir->word, 2);
    ft_putstr_fd("\n", 2);
    
    // Create pipe for heredoc content
    if (pipe(pipe_fds) == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Collect heredoc content into pipe
    result = collect_heredoc_input(redir->word, pipe_fds[1]);
    close(pipe_fds[1]); // Close write end
    
    if (result == 0)
    {
        close(pipe_fds[0]);
        return (1);
    }
    
    // Connect pipe to stdin
    if (dup2(pipe_fds[0], STDIN_FILENO) == -1)
    {
        close(pipe_fds[0]);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    close(pipe_fds[0]); // Close original FD after duplication
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
