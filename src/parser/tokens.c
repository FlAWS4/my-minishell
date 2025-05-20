/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tokens.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:30:53 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/20 21:00:29 by mshariar         ###   ########.fr       */
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
        
    // Free environment variables
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
    
    // Free shell struct itself
    free(shell);
}
