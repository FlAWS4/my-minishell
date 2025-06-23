/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 03:02:20 by my42             ###   ########.fr       */
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
	str = ft_strjoin(UNSUPPORT, symbol);
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
	if (ft_strcmp(str, SYNTAXNL))
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
			if (add_token_error(tokens, current_token->next, SYNTAXER)
				== -1)
				free_tokens_list(tokens);
			return (1);
		}
		else if (current_token->next)
		{
			if (current_token->type == PIPE
				&& current_token->next->type == PIPE)
			{
				if (add_token_error(tokens, current_token->next, SYNTAXER)
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
		ft_putstr_fd(SYNTAXPI, 2);
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
				add_token_error(tokens, token, SYNTAXER);
				return (check_token_error(tokens));
			}
			else if (is_token_operator(token->type)
				&& token->next->type == PIPE)
			{
				add_token_error(tokens, token->next, SYNTAXER);
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
				if (add_token_error(tokens, token, SYNTAXNL) == -1)
					free_tokens_list(tokens);
			}
			else if (token->type == PIPE && token->next->type
				== PIPE)
			{
				if (add_token_error(tokens, token, SYNTAXER) == -1)
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
		return (ft_putstr_fd(TOKENFAIL, 2), 1);
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
