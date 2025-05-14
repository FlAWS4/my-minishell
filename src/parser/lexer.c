/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:10 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/14 20:25:40 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Checks if a character is a whitespace
 */
int	is_whitespace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

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
 * Handle a quoted string and add it as a token
 */
int	handle_quote(char *input, int i, t_token **tokens)
{
    int		start;
    char	quote;
    char	*value;

    quote = input[i++];
    start = i;
    while (input[i] && input[i] != quote)
        i++;
    if (!input[i])
        return (-1); // Unclosed quote
    value = malloc(i - start + 1);
    if (!value)
        return (-1);
    ft_strlcpy(value, &input[start], i - start + 1);
    add_token(tokens, create_token(TOKEN_WORD, value));
    return (i);
}

/**
 * Handle special characters (pipes, redirections)
 */
int	handle_special(char *input, int i, t_token **tokens)
{
    if (input[i] == '|')
    {
        add_token(tokens, create_token(TOKEN_PIPE, ft_strdup("|")));
    }
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
    
    start = i;
    while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
        && input[i] != '\'' && input[i] != '\"')
        i++;
    add_token(tokens, create_token(TOKEN_WORD, 
        ft_substr(input, start, i - start)));
    return (i);
}

/**
 * Process tokens from input string
 */
t_token	*process_tokens(char *input)
{
    t_token	*tokens;
    int		i;
    
    tokens = NULL;
    i = 0;
    while (input[i])
    {
        if (is_whitespace(input[i]))
            i++;
        else if (input[i] == '\'' || input[i] == '\"')
        {
            i = handle_quote(input, i, &tokens);
            if (i == -1)
                return (tokens);
            i++;
        }
        else if (is_special(input[i]))
            i = handle_special(input, i, &tokens) + 1;
        else
            i = handle_word(input, i, &tokens);
    }
    return (tokens);
}

/**
 * Tokenize the input string
 */
t_token	*tokenize(char *input)
{
    if (!input)
        return (NULL);
    return (process_tokens(input));
}

/**
 * Tokenize the input string
 
t_token	*tokenize(char *input)
{
    t_token	*tokens;
    int		i;
    int		start;
    
    tokens = NULL;
    i = 0;
    while (input[i])
    {
        if (is_whitespace(input[i]))
        {
            i++;
            continue;
        }
        else if (input[i] == '\'' || input[i] == '\"')
        {
            i = handle_quote(input, i, &tokens);
            if (i == -1)
                return (tokens); // Error handling will be done elsewhere
        }
        else if (is_special(input[i]))
        {
            i = handle_special(input, i, &tokens);
        }
        else
        {
            start = i;
            while (input[i] && !is_whitespace(input[i]) && !is_special(input[i])
                && input[i] != '\'' && input[i] != '\"')
                i++;
            add_token(&tokens, create_token(TOKEN_WORD, 
                ft_substr(input, start, i - start)));
            continue;
        }
        i++;
    }
    return (tokens);
}
*/