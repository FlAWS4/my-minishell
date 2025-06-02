/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils2.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 20:19:29 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 16:57:10 by my42             ###   ########.fr       */
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

char	*ft_itoa(int n)
{
	long	num;
	size_t	len;
	char	*str;

	num = (long)n;
	len = slen(n);
	str = (char *)malloc (len + 1);
	if (!str)
		return (NULL);
	*(str + len--) = '\0';
	if (n < 0)
		num *= -1;
	while (num > 0)
	{
		*(str + len--) = num % 10 + '0';
		num /= 10;
	}
	if (str[1] == '\0' && len == 0)
		*(str + len) = '0';
	else if (str[1] != '\0' && len == 0)
		*(str + len) = '-';
	return (str);
}