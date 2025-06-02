/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:32:21 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 02:53:25 by my42             ###   ########.fr       */
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
 * Check line against delimiter for heredoc
 */
int check_heredoc_line(char *line, char *delimiter, int fd)
{
    int len;
    char *line_copy;

    if (!line)
        return (0);
        
    // Make a copy for comparison (to preserve original for writing)
    line_copy = ft_strdup(line);
    if (!line_copy)
    {
        free(line);
        return (0);
    }
    
    // Remove newline for comparison only
    len = ft_strlen(line_copy);
    if (len > 0 && line_copy[len - 1] == '\n')
        line_copy[len - 1] = '\0';
        
    // Compare with delimiter
    if (ft_strcmp(line_copy, delimiter) == 0)
    {
        free(line_copy);
        free(line);
        return (1);
    }
    
    // Write the original line to the file
    write(fd, line, ft_strlen(line));
    
    // Ensure we have a newline
    if (line[ft_strlen(line) - 1] != '\n')
        write(fd, "\n", 1);
    
    free(line_copy);
    free(line);
    return (0);
}

/**
 * Collect heredoc content
 */
int collect_heredoc_input(char *delimiter, int fd)
{
    char *line;
    
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
        
        if (check_heredoc_line(line, delimiter, fd))
            break;
    }
    
    // Sync the file to ensure all content is written
    fsync(fd);
    
    setup_signals();
    return (1);
}

/**
 * Set up heredoc input
 */
static int	setup_heredoc(t_cmd *cmd)
{
    int	fd;
    int	result;

    if (!cmd->heredoc_delim)
        return (0);
        
    // Create temporary file
    fd = open(".heredoc.tmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Collect heredoc content
    result = collect_heredoc_input(cmd->heredoc_delim, fd);
    close(fd);
    
    if (result == 0)
    {
        unlink(".heredoc.tmp");
        return (1);
    }
    
    // Reopen for reading
    fd = open(".heredoc.tmp", O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        unlink(".heredoc.tmp");
        return (1);
    }
    
    // Store the fd in the command for better tracking
    cmd->input_fd = fd;
    
    // Connect to stdin
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        cmd->input_fd = -1;
        unlink(".heredoc.tmp");
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // We don't close fd here so it can be cleaned up properly
    // in case of complex command chains
    unlink(".heredoc.tmp");  // Remove temp file
    return (0);
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
