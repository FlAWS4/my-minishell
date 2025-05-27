/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 01:38:00 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle special characters (pipes, redirections)
 */
int	handle_special(char *input, int i, t_token **tokens)
{
    if (input[i] == '|')
        add_token(tokens, create_token(TOKEN_PIPE, ft_strdup("|")));
    else if (input[i] == '<')
    {
        if (input[i + 1] && input[i + 1] == '<')
        {
            add_token(tokens, create_token(TOKEN_HEREDOC, ft_strdup("<<")));
            i++;
        }
        else
            add_token(tokens, create_token(TOKEN_REDIR_IN, ft_strdup("<")));
    }
    else if (input[i] == '>')
    {
        if (input[i + 1] && input[i + 1] == '>')
        {
            add_token(tokens, create_token(TOKEN_REDIR_APPEND, ft_strdup(">>")));
            i++;
        }
        else
            add_token(tokens, create_token(TOKEN_REDIR_OUT, ft_strdup(">")));
    }
    return (i);
}

/**
 * Handle a word token
 */
int	handle_word(char *input, int i, t_token **tokens)
{
    int		start;
    char	*word;
    
    start = i;
    // Only include characters until whitespace or special character
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
        i++;
    
    word = ft_substr(input, start, i - start);
    if (!word)
        return (-1);
        
    add_token(tokens, create_token(TOKEN_WORD, word));
    return (i);
}

/**
 * Validate token syntax
 */
int	validate_syntax(t_token *tokens)
{
    t_token	*current;
    t_token	*next;

    current = tokens;
    while (current)
    {
        next = current->next;
        if (current->type == TOKEN_PIPE && 
            (!next || current == tokens))
            return (0);
        
        if ((current->type == TOKEN_REDIR_IN || 
            current->type == TOKEN_REDIR_OUT ||
            current->type == TOKEN_REDIR_APPEND ||
            current->type == TOKEN_HEREDOC) && 
            (!next || next->type != TOKEN_WORD))
            return (0);
            
        current = next;
    }
    return (1);
}
