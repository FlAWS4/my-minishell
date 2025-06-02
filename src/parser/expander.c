/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 03:09:14 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Expand variables in all tokens
 */
void	expand_variables_in_tokens(t_token *tokens, t_shell *shell)
{
    t_token	*current;
    char	*expanded;

    current = tokens;
    while (current)
    {
        // Expand variables in words and double quoted strings
        if (current->type == TOKEN_WORD || current->type == TOKEN_DOUBLE_QUOTE)
        {
            expanded = expand_variables(shell, current->value);
            if (expanded)
            {
                free(current->value);
                current->value = expanded;
            }
        }
        
        // Also expand variables in heredoc delimiters
        if (current->type == TOKEN_HEREDOC && current->next && 
            current->next->type == TOKEN_WORD)
        {
            // Don't actually modify the token here, as this would affect
            // how the heredoc is processed later
            // This is just a comment for future implementation if needed
        }
        
        current = current->next;
    }
}

/**
 * Free memory allocated during expansion
 */
void	free_expansion_parts(char *name, char *value, char **parts)
{
    if (name)
        free(name);
    if (value)
        free(value);
    if (parts[0])
        free(parts[0]);
    if (parts[1])
        free(parts[1]);
}

/**
 * Expand a single variable
 */
char	*expand_one_var(t_shell *shell, char *str, int *i)
{
    char	*var_name;
    char	*var_value;
    char	*parts[2] = {NULL, NULL};
    char	*result;

    (*i)++;
    // Handle cases like $ followed by space or special chars
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
 * Expand variables in string
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
            // Avoid incrementing i after expansion since it's already adjusted
            continue;
        }
        i++;
    }
    return (result);
}

/**
 * Get variable name from string
 */
char	*get_var_name(char *str)
{
    int		i;
    char	*name;

    i = 0;
    if (str[i] == '?')
        return (ft_strdup("?"));
        
    while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
        i++;
        
    name = ft_substr(str, 0, i);
    return (name);
}

/**
 * Get variable value from environment
 */
char	*get_var_value(t_shell *shell, char *name)
{
    t_env	*env;
    char	*exit_status_str;

    if (ft_strcmp(name, "?") == 0)
    {
        exit_status_str = ft_itoa(shell->exit_status);
        return (exit_status_str);
    }
    
    env = find_env_var(shell->env, name);
    if (env && env->value)
        return (ft_strdup(env->value));
        
    return (ft_strdup(""));
}