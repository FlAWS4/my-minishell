/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 21:06:26 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Duplicate a string
 */
char	*ft_strdup(const char *s)
{
    char	*dest;
    size_t	len;
    size_t	i;

    i = 0;
    len = ft_strlen(s);
    dest = (char *)malloc(sizeof(char) * len + 1);
    if (!dest)
        return (NULL);
    while (s[i] != '\0')
    {
        dest[i] = s[i];
        i++;
    }
    dest[i] = '\0';
    return (dest);
}

/**
 * Compare two strings
 */
int	ft_strcmp(const char *s1, const char *s2)
{
    int	i;

    i = 0;
    while (s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * Get string length
 */
size_t	ft_strlen(const char *s)
{
    size_t	i;

    i = 0;
    while (s[i])
        i++;
    return (i);
}

/**
 * Copy string with size limit
 */
size_t	ft_strlcpy(char *dst, const char *src, size_t size)
{
    size_t	i;
    size_t	len;

    i = 0;
    len = 0;
    while (src[len] != '\0')
        len++;
    if (size == 0)
        return (len);
    while (src[i] != '\0' && i < size - 1)
    {
        dst[i] = src[i];
        i++;
    }
    if (size > 0)
        dst[i] = '\0';
    return (len);
}

/**
 * Extract substring from string
 */
char	*ft_substr(char const *s, unsigned int start, size_t len)
{
    unsigned int	i;
    char			*str;

    if (!s)
        return (NULL);
    if (start >= ft_strlen(s))
    {
        str = ft_calloc(1, sizeof(char));
        if (!str)
            return (NULL);
        return (str);
    }
    if (ft_strlen(s) - start < len)
        len = ft_strlen(s) - start;
    str = ft_calloc(sizeof(char), len + 1);
    if (!str)
        return (NULL);
    i = 0;
    while (s[start + i] && i < len)
    {
        str[i] = s[start + i];
        i++;
    }
    return (str);
}

void	*ft_memset(void *s, int c, size_t n)
{
	unsigned char	*dst;

	dst = (unsigned char *) s;
	while (n > 0)
	{
		*dst = (unsigned char)c;
		dst++;
		n--;
	}
	return (s);
}
