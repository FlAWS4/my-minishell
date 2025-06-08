/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_tokens.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/14 20:39:44 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 21:59:48 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Process a token and update the command structure
 * Returns 0 on success, non-zero on error
 */
int process_token(t_token **token, t_cmd **current)
{
    if (!token || !*token || !current || !*current)
        return (1);
        
    if ((*token)->type == TOKEN_WORD || 
        (*token)->type == TOKEN_SINGLE_QUOTE || 
        (*token)->type == TOKEN_DOUBLE_QUOTE)
    {
        // Handle word or quoted tokens as arguments
        handle_word_token(*current, token);
    }
    else if ((*token)->type == TOKEN_PIPE)
    {
        // Handle pipe token by creating a new command
        *current = handle_pipe_token(*current);
        if (!*current)
            return (1);
    }
    else if (is_redirection_token(*token))
    {
        // Handle redirection tokens
        if (parse_redirections(token, *current) != 0)
            return (1);
    }
    
    return (0);
}

/**
 * Handle word tokens by adding them as arguments to the command
 */
void handle_word_token(t_cmd *cmd, t_token **token)
{
    if (!cmd || !token || !*token)
        return;
    
    join_word_tokens(cmd, token);
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
    t_cmd *cmd;
    t_cmd *head;
    t_token *current;
    
    (void)shell; // Unused parameter, can be used for shell context later
    printf("DEBUG: Parsing tokens into commands\n");
    
    if (!tokens) {
        printf("DEBUG: No tokens to parse\n");
        return (NULL);
    }
    
    // Create first command
    cmd = create_cmd();
    if (!cmd)
        return (NULL);
    
    head = cmd;
    current = tokens;
    
    // Process each token
    while (current)
    {
        printf("DEBUG: Processing token: %s (type %d)\n", 
               current->value, current->type);
               
        if (process_token(&current, &cmd) != 0)
        {
            printf("DEBUG: Error processing token\n");
            free_cmd_list(head);
            return (NULL);
        }
        
        if (current)
            current = current->next;
    }
    
    // Show first command's args for debugging
    if (head && head->args && head->args[0])
        printf("DEBUG: First command: %s\n", head->args[0]);
    else
        printf("DEBUG: No command args found\n");
        
    return (head);
}
