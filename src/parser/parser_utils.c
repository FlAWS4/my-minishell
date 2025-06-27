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
	if (ft_strcmp(str, ERROR_SYNTAX_NL ))
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

int	is_token_operator(t_token_type token_type)
{
	if (token_type == REDIR_IN
		|| token_type == APPEND || token_type == HEREDOC
		|| token_type == REDIR_OUT)
		return (1);
	return (0);
}

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
