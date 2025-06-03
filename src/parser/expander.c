/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 05:45:48 by my42             ###   ########.fr       */
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
char *expand_variables(t_shell *shell, char *token)
{
    char *result = ft_strdup("");
    char *var_name;
    char *var_value;
    int i = 0;
    
    if (!token || !result)
        return (NULL);
    
    while (token[i])
    {
        if (token[i] == '$' && token[i+1] && ft_isalnum(token[i+1]))
        {
            // Found a variable to expand
            int start = i + 1;
            int len = 0;
            
            // Find the length of the variable name
            while (token[start + len] && (ft_isalnum(token[start + len]) || token[start + len] == '_'))
                len++;
                
            // Extract the variable name
            var_name = ft_substr(token, start, len);
            if (!var_name)
            {
                free(result);
                return (NULL);
            }
            
            // Get the variable value
            var_value = get_env_value(shell->env, var_name);
            
            // Append the value to the result
            char *temp = result;
            if (var_value)
                result = ft_strjoin(result, var_value);
            else
                result = ft_strjoin(result, "");
                
            free(temp);
            free(var_name);
            
            // Skip past the variable name
            i += len + 1;
        }
        else
        {
            // Add the character to the result
            char c[2] = {token[i], '\0'};
            char *temp = result;
            result = ft_strjoin(result, c);
            free(temp);
            i++;
        }
    }
    
    return result;
}
