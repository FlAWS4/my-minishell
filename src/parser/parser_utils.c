/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:27:49 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:29:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * return_token_type - Returns the string representation of a token type
 * @t_type: Pointer to the token whose type is to be returned
 *
 * This function checks the type of the token and returns a string
 * representation of it. If the token is an operator, it returns
 * the corresponding operator string.
 * Returns NULL if the token type does not match any known operator.
 */

static char	*return_token_type(t_token *t_type)
{
	if (t_type->next && is_token_operator(t_type->type)
		&& t_type->next->type == T_EOF && t_type->previous
		&& !is_token_operator(t_type->previous->type))
		return (" 'newline'\n");
	if (t_type->type == APPEND)
		return (" '>>'\n");
	else if (t_type->type == HEREDOC)
		return (" '<<'\n");
	else if (t_type->type == PIPE)
		return (" '|'\n");
	else if (t_type->type == REDIR_IN)
		return (" '<'\n");
	else if (t_type->type == REDIR_OUT)
		return (" '>'\n");
	return (NULL);
}

/**
 * add_token_error - Adds a syntax error token to the list of tokens
 * @tokens: Pointer to the list of tokens
 * @error: The token that caused the error
 * @str: The error message string
 * This function creates a new token of type
 *  ERROR with the provided error message.
 * It appends the token type to the error 
 * message if it is not a newline syntax error.
 * Returns 0 on success, -1 on failure (memory allocation error).
 */

int	add_token_error(t_token **tokens, t_token *error, char *str)
{
	char	*token_type;
	t_token	*error_token;

	token_type = return_token_type(error);
	if (!token_type)
		return (-1);
	error_token = create_token(ERROR, NULL);
	if (!error_token)
		return (-1);
	if (ft_strcmp(str, ERROR_SYNTAX_NL))
		error_token->value = ft_strjoin(str, token_type);
	else
		error_token->value = ft_strdup(str);
	if (!error_token->value)
	{
		free(error_token);
		return (-1);
	}
	add_token(tokens, error_token);
	return (0);
}

/**
 * is_token_operator - Checks if the token type is an operator
 * @token_type: The type of the token to check
 *
 * This function checks if the given token type is one of the operator types.
 * Returns 1 if it is an operator, 0 otherwise.
 */

int	is_token_operator(t_token_type token_type)
{
	if (token_type == REDIR_IN
		|| token_type == APPEND || token_type == HEREDOC
		|| token_type == REDIR_OUT)
		return (1);
	return (0);
}

/**
 * is_operator_follow - Checks if the current token is followed by an operator
 * @tokens: Pointer to the list of tokens
 * @current_token: The current token to check
 *
 * This function checks if the current token is an operator and if it is followed
 * by another operator. If so, it adds a syntax error to the token list.
 * Returns 1 if an error is added, 0 otherwise.
 */

int	is_operator_follow(t_token **tokens, t_token *current_token)
{
	if (current_token->next)
	{
		if (is_token_operator(current_token->type)
			&& is_token_operator(current_token->next->type))
		{
			if (add_token_error(tokens, current_token->next, ERROR_SYNTAX)
				== -1)
				free_tokens_list(tokens);
			return (1);
		}
		else if (current_token->next)
		{
			if (current_token->type == PIPE
				&& current_token->next->type == PIPE)
			{
				if (add_token_error(tokens, current_token->next, ERROR_SYNTAX)
					== -1)
					free_tokens_list(tokens);
				return (1);
			}
		}
	}
	return (0);
}

/**
 * check_token_error - Checks for syntax errors in the token list
 * @tokens: Pointer to the list of tokens
 *
 * This function iterates through the tokens and checks for syntax errors.
 * If a PIPE token is found at the beginning or if an ERROR token is found,
 * it prints the corresponding error message and sets the global exit status.
 * Returns 1 if an error is found, 0 otherwise.
 */

int	check_token_error(t_token **tokens)
{
	t_token	*token;

	token = *tokens;
	if (token->type == PIPE)
	{
		ft_putstr_fd(ERROR_SYNTAX_PIPE, 2);
		return (1);
	}
	while (token)
	{
		if (token->type == ERROR)
		{
			ft_putstr_fd(token->value, 2);
			g_exit_status = 2;
			return (1);
		}
		token = token->next;
	}
	return (0);
}
