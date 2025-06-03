/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redir_utils.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:55:39 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 04:30:23 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Read a line for heredoc input
 * Returns a newly allocated string without newline
 * Returns NULL on signal interrupt or EOF
 */
char *read_heredoc_line(void)
{
    char *line;
    int len;
    
    ft_putstr_fd("> ", 1);
    line = get_next_line(STDIN_FILENO);
    
    // Check global signal flag
    if (g_signal == SIGINT)
        return (NULL);
        
    if (!line)
        return (NULL);
        
    // Remove newline character
    len = ft_strlen(line);
    if (len > 0 && line[len - 1] == '\n')
        line[len - 1] = '\0';
        
    return (line);
}

/**
 * Output integer to file descriptor
 * Handles negative numbers correctly
 */
void	ft_putnbr_fd(int n, int fd)
{
    unsigned int	nb;

    if (fd < 0)
        return;
        
    if (n < 0)
    {
        ft_putchar_fd('-', fd);
        nb = -n;
    }
    else
        nb = (unsigned int)n;
        
    if (nb >= 10)
        ft_putnbr_fd(nb / 10, fd);
        
    ft_putchar_fd((char)(nb % 10 + '0'), fd);
}

/**
 * Reset get_next_line's static buffer
 * Useful after interrupting heredoc input
 */
void reset_gnl_buffer(void)
{
    // Create and immediately close a real file descriptor
    int temp_fd = open("/dev/null", O_RDONLY);
    char *temp;
    
    if (temp_fd != -1) {
        // Read from this fd to clear any lingering content
        temp = get_next_line(temp_fd);
        if (temp)
            free(temp);
        close(temp_fd);
    }
    
    // Also explicitly call with STDIN_FILENO to reset that buffer
    temp = get_next_line(STDIN_FILENO);
    if (temp)
        free(temp);
}
/**
 * Create a heredoc temporary file
 * Returns the file descriptor or -1 on error
 */
int create_heredoc_file(void)
{
    char filename[32];
    int fd;
    static int counter = 0;
    
    snprintf(filename, sizeof(filename), "/tmp/minishell_heredoc_%d", counter++);
    fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    
    if (fd == -1)
    {
        display_error(ERR_REDIR, "heredoc", "Failed to create temporary file");
        return (-1);
    }
    
    // Immediately unlink so file is cleaned up on close
    unlink(filename);
    return (fd);
}

/**
 * Handle input redirection
 * Returns file descriptor or -1 on error
 */
int handle_input_redirection(char *filename)
{
    int fd;
    
    if (!filename)
        return (-1);
        
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        display_error(ERR_REDIR, filename, strerror(errno));
        return (-1);
    }
    
    return (fd);
}

/**
 * Handle output redirection
 * Returns file descriptor or -1 on error
 */
int handle_output_redirection(char *filename, int append_mode)
{
    int fd;
    int flags;
    
    if (!filename)
        return (-1);
        
    flags = O_WRONLY | O_CREAT;
    flags |= (append_mode ? O_APPEND : O_TRUNC);
    
    fd = open(filename, flags, 0644);
    if (fd == -1)
    {
        display_error(ERR_REDIR, filename, strerror(errno));
        return (-1);
    }
    
    return (fd);
}
