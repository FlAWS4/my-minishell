/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 22:16:36 by my42             ###   ########.fr       */
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
 * Add a redirection to the cmd structure
 */
int	add_redirection(t_cmd *cmd, int type, char *word)
{
    t_redirection	*new;
    t_redirection	*temp;

    if (!cmd || !word)
        return (0);
        
    new = (t_redirection *)malloc(sizeof(t_redirection));
    if (!new)
        return (0);
        
    new->type = type;
    new->word = ft_strdup(word);
    if (!new->word)
    {
        free(new);
        return (0);
    }
    
    // Initialize file descriptors to -1
    new->input_fd = -1;
    new->output_fd = -1;
    new->temp_file = NULL;
    new->next = NULL;
    
    // Add to list
    if (!cmd->redirections)
        cmd->redirections = new;
    else
    {
        temp = cmd->redirections;
        while (temp->next)
            temp = temp->next;
        temp->next = new;
    }
    
    return (1);
}

/**
 * Handle input redirection
 */
int	handle_redir_in(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    
    // Store filename for backward compatibility
    cmd->input_file = ft_strdup((*token)->next->value);
    if (!cmd->input_file)
        return (0);
    
    // Add to redirections list - this is the primary mechanism now
    if (!add_redirection(cmd, TOKEN_REDIR_IN, (*token)->next->value))
        return (0);
    
    // Mark input_fd as not set
    cmd->input_fd = -1;
    
    // Skip target token
    *token = (*token)->next;
    return (1);
}

/**
 * Handle output redirection
 */
int	handle_redir_out(t_token **token, t_cmd *cmd, int append)
{
    int	token_type;

    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
    
    // Store filename for backward compatibility
    cmd->output_file = ft_strdup((*token)->next->value);
    if (!cmd->output_file)
        return (0);
    
    // Set append mode flag
    cmd->append_mode = append;
    
    // Determine token type
    token_type = append ? TOKEN_REDIR_APPEND : TOKEN_REDIR_OUT;
    
    // Add to redirections list - this is the primary mechanism now
    if (!add_redirection(cmd, token_type, (*token)->next->value))
        return (0);
    
    // Skip target token
    *token = (*token)->next;
    return (1);
}

/**
 * Handle heredoc redirection
 */
int	handle_heredoc(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (1);  // FIXED: Return 1 for error
    
    printf("DEBUG: Found heredoc with delimiter: %s\n", (*token)->next->value);
    
    // Store the delimiter
    cmd->heredoc_delim = ft_strdup((*token)->next->value);
    if (!cmd->heredoc_delim)
        return (1);  // FIXED: Return 1 for error
    
    // Initialize heredoc file to NULL and input_fd to -1
    cmd->heredoc_file = NULL;
    cmd->input_fd = -1;
    
    // Add to redirections list for completeness
    if (!add_redirection(cmd, TOKEN_HEREDOC, (*token)->next->value))
        return (1);  // FIXED: Return 1 for error
    
    // Skip the delimiter token
    *token = (*token)->next;
    
    return (0);  // FIXED: Return 0 for success
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
 * Parse redirection tokens
 */
int parse_redirections(t_token **token, t_cmd *cmd)
{
    t_token_type type;
    
    if (!token || !*token || !cmd)
        return (1);
    
    type = (*token)->type;
    printf("DEBUG: Parsing redirection token type: %d\n", type);
    
    if (type == TOKEN_HEREDOC)
        return handle_heredoc(token, cmd);
    else if (type == TOKEN_REDIR_IN)
        return handle_redir_in(token, cmd);
    else if (type == TOKEN_REDIR_OUT || type == TOKEN_REDIR_APPEND)
        return handle_redir_out(token, cmd, type == TOKEN_REDIR_APPEND);
    
    return (1);
}
