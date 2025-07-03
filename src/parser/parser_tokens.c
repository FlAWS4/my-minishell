/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:29:35 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * check_unsupported_character - Checks for unsupported characters in tokens
 * @tokens: Pointer to the list of tokens
 *
 * This function iterates through the tokens and checks 
 * for unsupported characters.
 * If an unsupported character is found,
 *  it adds a syntax error to the token list.
 * Returns 0 if no errors are found, 1 if an error is added.
 */

static int	check_inside_pipe(t_token **tokens)
{
	t_token	*token;

	token = *tokens;
	while (token)
	{
		if (token->next)
		{
			if (token->type == PIPE && is_token_operator(token->next->type)
				&& token->next->next
				&& token->next->next->type == PIPE)
			{
				add_token_error(tokens, token, ERROR_SYNTAX);
				return (check_token_error(tokens));
			}
			else if (is_token_operator(token->type)
				&& token->next->type == PIPE)
			{
				add_token_error(tokens, token->next, ERROR_SYNTAX);
				return (check_token_error(tokens));
			}
		}
		token = token->next;
	}
	return (check_token_error(tokens));
}

/**
 * check_operator - Checks for syntax errors related to operators
 * @tokens: Pointer to the list of tokens
 *
 * This function checks if operators are followed by valid tokens.
 * If an operator is followed by an invalid token or if there are
 * consecutive operators, it adds a syntax error to the token list.
 */

static void	check_operator(t_token **tokens)
{
	t_token	*token;

	token = *tokens;
	while (token)
	{
		if (token->next)
		{
			if (is_token_operator(token->type)
				&& is_operator_follow(tokens, token))
				return ;
			else if (is_token_operator(token->type)
				&& token->next->type == T_EOF)
			{
				if (add_token_error (tokens, token, ERROR_SYNTAX_NL) == -1)
					free_tokens_list(tokens);
			}
			else if (token->type == PIPE && token->next->type
				== PIPE)
			{
				if (add_token_error(tokens, token, ERROR_SYNTAX) == -1)
					free_tokens_list(tokens);
			}
		}
		token = token->next;
	}
}

/**
 * syntax_check - Performs syntax checks on the command structure
 * @cmd: Pointer to the shell command structure
 *
 * This function checks for various syntax errors in the command tokens.
 * It checks for token errors, operator errors, unsupported characters,
 * and splits commands with pipes.
 * Returns 0 if no errors are found, 1 if an error is detected.
 */

int	syntax_check(t_shell *cmd)
{
	cmd->heredoc_interupt = 0;
	if (!cmd->tokens)
		return (ft_putstr_fd(ERROR_TOKENIZE, 2), 1);
	if (check_token_error(&cmd->tokens))
		return (free_tokens_list(&cmd->tokens), 1);
	check_operator(&cmd->tokens);
	if (check_token_error(&cmd->tokens))
		return (free_tokens_list(&cmd->tokens), 1);
	if (check_inside_pipe(&cmd->tokens))
		return (free_tokens_list(&cmd->tokens), 1);
	if (check_unsupported_character(&cmd->tokens))
		return (free_tokens_list(&cmd->tokens), 1);
	if (split_cmd_with_pipe(cmd))
		return (free_tokens_list(&cmd->tokens), 1);
	return (0);
}
