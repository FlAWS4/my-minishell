/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_process.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:40:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 03:11:17 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process a token at the current position
 * All handler functions return the index of the next character to process
 * or -1 on error
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
    {
        i = handle_special(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    else
    {
        i = handle_word(input, i, tokens);
        if (i == -1)
            return (-1);
    }
    
    // Update preceded_by_space flag
    last = get_last_token(*tokens);
    if (last)
        last->preceded_by_space = *was_space;
    *was_space = 0;
    
    return (i);
}

/**
 * Process tokens from input string
 * Returns the list of tokens or NULL on error
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    int		was_space;

    tokens = NULL;
    i = 0;
    was_space = 1;  // Assume leading space for the first token
    
    while (input[i])
    {
        i = process_token_part(input, i, &tokens, &was_space);
        if (i == -1)
        {
            free_token_list(tokens);
            ft_putstr_fd("minishell: syntax error: unclosed quote\n", 2);
            return (NULL);
        }
    }
    
    return (tokens);
}

/**
 * Get the last token in a list
 * Returns NULL if the list is empty
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
 * This is the main entry point for the lexer
 */
t_token	*tokenize(char *input)
{
    t_token	*tokens;
    
    if (!input)
        return (NULL);
        
    // Generate raw tokens
    tokens = process_tokens(input);
    if (!tokens)
        return (NULL);
    
    // Post-process tokens: merge adjacent quoted segments
    merge_adjacent_quoted_tokens(&tokens);
        
    // Validate syntax
    if (!validate_syntax(tokens))
    {
        free_token_list(tokens);
        ft_putstr_fd("minishell: syntax error near unexpected token\n", 2);
        return (NULL);
    }
    
    return (tokens);
}

#ifdef DEBUG
/**
 * Print token list for debugging (only compiled in DEBUG mode)
 */
void print_tokens(t_token *tokens)
{
    t_token *current = tokens;
    int i = 0;
    
    ft_putstr_fd("Token list:\n", 2);
    while (current)
    {
        ft_putstr_fd("  ", 2);
        ft_putnbr_fd(i++, 2);
        ft_putstr_fd(": type=", 2);
        ft_putnbr_fd(current->type, 2);
        ft_putstr_fd(" value='", 2);
        ft_putstr_fd(current->value, 2);
        ft_putstr_fd("' space=", 2);
        ft_putnbr_fd(current->preceded_by_space, 2);
        ft_putstr_fd("\n", 2);
        current = current->next;
    }
}
#endif