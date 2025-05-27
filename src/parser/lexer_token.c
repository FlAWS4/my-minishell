/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_token.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:39:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 01:48:06 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks if a character is a special token character
 */
int	is_special(char c)
{
    return (c == '|' || c == '<' || c == '>');
}

/**
 * Create a new token
 */
t_token	*create_token(t_token_type type, char *value)
{
    t_token	*new_token;

    new_token = (t_token *)malloc(sizeof(t_token));
    if (!new_token)
        return (NULL);
    new_token->type = type;
    new_token->value = value;
    new_token->next = NULL;
    return (new_token);
}

/**
 * Add a token to the token list
 */
void	add_token(t_token **list, t_token *new)
{
    t_token	*temp;

    if (!new)
        return ;
    if (!*list)
    {
        *list = new;
        return ;
    }
    temp = *list;
    while (temp->next)
        temp = temp->next;
    temp->next = new;
}

int	handle_quote(char *input, int i, t_token **tokens)
{
    char	quote;
    int		start;
    char	*word;

    quote = input[i];
    start = i + 1;  // Skip opening quote
    i++;
    
    while (input[i] && input[i] != quote)
        i++;
        
    if (!input[i])  // Unclosed quote
        return (-1);
        
    // Extract content without quotes
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
        
    add_token(tokens, create_token(TOKEN_WORD, word));
    return (i + 1);  // Position after closing quote
}
