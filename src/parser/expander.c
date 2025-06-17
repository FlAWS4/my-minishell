/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 01:21:21 by mshariar         ###   ########.fr       */
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
    int     i;
    char    *name;

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
    char *result;

    if (!name)
        return (NULL);
    if (ft_strcmp(name, "?") == 0)
    {
        result = ft_itoa(shell->exit_status);
        if (!result)
            handle_memory_error(shell, "variable expansion");
        return (result);
    }
    value = get_env_value(shell->env, name);
    if (value)
    {
        result = ft_strdup(value);
        if (!result)
            handle_memory_error(shell, "variable expansion");
        free(value);
        return (result);
    }
    return (ft_strdup("")); // Return empty string for undefined variables
}

/**
 * Handle quoting in variable expansion
 */
int is_quoted_var(char *str, int pos)
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
 * Expand a single variable
 */
char *expand_one_var(t_shell *shell, char *str, int *i)
{
    char *var_name;
    char *var_value;
    char *parts[2];
    char *result;

    parts[0] = NULL;
    parts[1] = NULL;
    (*i)++;
    if (!str[*i] || !(str[*i] == '?' || ft_isalnum(str[*i]) || str[*i] == '_'))
        return (str);
    var_name = get_var_name(&str[*i]);
    if (!var_name)
        return (str);
    var_value = get_var_value(shell, var_name);
    parts[0] = ft_substr(str, 0, *i - 1);
    parts[1] = ft_strdup(&str[*i + ft_strlen(var_name)]);
    if (!parts[0] || !parts[1])
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    result = ft_strjoin(parts[0], var_value ? var_value : "");
    if (!result)
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    *i = ft_strlen(parts[0]) + (var_value ? ft_strlen(var_value) : 0) - 1;
    free(var_name);
    free(var_value);
    var_name = result;
    result = ft_strjoin(result, parts[1]);
    free(var_name);
    free(parts[0]);
    free(parts[1]);
    if (!result)
        return (str);
    free(str);
    return (result);
}

/**
 * Expand variables in string
 */
char *expand_variables(t_shell *shell, char *token)
{
    char *result;
    int   i;

    if (!token)
        return (NULL);
    result = ft_strdup(token);
    if (!result)
        return (NULL);
    i = 0;
    while (result[i])
    {
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
 * Extract variable name from content starting at position
 */
char *extract_var_name(char *content, int *i)
{
    int   start;
    int   len;
    char  *var_name;

    start = *i + 1;
    if (content[start] == '?')
    {
        *i += 2;
        return (ft_strdup("?"));
    }
    len = 0;
    while (content[start + len] && (ft_isalnum(content[start + len])
            || content[start + len] == '_'))
        len++;
    var_name = ft_substr(content, start, len);
    *i += len + 1;
    return (var_name);
}

/**
 * Expand heredoc content with variables
 */
char *expand_heredoc_content(t_shell *shell, char *content)
{
    char *result;
    char *temp;
    int   i;
    char  *var_name;
    char  *var_value;

    result = ft_strdup("");
    if (!content || !result)
        return (NULL);
    i = 0;
    while (content[i])
    {
        if (content[i] == '$' && content[i + 1] && (ft_isalnum(content[i + 1])
                || content[i + 1] == '?' || content[i + 1] == '_'))
        {
            var_name = extract_var_name(content, &i);
            var_value = get_var_value(shell, var_name);
            temp = result;
            result = ft_strjoin(result, var_value ? var_value : "");
            free(temp);
            free(var_name);
            free(var_value);
            if (!result)
                return (NULL);
        }
        else
        {
            temp = result;
            result = ft_strjoin_free(result, ft_substr(content, i, 1));
            if (!result)
                return (NULL);
            i++;
        }
    }
    return (result);
}

/**
 * Process heredoc content for variable expansion
 */
char *process_heredoc_content(t_shell *shell, char *content, char *delimiter)
{
    int i;

    if (!delimiter)
        return (expand_heredoc_content(shell, content));
    i = 0;
    while (delimiter[i])
    {
        if (delimiter[i] == '\'' || delimiter[i] == '\"')
            return (ft_strdup(content));
        i++;
    }
    return (expand_heredoc_content(shell, content));
}
