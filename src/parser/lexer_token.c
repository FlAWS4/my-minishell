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

static int	join_token_bis(t_token **tokens)
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

int	prepare_token_str(t_shell *data)
{
	t_token	**token;

	token = &(data->tokens);
	if (expand_token(data))
		return (1);
	remove_useless_dollars(token);
	clean_empty_tokens(token);
	if (join_token(token))
		return (1);
	if (join_token_bis(token))
		return (1);
	if (join_empty_token(token))
		return (1);
	if (join_quoted_str(token))
		return (1);
	if (join_no_space(token))
		return (1);
	return (0);
}

