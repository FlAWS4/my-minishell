/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:51:14 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 03:44:26 by my42             ###   ########.fr       */
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
 * Expand a single variable in a string
 */
char	*expand_one_var(t_shell *shell, char *str, int *i)
{
    char	*var_name;
    char	*var_value;
    char	*parts[2] = {NULL, NULL};
    char	*result;

    (*i)++;
    if (!str[*i] || !(str[*i] == '?' || ft_isalnum(str[*i]) || str[*i] == '_'))
        return (str);
        
    var_name = get_var_name(&str[*i]);
    if (!var_name)
        return (str);
        
    var_value = get_var_value(shell, var_name);
    if (!var_value)
        var_value = ft_strdup(""); // Empty string for undefined variables
        
    parts[0] = ft_substr(str, 0, *i - 1);
    parts[1] = ft_strdup(&str[*i + ft_strlen(var_name)]);
    
    if (!parts[0] || !parts[1])
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    
    // Join parts with variable value
    result = ft_strjoin(parts[0], var_value);
    if (!result)
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    
    // Join with trailing part
    result = ft_strjoin_free(result, parts[1]);
    if (!result)
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    
    // Adjust index to point to the end of the expanded value
    *i = ft_strlen(parts[0]) + ft_strlen(var_value) - 1;
    
    free_expansion_parts(var_name, var_value, parts);
    free(str);
    return (result);
}

/**
 * Expand all variables in a string
 */
char	*expand_variables(t_shell *shell, char *str)
{
    int		i;
    int		in_single_quote;
    int		in_double_quote;
    char	*result;

    if (!str)
        return (NULL);
        
    i = 0;
    in_single_quote = 0;
    in_double_quote = 0;
    result = ft_strdup(str);
    if (!result)
        return (NULL);
        
    while (result && result[i])
    {
        // Handle quote state tracking
        if (result[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (result[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        // Expand variables outside single quotes
        else if (result[i] == '$' && !in_single_quote && result[i + 1] &&
                (ft_isalnum(result[i + 1]) || result[i + 1] == '?' ||
                result[i + 1] == '_'))
        {
            result = expand_one_var(shell, result, &i);
            if (!result)
                return (NULL);
            // Don't increment i after expansion since it's already adjusted
            continue;
        }
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
