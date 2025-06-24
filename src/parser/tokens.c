/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/24 00:47:01 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

int	is_operator(char *str, int i)
{
	if (str[i] == '<' && str[i + 1] && str[i + 1] == '<')
		return (HEREDOC);
	else if (str[i] == '>' && str[i + 1] && str[i + 1] == '>')
		return (APPEND);
	else if (str[i] == '|')
		return (PIPE);
	else if (str[i] == '>')
		return (REDIR_OUT);
	else if (str[i] == '<')
		return (REDIR_IN);
	return (-1);
}

void	add_token(t_token **head, t_token *new_token)
{
	t_token	*current;

	if (!*head)
	{
		*head = new_token;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_token;
	new_token->previous = current;
	new_token->next = NULL;
}

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

t_token	*create_token(t_token_type type, char *value)
{
	t_token	*token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	ft_memset(token, 0, sizeof(t_token));
	token->type = type;
	if (value)
	{
		token->value = ft_strdup(value);
		if (!token->value)
		{
			free(token);
			return (NULL);
		}
	}
	token->previous = NULL;
	token->next = NULL;
	return (token);
}


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
