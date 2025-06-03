/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 01:40:31 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Free a single command and its resources
 */
void	free_cmd(t_cmd *cmd)
{
    int			i;
    t_redirection	*redir;
    t_redirection	*next_redir;

    if (!cmd)
        return;
    if (cmd->args)
    {
        i = 0;
        while (cmd->args[i])
            free(cmd->args[i++]);
        free(cmd->args);
    }
    if (cmd->input_file)
        free(cmd->input_file);
    if (cmd->output_file)
        free(cmd->output_file);
    if (cmd->heredoc_delim)
        free(cmd->heredoc_delim);
        
    // Free redirections list
    redir = cmd->redirections;
    while (redir)
    {
        next_redir = redir->next;
        if (redir->word)
            free(redir->word);
        free(redir);
        redir = next_redir;
    }
    
    // Close any open file descriptors
    if (cmd->input_fd >= 0)
        close(cmd->input_fd);
    if (cmd->output_fd >= 0)
        close(cmd->output_fd);
        
    free(cmd);
}

/**
 * Free the shell structure and all its components
 */
void	free_shell(t_shell *shell)
{
    t_env	*current;
    t_env	*next;
    
    if (!shell)
        return;
    current = shell->env;
    while (current)
    {
        next = current->next;
        if (current->key)
            free(current->key);
        if (current->value)
            free(current->value);
        free(current);
        current = next;
    }
    if (shell->cmd)
        free_cmd_list(shell->cmd);
    free(shell);
}

/**
 * Join consecutive tokens without spaces
 */
static char	*join_consecutive_tokens(char *word, t_token **current)
{
    t_token	*next;
    char	*temp;

    next = (*current)->next;
    while (next && (next->type == TOKEN_WORD || 
            next->type == TOKEN_SINGLE_QUOTE ||
            next->type == TOKEN_DOUBLE_QUOTE))
    {
        if (next->preceded_by_space)
            break;
        temp = word;
        word = ft_strjoin(word, next->value);
        free(temp);
        if (!word)
            return (NULL);
        *current = next;
        next = next->next;
    }
    return (word);
}

/**
 * Process word tokens and handle token joining when no spaces
 */
void	join_word_tokens(t_cmd *cmd, t_token **token)
{
    char	*word;
    t_token	*current;
    int		is_first_arg;

    is_first_arg = (cmd->args == NULL);
    word = ft_strdup((*token)->value);
    if (!word)
        return;
    current = *token;
    
    if (is_first_arg)
    {
        word = join_consecutive_tokens(word, &current);
        if (!word)
            return;
    }
    
    add_arg(cmd, word);
    *token = current;
}
