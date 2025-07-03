/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_helpers.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 16:38:04 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 16:38:04 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if character is alphanumeric
 */
int	ft_isalnum(int c)
{
	return ((c >= 'a' && c <= 'z') || \
		(c >= 'A' && c <= 'Z') || \
		(c >= '0' && c <= '9'));
}

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
 * Compare two strings
 */
int	ft_strcmp(const char *s1, const char *s2)
{
	int	i;

	if (!s1)
	{
		if (!s2)
			return (0);
		return (-1);
	}
	if (!s2)
		return (1);
	i = 0;
	while (s1[i] && s2[i] && s1[i] == s2[i])
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

/**
 * Safely adds two sizes with overflow check, 
 * including space for null terminator
 * Returns 1 if the addition is safe, 0 if overflow would occur
 */
int	ft_safe_size_add(size_t a, size_t b, size_t *result)
{
	if (a > SIZE_MAX - b)
		return (0);
	if ((a + b) > SIZE_MAX - 1)
		return (0);
	*result = a + b + 1;
	return (1);
}

char	*ft_strjoin(const char *s1, const char *s2)
{
	char	*result;
	size_t	i;
	size_t	j;

	if (!s1 || !s2)
		return (NULL);
	if (!ft_safe_size_add(ft_strlen(s1), ft_strlen(s2), &j))
		return (NULL);
	result = (char *)malloc(j);
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
