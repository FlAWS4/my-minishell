/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   perser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/14 20:39:14 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Handle input redirection
 */
int	handle_redir_in(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || (*token)->next->type != TOKEN_WORD)
        return (0);
    cmd->input_file = ft_strdup((*token)->next->value);
    *token = (*token)->next;
    return (1);
}

/**
 * Handle output redirection
 */
int	handle_redir_out(t_token **token, t_cmd *cmd, int append)
{
    if (!(*token)->next || (*token)->next->type != TOKEN_WORD)
        return (0);
    cmd->output_file = ft_strdup((*token)->next->value);
    cmd->append_mode = append;
    *token = (*token)->next;
    return (1);
}

/**
 * Handle heredoc redirection
 */
int	handle_heredoc(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || (*token)->next->type != TOKEN_WORD)
        return (0);
    cmd->heredoc_delim = ft_strdup((*token)->next->value);
    *token = (*token)->next;
    return (1);
}

/**
 * Parse redirection tokens
 */
int	parse_redirections(t_token **tokens, t_cmd *cmd)
{
    t_token	*token;
    
    token = *tokens;
    if (token->type == TOKEN_REDIR_IN)
        return (handle_redir_in(&token, cmd));
    else if (token->type == TOKEN_REDIR_OUT)
        return (handle_redir_out(&token, cmd, 0));
    else if (token->type == TOKEN_REDIR_APPEND)
        return (handle_redir_out(&token, cmd, 1));
    else if (token->type == TOKEN_HEREDOC)
        return (handle_heredoc(&token, cmd));
    *tokens = token;
    return (1);
}
