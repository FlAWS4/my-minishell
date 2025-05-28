/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 20:56:40 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Free token list
 */
void	free_token_list(t_token *tokens)
{
    t_token	*tmp;

    while (tokens)
    {
        tmp = tokens;
        tokens = tokens->next;
        if (tmp->value)
            free(tmp->value);
        free(tmp);
    }
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
void merge_adjacent_quoted_tokens(t_token **tokens)
{
    t_token *current = *tokens;
    
    while (current && current->next)
    {
        // Only merge tokens that are NOT separated by space
        if ((current->type == TOKEN_WORD || 
             current->type == TOKEN_SINGLE_QUOTE || 
             current->type == TOKEN_DOUBLE_QUOTE) &&
            (current->next->type == TOKEN_WORD ||
             current->next->type == TOKEN_SINGLE_QUOTE ||
             current->next->type == TOKEN_DOUBLE_QUOTE) &&
            !current->next->preceded_by_space)  // Add this condition
        {
            // Merge the values
            char *new_value = ft_strjoin(current->value, current->next->value);
            free(current->value);
            current->value = new_value;
            current->type = TOKEN_WORD;
            t_token *to_delete = current->next;
            current->next = to_delete->next;
            free(to_delete->value);
            free(to_delete);
        }
        else
        {
            current = current->next;
        }
    }
}
