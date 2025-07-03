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

/**
 * is_token_operator - Checks if the token type is an operator
 * @type: The type of the token to check
 *
 * Returns 1 if the token type is an operator, 0 otherwise.
 */

int	is_cmd(t_token *tokens)
{
	if (!tokens->previous)
		return (1);
	else if (!is_token_operator(tokens->previous->type))
		return (1);
	return (0);
}

/**
 * fill_words - Fills the args array with the token value
 * @tokens: The token to fill
 * @args: Pointer to the args array to fill
 *
 * Allocates memory for the args array and copies the token value into it.
 * Returns 0 on success, 1 on failure.
 */

int	fill_words(t_token *tokens, char **args)
{
	*args = ft_strdup(tokens->value);
	if (!*args)
		return (1);
	return (0);
}

/**
 * is_operator - Checks if the character at index i is an operator
 * @str: The string to check
 * @i: The index to check
 *
 * Returns the token type if it is an operator, -1 otherwise.
 */

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

/**
 * add_token - Adds a new token to the end of the linked list of tokens
 * @head: Pointer to the head of the token list
 * @new_token: The new token to add
 *
 * If the list is empty, sets the head to the new token.
 * Otherwise, traverses to the end of the list and adds the new token.
 */

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

/**
 * create_token - Creates a new token with the specified type and value
 * @type: The type of the token to create
 * @value: The value of the token to create
 *
 * Allocates memory for a new token, initializes its fields, and returns it.
 * Returns NULL if memory allocation fails.
 */

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
