/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 21:34:11 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Get variable name from string
 */
static char	*get_var_name(char *str)
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
static char	*get_var_value(t_shell *shell, char *name)
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
 * Expand a single variable
 */
static char	*expand_one_var(t_shell *shell, char *str, int *i)
{
    char	*var_name;
    char	*var_value;
    char	*before;
    char	*after;
    char	*temp;

    (*i)++;
    if (str[*i] == '?' || ft_isalnum(str[*i]) || str[*i] == '_')
    {
        var_name = get_var_name(&str[*i]);
        var_value = get_var_value(shell, var_name);
        before = ft_substr(str, 0, *i - 1);
        after = ft_strdup(&str[*i + ft_strlen(var_name)]);
        
        temp = ft_strjoin(before, var_value);
        free(before);
        before = temp;
        
        temp = ft_strjoin(before, after);
        free(before);
        free(after);
        free(var_name);
        free(var_value);
        free(str);
        
        *i = *i - 1 + ft_strlen(var_value);
        return (temp);
    }
    return (str);
}

/**
 * Expand variables in string
 */
char	*expand_variables(t_shell *shell, char *str)
{
    int		i;
    int		in_single_quote;
    int		in_double_quote;
    char	*result;

    i = 0;
    in_single_quote = 0;
    in_double_quote = 0;
    result = ft_strdup(str);
    
    while (result[i])
    {
        if (result[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (result[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        else if (result[i] == '$' && !in_single_quote && 
                (ft_isalnum(result[i + 1]) || result[i + 1] == '?' || 
                result[i + 1] == '_'))
            result = expand_one_var(shell, result, &i);
        i++;
    }
    
    return (result);
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
            free(current->value);
            current->value = expanded;
        }
        current = current->next;
    }
}
