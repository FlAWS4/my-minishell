/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/15 09:41:54 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Check if token can be used as a redirection target
 */
static int is_valid_redir_target(t_token *token)
{
    return (token && (token->type == TOKEN_WORD || 
            token->type == TOKEN_SINGLE_QUOTE || 
            token->type == TOKEN_DOUBLE_QUOTE));
}

/**
 * Initialize a new redirection structure
 */
static t_redirection *init_redirection(int type, char *word, int quoted)
{
    t_redirection *new;

    new = (t_redirection *)malloc(sizeof(t_redirection));
    if (!new)
        return (NULL);
    new->type = type;
    new->word = ft_strdup(word);
    if (!new->word)
    {
        free(new);
        return (NULL);
    }
    new->input_fd = -1;
    new->output_fd = -1;
    new->quoted = quoted;
    new->temp_file = NULL;
    new->next = NULL;
    return (new);
}

/**
 * Add a redirection to the cmd structure
 */
int add_redirection(t_cmd *cmd, int type, char *word, int quoted)
{
    t_redirection *new;
    t_redirection *temp;

    if (!cmd || !word)
        return (0);
    new = init_redirection(type, word, quoted);
    if (!new)
        return (0);
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
static int handle_redir_in(t_token **token, t_cmd *cmd)
{
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
        
    if (!add_redirection(cmd, TOKEN_REDIR_IN, (*token)->next->value, 0))
        return (0);
        
    cmd->input_fd = -1;
    *token = (*token)->next;
    return (1);
}

/**
 * Handle output redirection
 */
static int handle_redir_out(t_token **token, t_cmd *cmd, int append)
{
    int token_type;

    if (!(*token)->next || !is_valid_redir_target((*token)->next))
        return (0);
        
    token_type = append ? TOKEN_REDIR_APPEND : TOKEN_REDIR_OUT;
    
    if (!add_redirection(cmd, token_type, (*token)->next->value, 0))
        return (0);
        
    *token = (*token)->next;
    return (1);
}

/**
 * Handle heredoc redirection
 */
static int handle_heredoc(t_token **token, t_cmd *cmd, t_shell *shell)
{
    int quoted;
    char *delim = NULL;
    
    (void)shell;
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
    {
        display_error(ERR_SYNTAX, "expected delimiter after <<", NULL);
        return (1);
    }
    
    // Set quoted flag based on token type
    quoted = ((*token)->next->type == TOKEN_SINGLE_QUOTE || 
              (*token)->next->type == TOKEN_DOUBLE_QUOTE);
    
    // Get a clean copy of the token value
    delim = ft_strdup((*token)->next->value);
    if (!delim)
        return (1);
    
    // Only process for non-quoted tokens or for tokens with embedded quotes
    if (!quoted)
    {
        // Check for colon format (from old lexer format)
        char *colon = ft_strchr(delim, ':');
        if (colon)
        {
            // If colon is at beginning, skip over it and next char
            if (colon == delim && *(colon+1))
            {
                char *clean = ft_strdup(delim + 2);
                free(delim);
                delim = clean;
            }
            // If colon is in middle/end, truncate it
            else if (colon != delim)
            {
                *colon = '\0';
            }
        }
        // Check for embedded quotes in non-quoted tokens
        else if (ft_strchr(delim, '\'') || ft_strchr(delim, '\"'))
        {
            // For tokens that contain quotes but aren't quote tokens themselves
            quoted = 1;  // Mark as quoted if we find quotes
        }
    }
    
    printf("After stripping, delim: '%s'\n", delim);
    
    // Store the processed delimiter
   if (cmd->heredoc_delim)
        free(cmd->heredoc_delim);
    cmd->heredoc_delim = ft_strdup(delim);
    
    cmd->input_fd = -1;
    
    // Pass the current delimiter to add_redirection
    if (!add_redirection(cmd, TOKEN_HEREDOC, delim, quoted))
    {
        free(delim);
        return (1);
    }
    
    cmd->heredocs_processed = 0;
    *token = (*token)->next;
    return (0);
    cmd->heredocs_processed = 0;
    *token = (*token)->next;
    return (0);
}

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
 * Parse redirection tokens
 * Return 0 for success, 1 for failure
 */
int parse_redirections(t_token **token, t_cmd *cmd, t_shell *shell)
{
    t_token_type type;

    if (!token || !*token || !cmd)
        return (1);
        
    type = (*token)->type;
    
    if (type == TOKEN_HEREDOC)
        return handle_heredoc(token, cmd, shell);
    else if (type == TOKEN_REDIR_IN)
        return !handle_redir_in(token, cmd);
    else if (type == TOKEN_REDIR_OUT)
        return !handle_redir_out(token, cmd, 0);
    else if (type == TOKEN_REDIR_APPEND)
        return !handle_redir_out(token, cmd, 1);
        
    return (1);
}
