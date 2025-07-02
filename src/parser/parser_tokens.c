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
