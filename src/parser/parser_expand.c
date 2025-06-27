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

static int	check_token_value(t_shell *data, t_token **tokens)
{
	int	result;

	result = expantion(data, *tokens);
	if (result != 0)
		return (result);
	return (0);
}

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
