/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 05:05:41 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if token is a redirection token
 */
int is_redirection_token(t_token *token)
{
    if (!token)
        return (0);
    
    return (token->type == TOKEN_REDIR_IN || 
            token->type == TOKEN_REDIR_OUT ||
            token->type == TOKEN_REDIR_APPEND ||
            token->type == TOKEN_HEREDOC);
}

/**
 * Parse redirections from tokens
 */
int parse_redirections(t_token **token, t_cmd *cmd, t_shell *shell)
{
    int type;
    int quoted;
    char *word;
    
    (void)shell; // shell is not used in this function, but kept for consistency
    if (!token || !*token || !cmd)
        return (0);
    type = (*token)->type;
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
    {
        display_error(ERR_SYNTAX, "syntax error near unexpected token", NULL);
        return (1);
    }
    *token = (*token)->next;
    
    // Special handling for heredoc quoting
    quoted = 0;
    if (type == TOKEN_HEREDOC)
    {
        if ((*token)->preceded_by_space == 2)
            quoted = 1;
    }
    else
    {
        if ((*token)->type == TOKEN_SINGLE_QUOTE || 
            (*token)->type == TOKEN_DOUBLE_QUOTE)
            quoted = 1;
    }
                 
    word = ft_strdup((*token)->value);
    if (!word)
    {
        display_error(ERROR_MEMORY, "redirection", "Memory allocation failed");
        return (1);
    }
    if (!add_redirection(cmd, type, word, quoted))
    {
        free(word);
        return (1);
    }
    return (0);
}
