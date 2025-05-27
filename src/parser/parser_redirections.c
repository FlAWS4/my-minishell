/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 00:24:06 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if token can be used as a redirection target
 */
static int	is_valid_redir_target(t_token *token)
{
    return (token && (token->type == TOKEN_WORD || 
            token->type == TOKEN_SINGLE_QUOTE || 
            token->type == TOKEN_DOUBLE_QUOTE));
}

/**
 * Handle input redirection
 */
int	handle_redir_in(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    cmd->input_file = ft_strdup((*token)->next->value);
    if (!cmd->input_file)
        return (0);
    add_redirection(cmd, TOKEN_REDIR_IN, (*token)->next->value);
    *token = (*token)->next;
    return (1);
}

/**
 * Handle output redirection
 */
int	handle_redir_out(t_token **token, t_cmd *cmd, int append)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    cmd->output_file = ft_strdup((*token)->next->value);
    if (!cmd->output_file)
        return (0);
    cmd->append_mode = append;
    add_redirection(cmd, append ? TOKEN_REDIR_APPEND : TOKEN_REDIR_OUT, 
        (*token)->next->value);
    *token = (*token)->next;
    return (1);
}

/**
 * Handle heredoc redirection
 */
int	handle_heredoc(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
        
    // Store in legacy field (last heredoc only)
    if (cmd->heredoc_delim)
        free(cmd->heredoc_delim);
    cmd->heredoc_delim = ft_strdup((*token)->next->value);
    if (!cmd->heredoc_delim)
        return (0);
    
    // Add to redirections list (all heredocs)
    add_redirection(cmd, TOKEN_HEREDOC, (*token)->next->value);
    
    *token = (*token)->next;
    return (1);
}

/**
 * Check if token is a redirection token
 */
int	is_redirection_token(t_token *token)
{
    if (!token)
        return (0);
        
    return (token->type == TOKEN_REDIR_IN ||
            token->type == TOKEN_REDIR_OUT ||
            token->type == TOKEN_REDIR_APPEND ||
            token->type == TOKEN_HEREDOC);
}

/**
 * Parse redirections and add them to command
 */
int	parse_redirections(t_token **tokens, t_cmd *cmd)
{
    int	success;
    
    success = 1;
    while (*tokens && is_redirection_token(*tokens))
    {
        if ((*tokens)->type == TOKEN_REDIR_IN)
            success = handle_redir_in(tokens, cmd);
        else if ((*tokens)->type == TOKEN_REDIR_OUT)
            success = handle_redir_out(tokens, cmd, 0);
        else if ((*tokens)->type == TOKEN_REDIR_APPEND)
            success = handle_redir_out(tokens, cmd, 1);
        else if ((*tokens)->type == TOKEN_HEREDOC)
            success = handle_heredoc(tokens, cmd);
            
        if (!success)
            return (0);
            
        *tokens = (*tokens)->next;
    }
    return (1);
}
