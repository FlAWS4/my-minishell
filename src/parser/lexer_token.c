/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_token.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:39:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:21:20 by mshariar         ###   ########.fr       */
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
t_token	*create_token(t_token_type type, char *value, int preceded_by_space)
{
    t_token	*new_token;

    new_token = (t_token *)malloc(sizeof(t_token));
    if (!new_token)
    {
        free(value);
        return (NULL);
    }
    new_token->type = type;
    new_token->value = value;
    new_token->preceded_by_space = preceded_by_space;
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

/**
 * Handle quoted strings in input
 */
int	handle_quote(char *input, int i, t_token **tokens)
{
    char	quote;
    int		start;
    char	*word;
    int		preceded_by_space;
    
    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
    quote = input[i];
    start = i + 1;
    i++;
    while (input[i] && input[i] != quote)
        i++;
    if (!input[i])
        return (-1);
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
    if (quote == '\'')
        add_token(tokens, create_token(TOKEN_SINGLE_QUOTE, word, preceded_by_space));
    else
        add_token(tokens, create_token(TOKEN_DOUBLE_QUOTE, word, preceded_by_space));
    return (i + 1);
}

/**
 * Handle word token in input
 */
int	handle_word(char *input, int i, t_token **tokens)
{
    int		start;
    char	*word;
    int		preceded_by_space;
    
    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
    start = i;
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
        i++;
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
    add_token(tokens, create_token(TOKEN_WORD, word, preceded_by_space));
    return (i);  // Return current position - process_token_part doesn't increment
}