/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:51:14 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:16:22 by mshariar         ###   ########.fr       */
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

    if (ft_strcmp(name, "?") == 0)
    {
        sprintf(exit_status, "%d", shell->exit_status);
        return (ft_strdup(exit_status));
    }
    value = get_env_value(shell->env, name);
    if (value)
        return (ft_strdup(value));
    return (ft_strdup(""));
}

/**
 * Expand all variables in token list
 */
void	expand_token_variables(t_shell *shell, t_token *tokens)
{
    t_token	*current;
    char	*expanded;

    current = tokens;
    while (current)
    {
        if (current->type == TOKEN_WORD)
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
