/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_process.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:40:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 02:06:09 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process tokens from input string
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    int     last_was_word;
    
    tokens = NULL;
    i = 0;
    last_was_word = 0;
    
    while (input[i])
    {
        if (is_whitespace(input[i]))
        {
            // Mark end of a word/command
            last_was_word = 0;
            i++;
        }
        else if (input[i] == '\'' || input[i] == '\"')
        {
            i = handle_quote(input, i, &tokens);
            last_was_word = 1;
        }
        else if (is_special(input[i]))
        {
            i = handle_special(input, i, &tokens) + 1;
            last_was_word = 0;
        }
        else
        {
            i = handle_word(input, i, &tokens);
            last_was_word = 1;
        }
            
        if (i == -1)
        {
            free_token_list(tokens);
            return (NULL);
        }
    }
    return (tokens);
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
        return (NULL);
    }
    
    return (tokens);
}
