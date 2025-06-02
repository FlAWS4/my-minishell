/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:39:30 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Add any token type that can be an argument
 */
static int	is_arg_token(t_token *token)
{
    return (token && (token->type == TOKEN_WORD ||
            token->type == TOKEN_SINGLE_QUOTE ||
            token->type == TOKEN_DOUBLE_QUOTE));
}

/**
 * Parse arguments (handles both regular and quoted args)
 */
static void	parse_args(t_token **token, t_cmd *cmd)
{
    t_token	*next;
    char	*arg;

    next = (*token)->next;
    if (next && is_arg_token(next))
    {
        arg = ft_strdup(next->value);
        if (!arg)
            return;
        add_arg(cmd, arg);
        *token = next;
    }
}

/**
 * Handle pipe token during parsing
 */
t_cmd	*handle_pipe_token(t_cmd *current)
{
    if (!current)
        return (NULL);
        
    current->next = create_cmd();
    if (!current->next)
        return (NULL);
    return (current->next);
}

/**
 * Handle word token (command name or argument)
 */
void	handle_word_token(t_cmd *cmd, t_token **token)
{
    char	*word;

    if (!cmd || !token || !*token)
        return;
        
    if (!cmd->args)
    {
        word = ft_strdup((*token)->value);
        if (!word)
            return;
        init_args(cmd, word);
    }
    else
    {
        word = ft_strdup((*token)->value);
        if (!word)
            return;
        add_arg(cmd, word);
    }
}

/**
 * Process a single token and return status for token advancement
 * Return values: 
 * - 0 = error
 * - 1 = success (advance token)
 * - 2 = success (don't advance token - already advanced)
 */
int	process_token(t_token **token, t_cmd **current)
{
    if (!token || !*token || !current || !*current)
        return (0);  // Safety check
        
    if (is_arg_token(*token))
    {
        handle_word_token(*current, token);
        if ((*current)->args && (*current)->args[0] && 
            (*token)->next && is_arg_token((*token)->next))
        {
            parse_args(token, *current);
        }
        return (1);  // Normal token advancement
    }
    else if ((*token)->type == TOKEN_PIPE)
    {
        *current = handle_pipe_token(*current);
        if (!*current)
            return (0);  // Error
        return (1);  // Normal token advancement
    }
    else if (is_redirection_token(*token))
    {
        if (!parse_redirections(token, *current))
            return (0);  // Error
        return (2);  // Token already advanced, don't advance again
    }
    return (1);  // Default: normal token advancement
}

/**
 * Handle initial redirection tokens
 */
static int	handle_initial_redirections(t_token **token, t_cmd *current)
{
    if (!token || !*token || !current)
        return (0);
        
    if ((*token) && is_redirection_token(*token))
    {
        if (!parse_redirections(token, current))
            return (0);
        if (!current->args)
        {
            char *empty = ft_strdup("");
            if (!empty)
                return (0);
            add_arg(current, empty);
        }
        return (1);
    }
    return (1);
}

/**
 * Parse tokens into command structure
 */
t_cmd	*parse_tokens(t_token *tokens, t_shell *shell)
{
    t_cmd	*cmd_list;
    t_cmd	*current;
    t_token	*token;
    int		result;

    if (!tokens)
        return (NULL);
    cmd_list = create_cmd();
    if (!cmd_list)
        return (NULL);
    current = cmd_list;
    token = tokens;
    if (!handle_initial_redirections(&token, current))
    {
        free_cmd_list(cmd_list);
        return (NULL);
    }
    while (token)
    {
        result = process_token(&token, &current);
        if (result == 0)
        {
            free_cmd_list(cmd_list);
            return (NULL);
        }
        if (result == 1)
            token = token->next;
    }
    (void)shell; // Unused parameter for now
    return (cmd_list);
}
