/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:26:14 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	handle_operator(char *input, int *i, t_token **tokens)
{
	t_token_type	operator;

	operator = is_operator(input, *i);
	if (operator == HEREDOC || operator == APPEND)
		return (handle_double_operator(i, tokens, operator));
	else if ((operator == PIPE || operator == REDIR_OUT
			|| operator == REDIR_IN))
		return (handle_single_operator(i, tokens, operator));
	return (*i);
}

static int	handle_quote(char *input, int *i, t_token **tokens)
{
	t_token	*token;
	char	quote;
	int		start_quote;

	quote = input[(*i)];
	start_quote = ++(*i);
	while (input[(*i)] && input[(*i)] != quote)
		(*i)++;
	if (input[(*i)] == '\0')
	{
		token = create_token(ERROR, ERROR_QUOTES);
		if (!token)
			return (-1);
		add_token(tokens, token);
		return (*i);
	}
	return (handle_in_quote(start_quote, input, i, tokens));
}

static int	handle_word(char *input, int *i, t_token **tokens)
{
	t_token	*token;
	int		start_word;
	int		space_before;

	space_before = 0;
	if (*i > 0 && is_whitespace(input[*i - 1]))
		space_before = 1;
	start_word = (*i);
	while (input[(*i)] && !is_whitespace(input[(*i)])
		&& is_operator(input, *i) == -1 && !is_quote(input[(*i)]))
		(*i)++;
	token = create_token(WORD, NULL);
	if (!token)
		return (-1);
	token->value = ft_substr(input, start_word, *i - start_word);
	if (!token->value)
		return (free(token), -1);
	if (input[*i] && is_whitespace(input[*i]))
		token->space_after = 1;
	if (space_before)
		token->space_before = 1;
	return (add_token(tokens, token), *i);
}

static int	handle_string(char *input, int *i, t_token **tokens)
{
	if (is_quote(input[*i]))
	{
		*i = handle_quote(input, i, tokens);
		if (*i == -1)
			return (-1);
	}
	else if (is_operator(input, *i) != -1)
	{
		*i = handle_operator(input, i, tokens);
		if (*i == -1)
			return (-1);
	}
	else
	{
		*i = handle_word(input, i, tokens);
		if (*i == -1)
			return (-1);
	}
	return (*i);
}

t_token	*tokenize_input(char *input)
{
	t_token	*tokens;
	t_token	*eof_token;
	int		i;

	if (!input)
		return (NULL);
	tokens = NULL;
	i = 0;
	while (input[i] != '\0')
	{
		while (is_whitespace(input[i]))
			i++;
		if (input[i] == '\0')
			break ;
		else
		{
			if (handle_string(input, &i, &tokens) == -1)
				return (free_tokens_list(&tokens), NULL);
		}
	}
	eof_token = create_token(T_EOF, NULL);
	if (!eof_token)
		return (free_tokens_list(&tokens), NULL);
	return (add_token(&tokens, eof_token), free(input), tokens);
}
