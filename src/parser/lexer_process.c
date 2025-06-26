/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_process.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:40:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:41:31 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static int	is_quoted(t_token *token)
{
	if (token->double_quote || token->single_quote)
		return (1);
	return (0);
}

static void	unsupported_symbols(char **unsupported_symbols)
{
	int	i;

	i = 0;
	unsupported_symbols[i++] = "&&";
	unsupported_symbols[i++] = "||";
	unsupported_symbols[i++] = "!";
	unsupported_symbols[i++] = ";";
	unsupported_symbols[i++] = "&";
	unsupported_symbols[i++] = "(";
	unsupported_symbols[i++] = ")";
	unsupported_symbols[i++] = "[";
	unsupported_symbols[i++] = "]";
	unsupported_symbols[i++] = "[[";
	unsupported_symbols[i++] = "]]";
	unsupported_symbols[i++] = "((";
	unsupported_symbols[i++] = "))";
	unsupported_symbols[i++] = "*";
	unsupported_symbols[i++] = "\\";
	unsupported_symbols[i++] = "~";
	unsupported_symbols[i++] = "#";
	unsupported_symbols[i++] = "$(";
	unsupported_symbols[i++] = "$[";
}

static int	symbols_error(t_token **tokens, char *symbol)
{
	t_token	*token;
	char	*error;
	char	*str;

	token = create_token(ERROR, NULL);
	if (!token)
		return (-1);
	str = ft_strjoin(ERROR_UNSUPPORTED, symbol);
	if (!str)
		return (free(token), -1);
	error = ft_strjoin(str, "\n");
	free(str);
	if (!error)
		return (free(token), -1);
	free(token->value);
	token->value = error;
	add_token(tokens, token);
	return (0);
}

int	check_unsupported_character(t_token **tokens)
{
	t_token	*token;
	char	*symbols[19];
	int		i;

	unsupported_symbols(symbols);
	token = *tokens;
	while (token)
	{
		if (token->type == WORD && token->value && !is_quoted(token))
		{
			i = 0;
			while (i < 19)
			{
				if (ft_strstr(token->value, symbols[i]))
				{
					if (symbols_error(tokens, symbols[i]) == -1)
						return (1);
					return (check_token_error(tokens));
				}
				i++;
			}
		}
		token = token->next;
	}
	return (0);
}
