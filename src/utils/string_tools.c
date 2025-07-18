/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   string_tools.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 16:37:26 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 16:37:26 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
