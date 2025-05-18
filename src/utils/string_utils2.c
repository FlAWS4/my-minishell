/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strings_utils2.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 20:19:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/17 20:24:38 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Zero-fill memory area
 */
void	ft_bzero(void *s, size_t n)
{
    size_t	i;

    i = 0;
    while (i < n)
    {
        ((unsigned char *)s)[i] = 0;
        i++;
    }
}

/**
 * Join two strings
 */
char	*ft_strjoin(const char *s1, const char *s2)
{
    char	*result;
    size_t	i;
    size_t	j;

    if (!s1 || !s2)
        return (NULL);
    result = (char *)malloc(ft_strlen(s1) + ft_strlen(s2) + 1);
    if (!result)
        return (NULL);
    i = 0;
    while (s1[i])
    {
        result[i] = s1[i];
        i++;
    }
    j = 0;
    while (s2[j])
    {
        result[i + j] = s2[j];
        j++;
    }
    result[i + j] = '\0';
    return (result);
}

/**
 * Join two strings and free the first one
 */
char	*ft_strjoin_free(char *s1, const char *s2)
{
    char	*result;

    result = ft_strjoin(s1, s2);
    free(s1);
    return (result);
}

/**
 * Write string to file descriptor
 */
void	ft_putstr_fd(char *s, int fd)
{
    size_t	len;

    if (!s)
        return ;
    len = ft_strlen(s);
    write(fd, s, len);
}
