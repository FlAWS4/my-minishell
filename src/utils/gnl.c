/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:48:00 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:48:00 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*ft_strstr(const char *haystack, const char *needle)
{
	size_t	i;
	size_t	j;

	if (!*needle)
		return ((char *)haystack);
	i = 0;
	while (haystack[i])
	{
		j = 0;
		while (haystack[i + j] && needle[j] && haystack[i + j] == needle[j])
			j++;
		if (!needle[j])
			return ((char *)&haystack[i]);
		i++;
	}
	return (NULL);
}

char	*extract_line(char *buffer)
{
	char	*line;
	char	*newline_pos;

	if (!buffer || !*buffer)
		return (NULL);
	newline_pos = ft_strchr(buffer, '\n');
	if (newline_pos)
	{
		line = ft_substr(buffer, 0, newline_pos - buffer + 1);
		if (!line)
			return (free(buffer), NULL);
		return (line);
	}
	line = ft_strdup(buffer);
	if (!line)
		return (free(buffer), NULL);
	return (line);
}

char	*update_buffer(char *buffer)
{
	char	*newline_pos;
	char	*new_buffer;

	if (!buffer)
		return (NULL);
	newline_pos = ft_strchr(buffer, '\n');
	if (newline_pos)
	{
		new_buffer = ft_strdup(newline_pos + 1);
		if (!new_buffer)
			return (free(buffer), NULL);
		free(buffer);
		return (new_buffer);
	}
	free(buffer);
	return (NULL);
}

char	*read_and_store(char *buffer, int fd)
{
	char	*tmp;
	char	*read_line;
	int		bytes_read;

	if (!buffer)
		return (NULL);
	read_line = (char *)malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!read_line)
		return (free(buffer), NULL);
	while (!ft_strchr(buffer, '\n'))
	{
		bytes_read = read(fd, read_line, BUFFER_SIZE);
		if (bytes_read < 0)
			return (free(buffer), free(read_line), NULL);
		if (bytes_read == 0)
			break ;
		read_line[bytes_read] = '\0';
		tmp = buffer;
		buffer = ft_strjoin(buffer, read_line);
		free(tmp);
		if (!buffer)
			return (free(read_line), NULL);
	}
	return (free(read_line), buffer);
}

char	*get_next_line(int fd, int clear)
{
	static char	*buffers[1024] = {NULL};
	char		*line;

	if (fd < 0 || fd >= 1024 || BUFFER_SIZE <= 0)
		return (NULL);
	if (clear == 1)
		return (free(buffers[fd]), buffers[fd] = NULL, NULL);
	if (!buffers[fd])
	{
		buffers[fd] = ft_strdup("");
		if (!buffers[fd])
			return (NULL);
	}
	buffers[fd] = read_and_store(buffers[fd], fd);
	if (!buffers[fd] || *buffers[fd] == '\0')
		return (free(buffers[fd]), buffers[fd] = NULL, NULL);
	line = extract_line(buffers[fd]);
	if (!line)
		return (buffers[fd] = NULL, NULL);
	buffers[fd] = update_buffer(buffers[fd]);
	return (line);
}
