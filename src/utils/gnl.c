/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:14:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/10 21:55:27 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*g_buffers[MAX_FD];

/**
 * Calculate line length for extraction
 */
static int	get_line_length(char *buffer)
{
    int	i;

    i = 0;
    while (buffer[i] && buffer[i] != '\n')
        i++;
    if (buffer[i] == '\n')
        i++;
    return (i);
}

/**
 * Copy buffer content to line
 */
static void	copy_to_line(char *line, char *buffer)
{
    int	i;

    i = 0;
    while (buffer[i] && buffer[i] != '\n')
    {
        line[i] = buffer[i];
        i++;
    }
    if (buffer[i] == '\n')
        line[i++] = '\n';
    line[i] = '\0';
}

/**
 * Extract a line from the buffer up to newline
 */
static char	*extract_line(char *buffer)
{
    int		len;
    char	*line;

    if (!buffer || !buffer[0])
        return (NULL);
    len = get_line_length(buffer);
    line = (char *)malloc(sizeof(char) * (len + 1));
    if (!line)
        return (NULL);
    copy_to_line(line, buffer);
    return (line);
}

/**
 * Find position of newline or end
 */
static int	find_newline_pos(char *buffer)
{
    int	i;

    i = 0;
    while (buffer[i] && buffer[i] != '\n')
        i++;
    return (i);
}

/**
 * Allocate memory for remainder buffer
 */
static char	*allocate_remainder(char *buffer, int i)
{
    char	*remainder;

    remainder = (char *)malloc(sizeof(char) * (ft_strlen(buffer) - i + 1));
    if (!remainder)
    {
        free(buffer);
        return (NULL);
    }
    return (remainder);
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
    i = find_newline_pos(buffer);
    if (!buffer[i])
    {
        free(buffer);
        return (NULL);
    }
    remainder = allocate_remainder(buffer, i);
    if (!remainder)
        return (NULL);
    i++;
    j = 0;
    while (buffer[i])
        remainder[j++] = buffer[i++];
    remainder[j] = '\0';
    free(buffer);
    return (remainder);
}

/**
 * Handle read error cleanup
 */
static char	*handle_read_error(char *chunk, char *buffer)
{
    free(chunk);
    if (buffer)
        free(buffer);
    return (NULL);
}

/**
 * Process read bytes and update buffer
 */
static char	*process_read(char *buffer, char *chunk, int bytes_read)
{
    char	*temp;
    char	*src;

    chunk[bytes_read] = '\0';
    temp = buffer;
    if (buffer)
        src = buffer;
    else
        src = "";
    buffer = ft_strjoin(src, chunk);
    if (temp)
        free(temp);
    if (!buffer)
    {
        free(chunk);
        return (NULL);
    }
    return (buffer);
}

/**
 * Check if a string contains a newline character
 */
static int	has_newline(char *str)
{
    int	i;

    if (!str)
        return (0);
    i = 0;
    while (str[i])
    {
        if (str[i] == '\n')
            return (1);
        i++;
    }
    return (0);
}

/**
 * Read from fd and append to buffer until newline or EOF
 */
static char	*read_and_append(int fd, char *buffer)
{
    char	*chunk;
    int		bytes_read;

    chunk = malloc((BUFFER_SIZE + 1) * sizeof(char));
    if (!chunk)
    {
        if (buffer)
            free(buffer);
        return (NULL);
    }
    bytes_read = 1;
    while (!has_newline(buffer) && bytes_read > 0)
    {
        bytes_read = read(fd, chunk, BUFFER_SIZE);
        if (bytes_read < 0)
            return (handle_read_error(chunk, buffer));
        buffer = process_read(buffer, chunk, bytes_read);
        if (!buffer)
            return (NULL);
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

    if (fd < 0 || fd >= MAX_FD || BUFFER_SIZE <= 0)
        return (NULL);
    g_buffers[fd] = read_and_append(fd, g_buffers[fd]);
    if (!g_buffers[fd])
        return (NULL);
    line = extract_line(g_buffers[fd]);
    if (!line)
    {
        free(g_buffers[fd]);
        g_buffers[fd] = NULL;
        return (NULL);
    }
    g_buffers[fd] = save_remainder(g_buffers[fd]);
    return (line);
}

/**
 * Clean up any resources used by get_next_line for a specific fd
 */
void	gnl_cleanup(int fd)
{
    if (fd >= 0 && fd < MAX_FD && g_buffers[fd])
    {
        free(g_buffers[fd]);
        g_buffers[fd] = NULL;
    }
}
