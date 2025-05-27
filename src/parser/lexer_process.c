/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_process.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:40:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:19:36 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process a token at the current position
 * All handler functions return the index of the next character to process
 */
static int	process_token_part(char *input, int i, t_token **tokens, int *was_space)
{
    t_token	*last;

    if (is_whitespace(input[i]))
    {
        *was_space = 1;
        return (i + 1);
    }
    else if (input[i] == '\'' || input[i] == '\"')
    {
        i = handle_quote(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    else if (is_special(input[i]))
        i = handle_special(input, i, tokens);
    else
        i = handle_word(input, i, tokens);
    if (i == -1)
        return (-1);
    last = get_last_token(*tokens);
    if (last)
        last->preceded_by_space = *was_space;
    *was_space = 0;
    return (i);
}

/**
 * Process tokens from input string
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    int		was_space;

    tokens = NULL;
    i = 0;
    was_space = 1;
    while (input[i])
    {
        i = process_token_part(input, i, &tokens, &was_space);
        if (i == -1)
        {
            free_token_list(tokens);
            ft_putstr_fd("minishell: syntax error\n", 2);
            return (NULL);
        }
    }
    return (tokens);
}

/**
 * Get the last token in a list
 */
t_token	*get_last_token(t_token *tokens)
{
    t_token	*last;

    if (!tokens)
        return (NULL);
    last = tokens;
    while (last->next)
        last = last->next;
    return (last);
}

/**
 * Tokenize the input string
 */
t_token	*tokenize(char *input)
{
    t_token	*tokens;
    
    if (!input)
        return (NULL);
        
    tokens = process_tokens(input);
    if (!tokens)
        return (NULL);
        
    if (!validate_syntax(tokens))
    {
        free_token_list(tokens);
        ft_putstr_fd("minishell: syntax error near unexpected token\n", 2);
        return (NULL);
    }
    
    return (tokens);
}
