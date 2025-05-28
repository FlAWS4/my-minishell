/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 20:46:11 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle pipe token
 */
static int	create_pipe_token(t_token **tokens, int preceded_by_space)
{
    char	*value;

    value = ft_strdup("|");
    if (!value)
        return (-1);
    add_token(tokens, create_token(TOKEN_PIPE, value, preceded_by_space));
    return (0);
}

/**
 * Handle input redirection tokens
 */
static int	handle_input_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;

    if (input[i + 1] && input[i + 1] == '<')
    {
        value = ft_strdup("<<");
        if (!value)
            return (-1);
        add_token(tokens, create_token(TOKEN_HEREDOC, value, preceded_by_space));
        return (i + 2);  // Skip both '<' characters
    }
    value = ft_strdup("<");
    if (!value)
        return (-1);
    add_token(tokens, create_token(TOKEN_REDIR_IN, value, preceded_by_space));
    return (i + 1);  // Skip the '<' character
}

/**
 * Handle output redirection tokens
 */
static int	handle_output_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;

    if (input[i + 1] && input[i + 1] == '>')
    {
        value = ft_strdup(">>");
        if (!value)
            return (-1);
        add_token(tokens, create_token(TOKEN_REDIR_APPEND, value, preceded_by_space));
        return (i + 2);  // Skip both '>' characters
    }
    value = ft_strdup(">");
    if (!value)
        return (-1);
    add_token(tokens, create_token(TOKEN_REDIR_OUT, value, preceded_by_space));
    return (i + 1);  // Skip the '>' character
}

/**
 * Handle special characters (pipes, redirections)
 */
int	handle_special(char *input, int i, t_token **tokens)
{
    int	preceded_by_space;
    int	result;

    preceded_by_space = 0;
    if (i > 0 && is_whitespace(input[i - 1]))
        preceded_by_space = 1;
    
    if (input[i] == '|')
    {
        if (create_pipe_token(tokens, preceded_by_space) == -1)
            return (-1);
        return (i + 1);  // Return index of next character
    }
    else if (input[i] == '<')
        result = handle_input_redir(input, i, tokens, preceded_by_space);
    else if (input[i] == '>')
        result = handle_output_redir(input, i, tokens, preceded_by_space);
    else
        result = i + 1;  // Unexpected character, skip it
    
    return (result);
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
            (!next || (next->type != TOKEN_WORD && 
                      next->type != TOKEN_SINGLE_QUOTE && 
                      next->type != TOKEN_DOUBLE_QUOTE)))
            return (0);
            
        current = next;
    }
    return (1);
}

