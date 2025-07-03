/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_expand.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:53:16 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:54:42 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * expand_exit_status - Expands the exit status variable
 * @result: Pointer to the result string where the expanded value will be added
 * @var: The variable string containing the exit status or custom text
 * If the variable starts with "$?", 
 * it converts the global exit status to a string
 * and adds it to the result. 
 * If it starts with "$$" or any other text, it adds that text.
 * Returns 0 on success, or -1 on memory allocation failure.
 */

int	expand_exit_status(char **result, char *var)
{
	int		res;
	char	*status;

	if (var[1] == '?')
	{
		status = ft_itoa(g_exit_status);
		if (!status)
			return (-1);
		res = add_text(result, status);
		free(status);
	}
	else
		res = add_text(result, var + 2);
	return (res);
}

/**
 * expand_variable - Expands a variable in the input string
 * @data: Pointer to the shell data structure
 * @input: The input string containing the variable to expand
 * @str: Pointer to the string where the expanded value will be stored
 * This function checks if the input string contains a variable that needs to be
 * expanded and performs the expansion.
 * Returns 0 on success, or -1 on memory allocation failure.
 */

int	join_quoted_str(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (token->type == WORD && token->next && token->next->type == WORD
			&& (token->double_quote || token->single_quote)
			&& (token->next->double_quote || token->next->single_quote)
			&& !token->space_after && !token->next->space_before)
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
 * join_quoted_str - Joins adjacent quoted strings in the token list
 * @tokens: Pointer to the head of the linked list of tokens
 *
 * This function iterates through the tokens and joins adjacent WORD tokens
 * that are both quoted (single or double quotes) and not separated by spaces.
 * Returns 0 on success, 1 if memory allocation fails.
 */

int	join_no_space(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (token->type == WORD && token->next->type == WORD
			&& !token->space_after && !token->next->space_before)
		{
			str = ft_strjoin(token->value, token->next->value);
			if (!str)
				return (1);
			free(token->value);
			token->value = str;
			if (token->next->single_quote || token->next->double_quote)
				token->quoted_outside = 1;
			remove_useless_token(tokens, token->next);
			continue ;
		}
		token = token->next;
	}
	return (0);
}

/**
 * check_token_value - Checks and expands the value of a token
 * @data: Pointer to the shell data structure
 * @tokens: Pointer to the token to be checked and expanded
 *
 * This function performs expansion on the token's value.
 * If the expansion is successful, it returns 0.
 * If an error occurs during expansion, it returns a negative value.
 * If the token's value is empty after expansion, it removes the token.
 * Returns 0 on success, or a negative value on failure.
 */

static int	check_token_value(t_shell *data, t_token **tokens)
{
	int	result;

	result = expantion(data, *tokens);
	if (result != 0)
		return (result);
	return (0);
}

/**
 * expand_token - Expands all tokens in the shell data structure
 * @data: Pointer to the shell data structure containing tokens
 *
 * This function iterates through the tokens in the shell data structure
 * and expands each token's value. If an error occurs during expansion,
 * it returns 1. If a token's value is empty after expansion, it removes
 * that token from the list.
 * Returns 0 on success, or 1 if an error occurs.
 */

int	expand_token(t_shell *data)
{
	t_token	*token;
	t_token	*next_token;
	int		result;

	token = data->tokens;
	while (token && token->type != T_EOF)
	{
		next_token = token->next;
		result = check_token_value(data, &token);
		if (result < 0)
			return (1);
		if (result > 0)
		{
			while (token->next && token->next != next_token)
				token = token->next;
		}
		token = next_token;
	}
	return (0);
}
