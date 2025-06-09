/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/09 20:39:04 by mshariar         ###   ########.fr       */
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
 * Expand variables in tokens with word splitting
 * Returns updated token list with proper word splitting
 */
t_token *expand_variables_in_tokens_with_splitting(t_token *tokens, t_shell *shell)
{
    t_token *current;
    t_token *next;
    t_token *head = tokens;
    t_token *prev = NULL;
    char *expanded;
    
    if (!tokens || !shell)
        return (tokens);
    
    current = tokens;
    while (current)
    {
        next = current->next;
        
        // Skip expansion for single-quoted tokens
        if (current->type != TOKEN_SINGLE_QUOTE)
        {
            // Expand variables
            expanded = expand_variables(shell, current->value);
            if (!expanded)
            {
                prev = current;
                current = next;
                continue;
            }
            
            // If value changed and it's not in quotes, perform word splitting
            if (ft_strcmp(expanded, current->value) != 0 && 
                current->type == TOKEN_WORD) 
            {
                char **words = ft_split(expanded, ' ');
                free(expanded);
                
                if (!words || !words[0])
                {
                    if (words)
                        free_env_array(words);
                    prev = current;
                    current = next;
                    continue;
                }
                
                // Replace current token with first word
                free(current->value);
                current->value = ft_strdup(words[0]);
                
                // Create new tokens for additional words
                t_token *last = current;
                for (int i = 1; words[i]; i++)
                {
                    t_token *new_token = create_token(TOKEN_WORD, 
                                                     ft_strdup(words[i]), 1);
                    if (!new_token)
                        break;
                    
                    // Insert new token after last
                    new_token->next = last->next;
                    last->next = new_token;
                    last = new_token;
                }
                
                // Update next pointer to continue after all new tokens
                next = last->next;
                free_env_array(words);
            }
            else
            {
                // Just update the value for quoted tokens or unchanged tokens
                free(current->value);
                current->value = expanded;
            }
        }
        
        prev = current;
        current = next;
    }
    
    return (head);
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
