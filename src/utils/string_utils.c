/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 18:27:37 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Duplicate a string
 */
char *ft_strdup(const char *s)
{
    char    *dest;
    size_t  len;
    size_t  i;

    if (!s)
        return (NULL);
        
    i = 0;
    len = ft_strlen(s);
    dest = (char *)malloc(sizeof(char) * (len + 1));
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
 * Get string length
 */
size_t ft_strlen(const char *s)
{
    size_t  i;

    if (!s)
        return (0);
        
    i = 0;
    while (s[i])
        i++;
    return (i);
}
/**
 * Copy string with size limit
 */
size_t ft_strlcpy(char *dst, const char *src, size_t size)
{
    size_t src_len;
    size_t i;

    if (!src)
        return (0); // Added NULL check
        
    src_len = ft_strlen(src); // Calculate length once
    
    if (!dst || size == 0)
        return (src_len);
        
    i = 0;
    while (src[i] && i < size - 1)
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
    return (src_len);
}


int	end_with_pipe(char *input)
{
	int	i;

	i = 0;
	if (!input)
		return (0);
	while (input[i])
		i++;
	i--;
	if (input[i] == '|')
		return (1);
	return (0);
}

int	is_whitespace_bis(char *str)
{
	int	i;

	i = 0;
	while (str[i])
	{
		if (!is_whitespace(str[i]))
			return (0);
		i++;
	}
	return (1);
}
