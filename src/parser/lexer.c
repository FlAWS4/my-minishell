/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 16:48:43 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 00:30:17 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle pipe token
 */
static int	create_pipe_token(t_token **tokens, int preceded_by_space)
{
    char	*value;
    t_token *token;

    if (!tokens)
        return (-1);
        
    value = ft_strdup("|");
    if (!value)
        return (-1);
        
    token = create_token(TOKEN_PIPE, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    
    add_token(tokens, token);
    return (0);
}

/**
 * Handle input redirection tokens
 */
static int	handle_input_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;
    t_token *token;

    if (!input || !tokens)
        return (-1);
        
    if (input[i + 1] && input[i + 1] == '<')
    {
        value = ft_strdup("<<");
        if (!value)
            return (-1);
        token = create_token(TOKEN_HEREDOC, value, preceded_by_space);
        if (!token)
        {
            free(value);
            return (-1);
        }
        add_token(tokens, token);
        return (i + 2);  // Skip both '<' characters
    }
    value = ft_strdup("<");
    if (!value)
        return (-1);
    token = create_token(TOKEN_REDIR_IN, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (i + 1);  // Skip the '<' character
}

/**
 * Handle output redirection tokens
 */
static int	handle_output_redir(char *input, int i, t_token **tokens, 
    int preceded_by_space)
{
    char	*value;
    t_token *token;

    if (!input || !tokens)
        return (-1);
        
    if (input[i + 1] && input[i + 1] == '>')
    {
        value = ft_strdup(">>");
        if (!value)
            return (-1);
        token = create_token(TOKEN_REDIR_APPEND, value, preceded_by_space);
        if (!token)
        {
            free(value);
            return (-1);
        }
        add_token(tokens, token);
        return (i + 2);  // Skip both '>' characters
    }
    value = ft_strdup(">");
    if (!value)
        return (-1);
    token = create_token(TOKEN_REDIR_OUT, value, preceded_by_space);
    if (!token)
    {
        free(value);
        return (-1);
    }
    add_token(tokens, token);
    return (i + 1);  // Skip the '>' character
}

/**
 * Handle special characters (pipes, redirections)
 */
int	handle_special(char *input, int i, t_token **tokens)
{
    int	preceded_by_space;
    int	result;

    if (!input || !tokens || i < 0)
        return (-1);
        
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
 * Check if redirection token has a valid target
 */
static int	has_valid_redir_target(t_token *redir, t_token *next)
{
    if (!redir || !next)
        return (0);
        
    return (next->type == TOKEN_WORD || 
           next->type == TOKEN_SINGLE_QUOTE || 
           next->type == TOKEN_DOUBLE_QUOTE);
}

/**
 * Validate token syntax with improved error checking
 */
int	validate_syntax(t_token *tokens)
{
    t_token	*current;
    t_token	*next;

    if (!tokens)
        return (1); // Empty input is valid
        
    current = tokens;
    while (current)
    {
        next = current->next;
        
        // Pipes must not be at the beginning or end
        if (current->type == TOKEN_PIPE)
        {
            if (!next || current == tokens)
            {
                print_syntax_error("|", TOKEN_PIPE);
                return (0);
            }
                
            // Check for consecutive pipes
            if (next->type == TOKEN_PIPE)
            {
                print_syntax_error("|", TOKEN_PIPE);
                return (0);
            }
        }
        
        // Redirections must be followed by a valid target
        if (current->type == TOKEN_REDIR_IN || 
            current->type == TOKEN_REDIR_OUT ||
            current->type == TOKEN_REDIR_APPEND ||
            current->type == TOKEN_HEREDOC)
        {
            if (!has_valid_redir_target(current, next))
            {
                if (!next)
                    print_syntax_error("newline", -1);
                else
                    print_syntax_error(current->value, current->type);
                return (0);
            }
        }
        
        current = next;
    }
    
    return (1);
}

/**
 * Checks if a character is a whitespace
 */
int	is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n' || 
            c == '\v' || c == '\f' || c == '\r');
}

/**
 * Check for unclosed quotes in input string
 */
int	check_unclosed_quotes(char *input)
{
    int	i;
    int	in_single_quote;
    int	in_double_quote;

    if (!input)
        return (1);
        
    i = 0;
    in_single_quote = 0;
    in_double_quote = 0;
    
    while (input[i])
    {
        if (input[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (input[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        i++;
    }
    
    if (in_single_quote || in_double_quote)
    {
        ft_putstr_fd("minishell: syntax error: unclosed quotes\n", STDERR_FILENO);
        return (1);
    }
    
    return (0);
}
