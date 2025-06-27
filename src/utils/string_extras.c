/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_extras.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 16:45:41 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 16:45:41 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Write string to file descriptor
 */
void	ft_putstr_fd(const char *s, int fd)
{
    size_t	len;

    if (!s)
        return ;
    len = ft_strlen(s);
    write(fd, s, len);
}


/**
 * Find the first occurrence of the substring needle in the string haystack
 * Return a pointer to the beginning of the located substring,
 * or NULL if the substring is not found
 */
char *ft_strstr(const char *haystack, const char *needle)
{
    size_t i;
    size_t j;

    // If needle is empty, return haystack
    if (!*needle)
        return ((char *)haystack);
    
    i = 0;
    while (haystack[i])
    {
        j = 0;
        // Check if substring matches starting at this position
        while (haystack[i + j] && needle[j] && haystack[i + j] == needle[j])
            j++;
        
        // If we reached the end of needle, we found a match
        if (!needle[j])
            return ((char *)&haystack[i]);
        
        i++;
    }
    
    // If we get here, no match was found
    return (NULL);
}
/**
 * Find the first occurrence of character c in the string s
 * Return a pointer to the located character, or NULL if not found
 */
char *ft_strchr(const char *s, int c)
{
    while (*s != '\0')
    {
        if (*s == (char)c)
            return ((char *)s);
        s++;
    }
    
    // Also check the null terminator if c is '\0'
    if ((char)c == '\0')
        return ((char *)s);
    
    return (NULL);
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