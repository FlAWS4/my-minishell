/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_utils3.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 20:32:52 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 18:34:16 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Allocate and zero-initialize memory
 * Checks for overflow before allocation
 */
void	*ft_calloc(size_t nmemb, size_t size)
{
    void	*mem;

    if (size != 0 && nmemb * size / size != nmemb)
        return (NULL);
    mem = malloc(nmemb * size);
    if (!mem)
        return (NULL);
    ft_bzero(mem, nmemb * size);
    return (mem);
}

/**
 * Count words in a string
 * Words are defined as sequences of characters separated by delimiter c
 */
int	count_words_split(const char *s, char c)
{
    int	count;
    int	i;

    count = 0;
    i = 0;
    while (s[i])
    {
        while (s[i] && s[i] == c)
            i++;
        if (s[i])
            count++;
        while (s[i] && s[i] != c)
            i++;
    }
    return (count);
}

/**
 * Extract next word from string
 * Updates the index pointer to position after the word
 */
static char	*get_next_word(const char *s, char c, int *i)
{
    int		len;
    int		start;
    char	*word;

    while (s[*i] && s[*i] == c)
        (*i)++;
    start = *i;
    len = 0;
    while (s[*i] && s[*i] != c)
    {
        len++;
        (*i)++;
    }
    word = ft_substr(s, start, len);
    return (word);
}

/**
 * Clean up split result on error
 * Frees all allocated memory
 */
static void	free_split(char **result, int count)
{
    int	i;

    i = 0;
    while (i < count)
    {
        free(result[i]);
        i++;
    }
    free(result);
}

/**
 * Split a string by delimiter
 * Returns array of strings, last element is NULL
 */
char	**ft_split(const char *s, char c)
{
    int		i;
    int		j;
    int		word_count;
    char	**result;

    if (!s)
        return (NULL);
    word_count = count_words_split(s, c);
    result = (char **)ft_calloc(word_count + 1, sizeof(char *));
    if (!result)
        return (NULL);
    i = 0;
    j = 0;
    while (j < word_count)
    {
        result[j] = get_next_word(s, c, &i);
        if (!result[j])
        {
            free_split(result, j);
            return (NULL);
        }
        j++;
    }
    result[j] = NULL;
    return (result);
}

/**
 * Check if string is composed of digits only
 * Returns 1 if true, 0 if false
 */
int	ft_str_is_numeric(const char *str)
{
    int	i;

    if (!str || !*str)
        return (0);
    i = 0;
    while (str[i])
    {
        if (!ft_isdigit(str[i]))
            return (0);
        i++;
    }
    return (1);
}
int ft_strncmp(const char *s1, const char *s2, size_t n)
{
    size_t i;

    i = 0;
    if (n == 0)
        return (0);
    while (i < n - 1 && s1[i] && s2[i] && s1[i] == s2[i])
        i++;
    return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}
void	*ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
	{
		((unsigned char *)dest)[i] = ((unsigned char *)src)[i];
		i++;
	}
	return (dest);
}