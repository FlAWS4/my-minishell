/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:25:25 by mshariar         ###   ########.fr       */
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
            return ;
        add_arg(cmd, arg);
        *token = next;
    }
}

/**
 * Handle pipe token during parsing
 */
t_cmd	*handle_pipe_token(t_cmd *current)
{
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

    if (!cmd->args)
    {
        word = ft_strdup((*token)->value);
        if (!word)
            return ;
        init_args(cmd, word);
    }
    else
    {
        word = ft_strdup((*token)->value);
        if (!word)
            return ;
        add_arg(cmd, word);
    }
}

/**
 * Process a single token
 */
int	process_token(t_token **token, t_cmd **current)
{
    if (is_arg_token(*token))
    {
        handle_word_token(*current, token);
        if ((*current)->args && (*current)->args[0] && 
            (*token)->next && is_arg_token((*token)->next))
        {
            parse_args(token, *current);
        }
    }
    else if ((*token)->type == TOKEN_PIPE)
    {
        *current = handle_pipe_token(*current);
        if (!*current)
            return (0);
    }
    else if (is_redirection_token(*token))
    {
        if (!parse_redirections(token, *current))
            return (0);
        // Don't return here - let the main loop advance the token
    }
    return (1);
}

/**
 * Handle initial redirection tokens
 */
static int	handle_initial_redirections(t_token **token, t_cmd *current)
{
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
 * Parse tokens into command structures
 */
t_cmd	*parse_tokens(t_token *tokens)
{
    t_cmd	*cmd_list;
    t_cmd	*current;
    t_token	*token;
    
    if (!tokens)
        return (NULL);
    current = create_cmd();
    if (!current)
        return (NULL);
    cmd_list = current;
    token = tokens;
    if (!handle_initial_redirections(&token, current))
    {
        free_cmd_list(cmd_list);
        return (NULL);
    }
    while (token)
    {
        if (!process_token(&token, &current))
            break;
        token = token->next;
    }
    return (cmd_list);
}
