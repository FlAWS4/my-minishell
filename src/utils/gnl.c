/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:14:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 20:22:48 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 128
#endif

#ifndef MAX_FD
# define MAX_FD 1024
#endif

// Moved to file scope so gnl_cleanup can access it
static char	*g_buffers[MAX_FD];

/**
 * Find first occurrence of character in string
 */
static char	*ft_strchr_gnl(char *s, int c)
{
    int	i;

    i = 0;
    if (!s)
        return (NULL);
    while (s[i] != '\0')
    {
        if (s[i] == (char) c)
            return ((char *)&s[i]);
        i++;
    }
    return (NULL);
}

/**
 * Extract a line from the buffer up to newline
 */
static char	*extract_line(char *buffer)
{
    int		i;
    char	*line;

    i = 0;
    if (!buffer || !buffer[i])
        return (NULL);
        
    // Find length of line including newline if present
    while (buffer[i] && buffer[i] != '\n')
        i++;
    if (buffer[i] == '\n')
        i++;
        
    // Allocate memory for the line
    line = (char *)malloc(sizeof(char) * (i + 1));
    if (!line)
        return (NULL);
        
    // Copy characters to new line
    i = 0;
    while (buffer[i] && buffer[i] != '\n')
    {
        line[i] = buffer[i];
        i++;
    }
    if (buffer[i] == '\n')
        line[i++] = '\n';
    line[i] = '\0';
    
    return (line);
}

/**
 * Save the remainder of buffer after extracting a line
 */
static char	*save_remainder(char *buffer)
{
    int		i;
    int		j;
    char	*remainder;

    i = 0;
    if (!buffer)
        return (NULL);
        
    // Find the newline
    while (buffer[i] && buffer[i] != '\n')
        i++;
        
    // If no newline, nothing to save
    if (!buffer[i])
    {
        free(buffer);
        return (NULL);
    }
    
    // Allocate memory for remainder
    remainder = (char *)malloc(sizeof(char) * (ft_strlen(buffer) - i + 1));
    if (!remainder)
    {
        free(buffer);
        return (NULL);
    }
    
    // Copy remainder after newline
    i++;
    j = 0;
    while (buffer[i])
        remainder[j++] = buffer[i++];
    remainder[j] = '\0';
    
    free(buffer);
    return (remainder);
}

/**
 * Read from fd and append to buffer until newline or EOF
 */
static char	*read_and_append(int fd, char *buffer)
{
    char	*temp;
    char	*chunk;
    int		bytes_read;

    // Allocate read buffer
    chunk = malloc((BUFFER_SIZE + 1) * sizeof(char));
    if (!chunk)
    {
        if (buffer)
            free(buffer);
        return (NULL);
    }
    
    // Read until newline or EOF
    bytes_read = 1;
    while (!ft_strchr_gnl(buffer, '\n') && bytes_read > 0)
    {
        bytes_read = read(fd, chunk, BUFFER_SIZE);
        if (bytes_read < 0)
        {
            free(chunk);
            if (buffer)
                free(buffer);
            return (NULL);
        }
        
        chunk[bytes_read] = '\0';
        temp = buffer;
        buffer = ft_strjoin(buffer ? buffer : "", chunk);
        if (temp)
            free(temp);
        if (!buffer)
        {
            free(chunk);
            return (NULL);
        }
    }
    
    free(chunk);
    return (buffer);
}

/**
 * Get next line from file descriptor with support for multiple FDs
 */
char	*get_next_line(int fd)
{
    char	*line;

    // Validate input
    if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
        return (NULL);
        
    // Read from fd and append to buffer
    g_buffers[fd] = read_and_append(fd, g_buffers[fd]);
    if (!g_buffers[fd])
        return (NULL);
        
    // Extract line from buffer
    line = extract_line(g_buffers[fd]);
    if (!line)
    {
        free(g_buffers[fd]);
        g_buffers[fd] = NULL;
        return (NULL);
    }
    
    // Save remainder for next call
    g_buffers[fd] = save_remainder(g_buffers[fd]);
    
    return (line);
}

/**
 * Clean up any resources used by get_next_line for a specific fd
 * Call this when closing a file descriptor
 */
void	gnl_cleanup(int fd)
{
    if (fd >= 0 && fd < MAX_FD)
    {
        if (g_buffers[fd])
        {
            free(g_buffers[fd]);
            g_buffers[fd] = NULL;
        }
    }
}
