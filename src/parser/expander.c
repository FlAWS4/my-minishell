/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/03 21:40:40 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Join two strings and free the first one
 */
char *ft_strjoin_free(char *s1, char *s2)
{
    char *result;
    
    result = ft_strjoin(s1, s2);
    free(s1);
    return (result);
}

/**
 * Free all memory allocated during expansion
 */
void free_expansion_parts(char *var_name, char *var_value, char **parts)
{
    if (var_name)
        free(var_name);
    if (var_value)
        free(var_value);
    if (parts[0])
        free(parts[0]);
    if (parts[1])
        free(parts[1]);
}

/**
 * Get variable name from string starting at position
 */
char *get_var_name(char *str)
{
    int i;
    char *name;
    
    i = 0;
    if (str[i] == '?')
        return (ft_strdup("?"));
        
    while (str[i] && (ft_isalnum(str[i]) || str[i] == '_'))
        i++;
        
    if (i == 0)
        return (NULL);
        
    name = ft_substr(str, 0, i);
    return (name);
}

/**
 * Get variable value from environment or special vars
 */
char *get_var_value(t_shell *shell, char *name)
{
    char *value;
    
    if (!name)
        return (NULL);
        
    // Handle special variable $?
    if (ft_strcmp(name, "?") == 0)
        return (ft_itoa(shell->exit_status));
        
    // Get from environment
    value = get_env_value(shell->env, name);
    if (value)
        return (ft_strdup(value));
        
    return (NULL);
}

/**
 * Handle quoting in variable expansion
 */
static int is_quoted_var(char *str, int pos)
{
    int i;
    int in_single_quote;
    
    i = 0;
    in_single_quote = 0;
    
    while (i < pos)
    {
        if (str[i] == '\'')
            in_single_quote = !in_single_quote;
        i++;
    }
    
    return (in_single_quote);
}

/**
 * Expand variables in string
 */
char *expand_variables(t_shell *shell, char *token)
{
    char *result;
    int i;
    
    if (!token)
        return (NULL);
        
    result = ft_strdup(token);
    if (!result)
        return (NULL);
        
    i = 0;
    while (result[i])
    {
        // Skip variables in single quotes
        if (result[i] == '$' && !is_quoted_var(result, i))
        {
            result = expand_one_var(shell, result, &i);
            if (!result)
                return (NULL);
        }
        else
            i++;
    }
    
    return (result);
}

/**
 * Expand a single variable
 */
char *expand_one_var(t_shell *shell, char *str, int *i)
{
    char *var_name;
    char *var_value;
    char *parts[2] = {NULL, NULL};
    char *result;

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
 * Special handling for heredoc variable expansion
 */
char *expand_heredoc_content(t_shell *shell, char *content)
{
    // Similar to expand_variables but with different quoting rules
    // In heredocs, quotes don't prevent expansion
    char *result = ft_strdup("");
    char *var_name;
    char *var_value;
    int i = 0;
    
    if (!content || !result)
        return (NULL);
    
    while (content[i])
    {
        if (content[i] == '$' && content[i+1] && 
            (ft_isalnum(content[i+1]) || content[i+1] == '?' || content[i+1] == '_'))
        {
            // Found a variable to expand
            int start = i + 1;
            int len = 0;
            
            // Handle special variable $?
            if (content[start] == '?')
            {
                var_name = ft_strdup("?");
                len = 1;
            }
            else
            {
                // Find the length of the variable name
                while (content[start + len] && 
                      (ft_isalnum(content[start + len]) || content[start + len] == '_'))
                    len++;
                    
                // Extract the variable name
                var_name = ft_substr(content, start, len);
            }
            
            if (!var_name)
            {
                free(result);
                return (NULL);
            }
            
            // Get the variable value
            var_value = get_var_value(shell, var_name);
            
            // Append the value to the result
            char *temp = result;
            if (var_value)
                result = ft_strjoin(result, var_value);
            else
                result = ft_strjoin(result, "");
                
            free(temp);
            free(var_name);
            if (var_value)
                free(var_value);
            
            // Skip past the variable name
            i += len + 1;
        }
        else
        {
            // Add the character to the result
            char c[2] = {content[i], '\0'};
            char *temp = result;
            result = ft_strjoin(result, c);
            free(temp);
            i++;
        }
    }
    
    return result;
}

/**
 * Expand variables in tokens - Make sure this is implemented
 */
void expand_variables_in_tokens(t_token *tokens, t_shell *shell)
{
    t_token *current;
    char *expanded;
    
    if (!tokens || !shell)
        return;
    
    current = tokens;
    while (current)
    {
        // Only expand variables in words and double-quoted strings
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
/**
 * Check if heredoc should expand variables based on delimiter
 * (if delimiter is quoted, no expansion)
 */
int should_expand_heredoc(char *delimiter)
{
    int i = 0;
    
    while (delimiter[i])
    {
        if (delimiter[i] == '\'' || delimiter[i] == '\"')
            return (0);
        i++;
    }
    
    return (1);
}

/**
 * Process heredoc content for variable expansion
 */
char *process_heredoc_content(t_shell *shell, char *content, char *delimiter)
{
    // If delimiter is quoted, don't expand variables
    if (!should_expand_heredoc(delimiter))
        return (ft_strdup(content));
        
    // Otherwise expand variables
    return (expand_heredoc_content(shell, content));
}
