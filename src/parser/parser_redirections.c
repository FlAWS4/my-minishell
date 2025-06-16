/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_redirections.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:38:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 00:15:54 by mshariar         ###   ########.fr       */
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
/**
 * Handle heredoc redirection
 * Parses the heredoc token and its delimiter
 */
static int handle_heredoc(t_token **token, t_cmd *cmd, t_shell *shell)
{
    int quoted;
    char *delim = NULL;
    
    // Shell parameter might be needed for future features
    (void)shell;
    
    // Check if there's a valid token after heredoc
    if (!(*token)->next || !is_valid_redir_target((*token)->next))
    {
        display_error(ERR_SYNTAX, "expected delimiter after <<", NULL);
        return (1);
    }
    
    // Determine if the delimiter is quoted
    quoted = ((*token)->next->type == TOKEN_SINGLE_QUOTE || 
              (*token)->next->type == TOKEN_DOUBLE_QUOTE);
    
    // Copy the token value without modification
    delim = ft_strdup((*token)->next->value);
    if (!delim)
        return (1);
    
    // Process special cases for unquoted delimiters
    if (!quoted)
    {
        // Handle special colon format if present
        char *colon = ft_strchr(delim, ':');
        if (colon)
        {
            if (colon == delim && *(colon+1))
            {
                // For ":xyz" format, use "xyz"
                char *clean = ft_strdup(delim + 2);
                if (!clean)
                {
                    free(delim);
                    return (1);
                }
                free(delim);
                delim = clean;
            }
            else if (colon != delim)
            {
                // For "abc:xyz" format, use "abc"
                *colon = '\0';
            }
        }
        // If there are quotes within the unquoted delimiter, treat as quoted
        else if (ft_strchr(delim, '\'') || ft_strchr(delim, '\"'))
        {
            quoted = 1;
        }
        
        // Note: We do NOT expand variables in heredoc delimiters
        // This matches bash's behavior
    }
    
    // Store the delimiter in the command structure
    if (cmd->heredoc_delim)
        free(cmd->heredoc_delim);
        
    cmd->heredoc_delim = ft_strdup(delim);
    if (!cmd->heredoc_delim)
    {
        free(delim);
        return (1);
    }
    
    // Mark input as pending
    cmd->input_fd = -1;
    
    // Add the redirection to the command
    if (!add_redirection(cmd, TOKEN_HEREDOC, delim, quoted))
    {
        free(delim);
        return (1);
    }
    
    // Cleanup and advance token
    free(delim);
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
