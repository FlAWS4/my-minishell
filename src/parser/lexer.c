/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/12 23:02:50 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int    is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

t_token *create_token(t_token_type type, char *value)
{
	t_token *token;

	token = malloc(sizeof(t_token));
	if (!token)
		return (NULL);
	token->type = type;
	token->value = value;
	token->next = NULL;
	return (token);
}

void    add_token(t_token **token_list, t_token *new_token)
{
	t_token *current;

	if (!*token_list)
	{
		*token_list = new_token;
		return ;
	}
	current = *token_list;
	while (current->next)
		current = current->next;
	current->next = new_token;
}
