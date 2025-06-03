/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:51:14 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 01:46:10 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Get variable name from string
 */
char	*get_var_name(char *str)
{
    int		i;
    char	*name;

    i = 0;
    if (!str)
        return (NULL);
    
    /* Special case for $? */
    if (str[0] == '?')
        return (ft_strdup("?"));
        
    while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
        i++;
    
    name = ft_substr(str, 0, i);
    return (name);
}

/**
 * Get variable value, including special vars
 */
char	*get_var_value(t_shell *shell, char *name)
{
    char	*value;
    char	exit_status[12];

    if (!shell || !name)
        return (NULL);
        
    if (ft_strcmp(name, "?") == 0)
    {
        snprintf(exit_status, sizeof(exit_status), "%d", shell->exit_status);
        return (ft_strdup(exit_status));
    }
    
    value = get_env_value(shell->env, name);
    /* get_env_value already returns a duplicated string */
    if (value)
        return (value);
        
    return (ft_strdup(""));
}

/**
 * Free resources used during variable expansion
 */
void	free_expansion_parts(char *name, char *value, char **parts)
{
    if (name)
        free(name);
    if (value)
        free(value);
    if (parts && parts[0])
        free(parts[0]);
    if (parts && parts[1])
        free(parts[1]);
}

/**
 * Expand all variables in token list
 */
void	expand_token_variables(t_shell *shell, t_token *tokens)
{
    t_token	*current;
    char	*expanded;

    if (!shell || !tokens)
        return;
        
    current = tokens;
    while (current)
    {
        if (current->type == TOKEN_WORD || current->type == TOKEN_DOUBLE_QUOTE)
        {
            expanded = expand_variables(shell, current->value);
            if (expanded)
            {
                free(current->value);
                current->value = expanded;
            }
        }
        current = current->next;
    }
}
