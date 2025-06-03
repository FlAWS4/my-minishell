/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirection_list.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:32:19 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 04:48:41 by my42             ###   ########.fr       */
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
 * Process heredoc content in child process
 */
static int collect_heredoc_content(char *delimiter, int fd)
{
    char *line;
    
    // Set up heredoc specific signal handling
    setup_signals_heredoc();
    
    // Reset any existing buffers
    reset_gnl_buffer();
    
    // Read heredoc content
    ft_putstr_fd("heredoc> ", STDOUT_FILENO);
    line = get_next_line(STDIN_FILENO);
    
    while (line != NULL)
    {
        // Remove newline for comparison
        int len = ft_strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';
        
        // Check for delimiter
        if (ft_strcmp(line, delimiter) == 0)
        {
            free(line);
            break;
        }
        
        // Write line to file with newline
        ft_putstr_fd(line, fd);
        ft_putstr_fd("\n", fd);
        
        free(line);
        ft_putstr_fd("heredoc> ", STDOUT_FILENO);
        line = get_next_line(STDIN_FILENO);
    }
    
    if (line)
        free(line);
        
    exit(0);
}

/**
 * Process heredoc redirection
 */
int process_heredoc_redir(t_redirection *redir)
{
    int fd;
    char *temp_file;
    pid_t pid;
    int status;
    
    // Use unique temp file name to avoid conflicts with multiple heredocs
    temp_file = ft_strdup("/tmp/minishell_heredoc_XXXXXX");
    if (!temp_file)
    {
        display_error(ERROR_MEMORY, "heredoc", "Memory allocation failed");
        return (1);
    }
    
    // Create temporary file with unique name
    fd = mkstemp(temp_file);
    if (fd == -1)
    {
        free(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Use fork to handle heredoc input in a separate process
    pid = fork();
    if (pid == -1)
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        display_error(ERR_FORK, "heredoc", strerror(errno));
        return (1);
    }
    
    if (pid == 0)
    {
        // Child process collects heredoc content
        collect_heredoc_content(redir->word, fd);
        // Not reached
        exit(0);
    }
    
    // Parent process
    signal(SIGINT, SIG_IGN);
    waitpid(pid, &status, 0);
    setup_signals();
    
    // Check if heredoc was interrupted
    if (WIFSIGNALED(status) && WTERMSIG(status) == SIGINT)
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        g_signal = SIGINT;
        return (1);
    }
    
    close(fd);
    
    // Reopen file for reading
    fd = open(temp_file, O_RDONLY);
    if (fd == -1)
    {
        unlink(temp_file);
        free(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Connect to stdin
    if (dup2(fd, STDIN_FILENO) == -1)
    {
        close(fd);
        unlink(temp_file);
        free(temp_file);
        display_error(ERR_REDIR, "heredoc", strerror(errno));
        return (1);
    }
    
    // Clean up
    close(fd);
    unlink(temp_file);
    free(temp_file);
    
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
