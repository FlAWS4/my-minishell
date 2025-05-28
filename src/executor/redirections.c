/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/29 00:19:09 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Set up input redirection
 */
static int	setup_input(t_cmd *cmd)
{
    int	fd;

    if (!cmd->input_file)
        return (0);
    fd = open(cmd->input_file, O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, cmd->input_file, strerror(errno));
        return (1);
    }
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        display_error(ERR_REDIR, cmd->input_file, strerror(errno));
        return (1);
    }
    close(fd);
    return (0);
}

/**
 * Set up output redirection
 */
static int	setup_output(t_cmd *cmd)
{
    int	fd;
    int	flags;

    if (!cmd->output_file)
        return (0);
    flags = O_WRONLY | O_CREAT;
    if (cmd->append_mode)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    fd = open(cmd->output_file, flags, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, cmd->output_file, strerror(errno));
        return (1);
    }
    if (dup2(fd, STDOUT_FILENO) == -1)
    {
        close(fd);
        display_error(ERR_REDIR, cmd->output_file, strerror(errno));
        return (1);
    }
    close(fd);
    return (0);
}

/**
 * Set up heredoc input
 */
static int	setup_heredoc(t_cmd *cmd)
{
    int	pipe_fds[2];
    int	result;

    if (!cmd->heredoc_delim)
        return (0);
    if (pipe(pipe_fds) == -1)
    {
        display_error(ERR_PIPE, "pipe", strerror(errno));
        return (1);
    }
    result = collect_heredoc_input(cmd->heredoc_delim, pipe_fds[1]);
    close(pipe_fds[1]);
    if (result == 0)
    {
        close(pipe_fds[0]);
        return (1);
    }
    if (dup2(pipe_fds[0], STDIN_FILENO) == -1)
    {
        close(pipe_fds[0]);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    close(pipe_fds[0]);
    return (0);
}
/**
 * Collect heredoc input and write to file descriptor
 */
int	collect_heredoc_input(char *delimiter, int fd)
{
    char	*line;
    int		len;

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
        len = ft_strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';
        if (ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        ft_putstr_fd(line, fd);
        ft_putstr_fd("\n", fd);
        free(line);
    }
    setup_signals();
    return (1);
}

/**
 * Set up redirections for a command
 */
int	setup_redirections(t_cmd *cmd)
{
    // Process redirections list if it exists
    if (cmd->redirections)
        return (process_redirections(cmd));
    
    // Only fall back to legacy methods if no redirections list
    if (cmd->heredoc_delim && setup_heredoc(cmd) != 0)
        return (1);
    if (cmd->input_file && setup_input(cmd) != 0)
        return (1);
    if (cmd->output_file && setup_output(cmd) != 0)
        return (1);
    
    return (0);
}