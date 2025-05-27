/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 01:59:50 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Parse additional arguments after command name
 */
static void	parse_args(t_token **token, t_cmd *cmd)
{
    t_token *current = *token;
    t_token *next = current->next;
    
    // Skip to next token after command name
    if (next && next->type == TOKEN_WORD)
    {
        // Found argument, add it
        add_arg(cmd, ft_strdup(next->value));
        *token = next; // Update token position
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

int	process_token(t_token **token, t_cmd **current)
{
    if ((*token)->type == TOKEN_WORD)
    {
        handle_word_token(*current, token);
        if ((*current)->args && (*current)->args[0] && 
            (*token)->next && (*token)->next->type == TOKEN_WORD)
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
    else if ((*token)->type == TOKEN_REDIR_OUT || 
         (*token)->type == TOKEN_REDIR_IN ||
         (*token)->type == TOKEN_REDIR_APPEND ||
         (*token)->type == TOKEN_HEREDOC)
    {
        if (!parse_redirections(token, *current))
            return (0);
        return (1);
    }
    return (1);
}

/**
 * Handle initial redirection tokens
 */
static int	handle_initial_redirections(t_token **token, t_cmd *current)
{
    if ((*token) && ((*token)->type == TOKEN_REDIR_OUT || 
                     (*token)->type == TOKEN_REDIR_IN ||
                     (*token)->type == TOKEN_REDIR_APPEND ||
                     (*token)->type == TOKEN_HEREDOC))
    {
        if (!parse_redirections(token, current))
            return (0);
        if (!current->args)
            add_arg(current, ft_strdup(""));
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
