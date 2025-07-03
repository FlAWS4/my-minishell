/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_tokens.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:25:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:26:26 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_whitespace(char c)
{
	if (c == ' ' || (c >= 9 && c <= 13))
		return (1);
	return (0);
}

int	is_quote(char c)
{
	if (c == '\'')
		return (1);
	else if (c == '"')
		return (2);
	return (0);
}

/**
 * handle_in_quote - Handles the case when a quote is found in the input
 * @start_quote: The index where the quote started
 * @input: The input string being parsed
 * @i: Pointer to the current index in the input string
 * @tokens: Pointer to the list of tokens being created
 *
 * This function creates a new token for the quoted word 
 * and adds it to the tokens list.
 * It also handles whether the quote is single or double 
 * and sets appropriate flags.
 * Returns the updated index after processing the quote.
 */

int	handle_in_quote(int start_quote, char *input, int *i, t_token **tokens)
{
	t_token	*token;

	token = create_token(WORD, NULL);
	if (!token)
		return (-1);
	if (is_quote(input[*i]) == 1)
		token->single_quote = 1;
	else if (is_quote(input[*i]) == 2)
		token->double_quote = 2;
	add_token(tokens, token);
	token->value = ft_substr(input, start_quote, *i - start_quote);
	if (!token->value)
		return (-1);
	(*i)++;
	if (start_quote >= 2 && input[start_quote - 2]
		&& is_whitespace(input[start_quote - 2]))
		token->space_before = 1;
	if (input[*i] && is_whitespace(input[*i]))
		token->space_after = 1;
	return (*i);
}

/**
 * handle_in_word - Handles the case when a word is found in the input
 * @start: The index where the word started
 * @input: The input string being parsed
 * @i: Pointer to the current index in the input string
 * @tokens: Pointer to the list of tokens being created
 * This function creates a new token for the word and adds it to the tokens list.
 * It also sets appropriate flags for spaces before and after the word.
 * Returns the updated index after processing the word.
 */

int	handle_double_operator(int *i, t_token **tokens,
	t_token_type operator)
{
	t_token	*token;

	token = NULL;
	if (operator == APPEND)
		token = create_token(APPEND, NULL);
	else if (operator == HEREDOC)
		token = create_token(HEREDOC, NULL);
	if (!token)
		return (-1);
	add_token(tokens, token);
	(*i) += 2;
	return (*i);
}

/**
 * handle_single_operator - Handles the case when a single operator is found
 * @i: Pointer to the current index in the input string
 * @tokens: Pointer to the list of tokens being created
 * @operator: The type of operator found (PIPE, REDIR_IN, REDIR_OUT)
 * This function creates a new token for the 
 * operator and adds it to the tokens list.
 * Returns the updated index after processing the operator.
 */

int	handle_single_operator(int *i, t_token **tokens,
	t_token_type operator)
{
	t_token	*token;

	token = NULL;
	if (operator == PIPE)
		token = create_token(PIPE, NULL);
	else if (operator == REDIR_IN)
		token = create_token(REDIR_IN, NULL);
	else
		token = create_token(REDIR_OUT, NULL);
	if (!token)
		return (-1);
	add_token(tokens, token);
	(*i)++;
	return (*i);
}
