/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_token.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:21:47 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:25:26 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_cmd(t_token *tokens)
{
	if (!tokens->previous)
		return (1);
	else if (!is_token_operator(tokens->previous->type))
		return (1);
	return (0);
}

int	fill_words(t_token *tokens, char **args)
{
	*args = ft_strdup(tokens->value);
	if (!*args)
		return (1);
	return (0);
}
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