/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 20:19:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 00:28:16 by mshariar         ###   ########.fr       */
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
 * Safely adds two sizes with overflow check, including space for null terminator
 * Returns 1 if the addition is safe, 0 if overflow would occur
 */
int ft_safe_size_add(size_t a, size_t b, size_t *result)
{
    // Check if a + b would overflow
    if (a > SIZE_MAX - b)
        return (0);
    
    // Check if (a + b + 1) would overflow (for null terminator)
    if ((a + b) > SIZE_MAX - 1)
        return (0);
    
    *result = a + b + 1;
    return (1);
}

/**
 * Join two strings
 */
char *ft_strjoin(const char *s1, const char *s2)
{
    char    *result;
    size_t  i;
    size_t  j;
    size_t  total_size;

    if (!s1 || !s2)
        return (NULL);
        
    if (!ft_safe_size_add(ft_strlen(s1), ft_strlen(s2), &total_size))
        return (NULL);   
    result = (char *)malloc(total_size);
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

/**
 * Check if character is alphanumeric
 */
int	ft_isalnum(int c)
{
    return ((c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') || 
        (c >= '0' && c <= '9'));
}
int	slen(int n)
{
	size_t	size;

	if (n > 0)
		size = 0;
	else
		size = 1;
	while (n)
	{
		n = n / 10;
		size++;
	}
	return (size);
}

char *ft_itoa(int n)
{
    long    num;
    size_t  len;
    char    *str;

    num = (long)n;
    len = slen(n);
    str = (char *)malloc(len + 1);
    if (!str)
        return (NULL);
    str[len] = '\0';
    if (n == 0)
        str[0] = '0';
    if (n < 0)
    {
        str[0] = '-';
        num *= -1;
    }
    while (num > 0)
    {
        str[--len] = (num % 10) + '0';
        num /= 10;
    }
    return (str);
}
/**
 * Check if character is alphabetic
 */
int ft_isalpha(int c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
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
