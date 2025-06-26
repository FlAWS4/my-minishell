/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expand_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:09:17 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:10:00 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	insert_token_bis(t_token *current, t_token *new_token)
{
	new_token->next = current->next;
	new_token->previous = current;
	if (current->next)
		current->next->previous = new_token;
	current->next = new_token;
}

static int	insert_token(t_token *current, char **words)
{
	t_token	*new_token;
	t_token	*token;
	int		i;

	token = current;
	current->ar = 1;
	i = 1;
	while (words[i])
	{
		new_token = create_token(WORD, words[i]);
		if (!new_token)
			return (-1);
		new_token->space_before = 1;
		token->space_after = 1;
		insert_token_bis(token, new_token);
		token = new_token;
		i++;
	}
	return (0);
}

int	split_tokens(t_token *current, char *str)
{
	char	**words;
	int		word_count;

	word_count = count_words_split(str, ' ');
	if (word_count <= 1)
	{
		current->value = ft_strdup(str);
		if (!current->value)
			return (-1);
		return (0);
	}
	words = ft_split(str, ' ');
	if (!words)
		return (-1);
	current->value = ft_strdup(words[0]);
	if (!current->value)
		return (free_array(words), -1);
	if (insert_token(current, words) == -1)
		return (free_array(words), -1);
	return (free_array(words), 1);
}
