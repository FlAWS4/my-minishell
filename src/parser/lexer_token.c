/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_token.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:39:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:22:11 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * remove_useless_token - Removes a token from the linked list of tokens
 * @tokens: Pointer to the head of the linked list of tokens
 * @token: The token to be removed
 *
 * Adjusts the pointers of the previous and next tokens to remove the specified
 * token from the linked list. Frees the memory allocated for the token's value
 * and the token itself.
 */

void	remove_useless_token(t_token **tokens, t_token *token)
{
	t_token	*previous;
	t_token	*next;

	previous = token->previous;
	next = token->next;
	if (previous)
		previous->next = next;
	else
		*tokens = next;
	if (next)
		next->previous = previous;
	if (token->value)
		free(token->value);
	free(token);
}

static int	join_token(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next)
	{
		if (token->type == WORD && token->next && token->next->type == WORD
			&& token->space_after == 0 && token->next->space_before == 0
			&& token->double_quote == 0 && token->single_quote == 0
			&& (token->next->double_quote || token->next->single_quote))
		{
			str = ft_strjoin(token->value, token->next->value);
			if (!str)
				return (1);
			free(token->value);
			token->value = str;
			token->quoted_outside = 1;
			remove_useless_token(tokens, token->next);
		}
		else
			token = token->next;
	}
	return (0);
}

/**
 * join_raw_words - Joins adjacent unquoted words into a single token
 * @tokens: Pointer to the head of the linked list of tokens
 *
 * Iterates through the tokens and joins adjacent WORD tokens that are not
 * separated by spaces or quotes into a single WORD token.
 * Returns 0 on success, 1 if memory allocation fails.
 */

static int	join_raw_words(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (!token->space_after && !token->next->space_before
			&& !token->double_quote && !token->single_quote
			&& !token->next->double_quote && !token->next->single_quote
			&& token->type == WORD && token->next->type == WORD)
		{
			str = ft_strjoin(token->value, token->next->value);
			if (!str)
				return (1);
			free(token->value);
			token->value = str;
			remove_useless_token(tokens, token->next);
		}
		else
			token = token->next;
	}
	return (0);
}

/**
 * join_empty_token - Joins empty tokens with adjacent WORD tokens
 * @tokens: Pointer to the head of the linked list of tokens
 *
 * Iterates through the tokens and joins empty WORD tokens (those with an empty
 * value) with adjacent WORD tokens that are not separated by spaces or quotes.
 * Returns 0 on success, 1 if memory allocation fails.
 */

static int	join_empty_token(t_token **tokens)
{
	t_token	*token;
	char	*joined;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (token->type == WORD && token->next && token->next->type == WORD
			&& token->value && token->value[0] == '\0'
			&& !token->next->space_before && !token->space_after
			&& (token->double_quote || token->single_quote))
		{
			joined = ft_strjoin(token->value, token->next->value);
			if (!joined)
				return (1);
			free(token->value);
			token->value = joined;
			remove_useless_token(tokens, token->next);
		}
		else
			token = token->next;
	}
	return (0);
}

/**
 *process_and_join_tokens - Processes and joins tokens in the shell data
 * @data: Pointer to the shell data structure containing tokens
 * * This function performs various operations on the tokens, including
 * expanding them, removing unnecessary dollar signs,
 * cleaning empty tokens,
 * joining adjacent tokens,
 * joining raw words,
 * joining empty tokens,
 * joining quoted strings, and joining tokens without spaces.
 * * Returns 0 on success, or 1 if any operation fails.
 */

int	process_and_join_tokens(t_shell *data)
{
	t_token	**token;

	token = &(data->tokens);
	if (expand_token(data))
		return (1);
	remove_useless_dollars(token);
	clean_empty_tokens(token);
	if (join_token(token))
		return (1);
	if (join_raw_words(token))
		return (1);
	if (join_empty_token(token))
		return (1);
	if (join_quoted_str(token))
		return (1);
	if (join_no_space(token))
		return (1);
	return (0);
}
