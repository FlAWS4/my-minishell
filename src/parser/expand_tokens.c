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

/**
 * link_token_to_list - Inserts a new token after the current token
 * @current: The current token after which to insert the new token
 * @new_token: The new token to insert
 *
 * Adjusts the pointers of the current and new tokens 
 * to maintain the linked list structure.
 */

static void	link_token_to_list(t_token *current, t_token *new_token)
{
	new_token->next = current->next;
	new_token->previous = current;
	if (current->next)
		current->next->previous = new_token;
	current->next = new_token;
}

/**
 * insert_token - Inserts new tokens into the linked list
 *  starting from the current token
 * @current: The current token to start inserting after
 * @words: Array of words to create new tokens from
 *
 * Creates new tokens for each word in the array and links 
 * them to the current token.
 * Returns 0 on success, -1 on failure.
 */

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
		link_token_to_list(token, new_token);
		token = new_token;
		i++;
	}
	return (0);
}

/**
 * split_tokens - Splits a string into tokens based on spaces
 *  and inserts them into the current token
 * @current: The current token to insert the first word into
 * @str: The string to split into tokens
 *
 * If the string contains only one word, it duplicates the 
 * string and assigns it to the current token.
 * If there are multiple words, it splits the 
 * string and creates new tokens for each word.
 * Returns 0 on success, -1 on failure, or 1 if multiple tokens were created.
 */

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
