/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   gnl.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/27 22:14:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:59:23 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static char	*ft_strchr(char *s, int c)
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

static char	*get_lines(char *str)
{
    int		i;
    char	*s;

    i = 0;
    if (!str || !str[i])
        return (NULL);
    while (str[i] && str[i] != '\n')
        i++;
    if (str[i] == '\n')
        i++;
    s = (char *)malloc(sizeof(char) * (i + 1));
    if (!s)
        return (NULL);
    i = 0;
    while (str[i] && str[i] != '\n')
    {
        s[i] = str[i];
        i++;
    }
    if (str[i] == '\n')
        s[i++] = '\n';
    s[i] = '\0';
    return (s);
}

static char	*stock(char *str)
{
    int		i;
    int		j;
    char	*s;

    i = 0;
    if (!str)
        return (NULL);
    while (str[i] && str[i] != '\n')
        i++;
    if (!str[i])
    {
        free(str);
        return (NULL);
    }
    s = (char *)malloc(sizeof(char) * (ft_strlen(str) - i + 1));
    if (!s)
    {
        free(str);
        return (NULL);
    }
    i++;
    j = 0;
    while (str[i])
        s[j++] = str[i++];
    s[j] = '\0';
    free(str);
    return (s);
}

static char	*read_and_append(int fd, char *str)
{
    char	*buff;
    int		read_bytes;

    buff = malloc((BUFFER_SIZE + 1) * sizeof(char));
    if (!buff)
    {
        if (str)
            free(str);
        return (NULL);
    }
    read_bytes = 1;
    while (!ft_strchr(str, '\n') && read_bytes != 0)
    {
        read_bytes = read(fd, buff, BUFFER_SIZE);
        if (read_bytes < 0)
        {
            free(buff);
            if (str)
                free(str);
            return (NULL);
        }
        buff[read_bytes] = '\0';
        str = ft_strjoin(str, buff);
    }
    free(buff);
    return (str);
}

char	*get_next_line(int fd)
{
    char		*line;
    static char	*str;

    if (fd < 0 || BUFFER_SIZE <= 0)
        return (NULL);
    str = read_and_append(fd, str);
    if (!str)
        return (NULL);
    line = get_lines(str);
    if (!line)
    {
        free(str);
        str = NULL;
        return (NULL);
    }
    str = stock(str);
    return (line);
}