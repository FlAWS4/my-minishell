/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   process_redir.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 23:45:45 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/29 00:05:03 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process all heredocs in sequence, but only connect the last one

static int	collect_all_heredocs(t_cmd *cmd, t_redirection *last_heredoc)
{
    t_redirection	*redir;

    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            if (redir == last_heredoc)
            {
                if (process_heredoc_redir(redir) != 0)
                    return (1);
            }
            else
            {
                if (collect_and_discard_heredoc(redir->word) != 1)
                    return (1);
            }
        }
        redir = redir->next;
    }
    return (0);
}
*/

/**
 * Check line against delimiter for heredoc
 */
int	check_heredoc_line(char *line, char *delimiter, int fd)
{
    int	len;

    if (!line)
        return (0);
    len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
    if (ft_strcmp(line, delimiter) == 0)
    {
        free(line);
        return (1);
    }
    ft_putstr_fd(line, fd);
    ft_putstr_fd("\n", fd);
    free(line);
    return (0);
}

/**
 * Check line against delimiter for heredoc (discard version)
 */
static int	check_heredoc_discard(char *line, char *delimiter)
{
    int	len;

    if (!line)
        return (0);
    len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
    if (ft_strcmp(line, delimiter) == 0)
    {
        free(line);
        return (1);
    }
    // Just discard the line, don't write it anywhere
    free(line);
    return (0);
}

/**
 * Collect heredoc content but discard it (for non-last heredocs)
 */
int	collect_and_discard_heredoc(char *delimiter)
{
    char	*line;

    setup_signals_heredoc();
    if (g_signal == SIGINT)
    {
        setup_signals();
        return (0);
    }
    while (1)
    {
        if (isatty(STDIN_FILENO))
            ft_putstr_fd("> ", 1);
        line = get_next_line(STDIN_FILENO);
        if (g_signal == SIGINT || !line)
        {
            setup_signals();
            return (0);
        }
        if (check_heredoc_discard(line, delimiter))
            break;
    }
    setup_signals();
    return (1);
}

/**
 * Find the last heredoc in redirections list
 */
static t_redirection	*find_last_heredoc(t_redirection *redir_list)
{
    t_redirection	*last_heredoc;
    t_redirection	*current;

    last_heredoc = NULL;
    current = redir_list;
    while (current)
    {
        if (current->type == TOKEN_HEREDOC)
            last_heredoc = current;
        current = current->next;
    }
    return (last_heredoc);
}

int	process_redirections(t_cmd *cmd)
{
    t_redirection	*redir;
    t_redirection	*last_heredoc;

    if (!cmd || !cmd->redirections)
        return (0);
    g_signal = 0;
    
    // Process heredocs first
    last_heredoc = find_last_heredoc(cmd->redirections);
    redir = cmd->redirections;
    while (redir)
    {
        if (redir->type == TOKEN_HEREDOC)
        {
            if (redir == last_heredoc)
            {
                if (process_heredoc_redir(redir) != 0)
                    return (1);
            }
            else
            {
                if (collect_and_discard_heredoc(redir->word) != 1)
                    return (1);
            }
        }
        redir = redir->next;
    }
    
    // Then process other redirections
    redir = cmd->redirections;
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
