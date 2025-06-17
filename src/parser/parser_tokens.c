/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/18 00:47:07 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"


/**
 * Process a token and update the command structure
 * Returns 0 on success, non-zero on error
 */
int process_token(t_token **token, t_cmd **current, t_shell *shell)
{
    if (!token || !*token || !current || !*current)
        return (1);
        
    if ((*token)->type == TOKEN_WORD || (*token)->type == TOKEN_SINGLE_QUOTE
        || (*token)->type == TOKEN_DOUBLE_QUOTE)
    {
        // Add word to command arguments
        join_word_tokens(*current, token);
    }
    else if ((*token)->type == TOKEN_PIPE)
    {
        *current = handle_pipe_token(*current);
        if (!*current)
            return (1);
    }
    else if (is_redirection_token(*token))
    {
        if (parse_redirections(token, *current, shell) != 0)
            return (1);
    }
    return (0);
}

/**
 * Handle pipe token by creating a new command in the chain
 */
t_cmd *handle_pipe_token(t_cmd *current)
{
    t_cmd *new_cmd;

    if (!current)
        return (NULL);
    new_cmd = create_cmd();
    if (!new_cmd)
        return (NULL);
    current->next = new_cmd;
    return (new_cmd);
}

/**
 * Parse tokens into command structures
 */
t_cmd *parse_tokens(t_token *tokens, t_shell *shell)
{
    t_cmd *cmd_list;
    t_cmd *current;
    t_token *token;
    
    if (!shell || !tokens)
        return (NULL);
        
    cmd_list = create_cmd();
    if (!cmd_list)
        return (NULL);  
        
    current = cmd_list;
    token = tokens;
    
    while (token)
    {
        if (process_token(&token, &current, shell) != 0)
        {
            free_cmd_list(cmd_list);
            return (NULL);
        }
        token = token->next;
    }
    return (cmd_list);
}

