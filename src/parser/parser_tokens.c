/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/14 20:39:49 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle word token during parsing
 */
void	handle_word_token(t_cmd *cmd, t_token *token)
{
    add_arg(cmd, ft_strdup(token->value));
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
 * Process a single token
 */
int	process_token(t_token **token, t_cmd **current)
{
    if ((*token)->type == TOKEN_WORD)
        handle_word_token(*current, *token);
    else if ((*token)->type == TOKEN_PIPE)
    {
        *current = handle_pipe_token(*current);
        if (!*current)
            return (0);
    }
    else if ((*token)->type >= TOKEN_REDIR_IN && 
            (*token)->type <= TOKEN_HEREDOC)
    {
        if (!parse_redirections(token, *current))
            return (0);
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
    while (token)
    {
        if (!process_token(&token, &current))
            break;
        token = token->next;
    }
    return (cmd_list);
}
