/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/27 01:51:02 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Free token list
 */
void free_token_list(t_token *tokens)
{
    t_token *tmp;

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
void free_shell(t_shell *shell)
{
    t_env *current;
    t_env *next;
    
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
/*
 * Check if character is a shell special character

static int	is_special_char(char c)
{
    return (c == '|' || c == '<' || c == '>' || c == ';' || 
            c == ' ' || c == '\t' || c == '\n');
}
*/

void	handle_word_token(t_cmd *cmd, t_token **token)
{
    char	*word;
    t_token	*current;
    t_token	*next;
    int	is_first_arg;

    is_first_arg = (cmd->args == NULL);
    word = ft_strdup((*token)->value);
    current = *token;
    next = current->next;

    // Only join adjacent tokens for command name (first argument)
    // Once we have the command name, additional tokens become separate arguments
    if (is_first_arg)
    {
        // Join adjacent word tokens for command name only
        while (next && next->type == TOKEN_WORD && 
               !is_whitespace(next->value[0]))
        {
            char *temp = word;
            word = ft_strjoin(word, next->value);
            free(temp);
            current = next;
            next = current->next;
        }
    }

    // Add the word as an argument
    add_arg(cmd, word);
    *token = current;
}
