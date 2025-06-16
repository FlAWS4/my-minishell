/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/16 20:51:44 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Join two strings and free the first one
 */
char	*ft_strjoin_free(char *s1, char *s2)
{
    char	*result;

    result = ft_strjoin(s1, s2);
    free(s1);
    return (result);
}

/**
 * Free all memory allocated during expansion
 */
void	free_expansion_parts(char *var_name, char *var_value, char **parts)
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
char	*get_var_name(char *str)
{
    int		i;
    char	*name;

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
    return (NULL);
}

/**
 * Handle quoting in variable expansion
 */
static int	is_quoted_var(char *str, int pos)
{
    int	i;
    int	in_single_quote;

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
 * Prepare parts for variable expansion
 */
static int	prepare_var_parts(char *str, int i, char **parts, char *var_name)
{
    parts[0] = ft_substr(str, 0, i - 1);
    parts[1] = ft_strdup(&str[i + ft_strlen(var_name)]);
    if (!parts[0] || !parts[1])
        return (0);
    return (1);
}

/**
 * Create expanded result string
 */
static char	*join_expanded_parts(char **parts, char *var_value)
{
    char	*result;
    char	*temp;

    result = ft_strjoin(parts[0], var_value);
    if (!result)
        return (NULL);
    temp = result;
    result = ft_strjoin(result, parts[1]);
    if (!result)
    {
        free(temp);
        return (NULL);
    }
    free(temp);
    return (result);
}

/**
 * Check if variable name is valid
 */
static int	is_valid_var_char(char c)
{
    return (c == '?' || ft_isalnum(c) || c == '_');
}

/**
 * Expand a single variable
 */
char	*expand_one_var(t_shell *shell, char *str, int *i)
{
    char	*var_name;
    char	*var_value;
    char	*parts[2];
    char	*result;

    parts[0] = NULL;
    parts[1] = NULL;
    (*i)++;
    if (!str[*i] || !is_valid_var_char(str[*i]))
        return (str);
    var_name = get_var_name(&str[*i]);
    if (!var_name)
        return (str);
    var_value = get_var_value(shell, var_name);
    if (!var_value)
        var_value = ft_strdup("");
    if (!prepare_var_parts(str, *i, parts, var_name))
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    result = join_expanded_parts(parts, var_value);
    *i = ft_strlen(parts[0]) + ft_strlen(var_value) - 1;
    free_expansion_parts(var_name, var_value, parts);
    if (!result)
        return (str);  
    free(str);
    return (result);
}

/**
 * Expand variables in string
 */
char	*expand_variables(t_shell *shell, char *token)
{
    char	*result;
    int		i;

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
 * Extract and expand a variable in heredoc
 */
static int	extract_and_expand_var(char *content, int *i, char **result,
        t_shell *shell)
{
    int		start;
    int		len;
    char	*var_name;
    char	*var_value;
    char	*temp;

    start = *i + 1;
    if (content[start] == '?')
    {
        var_name = ft_strdup("?");
        *i += 2;
    }
    else
    {
        len = 0;
        while (content[start + len] && (ft_isalnum(content[start + len])
                || content[start + len] == '_'))
            len++;
        var_name = ft_substr(content, start, len);
        *i += len + 1;
    }
    var_value = get_var_value(shell, var_name);
    temp = *result;
    *result = var_value ? ft_strjoin(*result, var_value) : ft_strjoin(*result, "");
    return (free(temp), free(var_name), free(var_value), *result != NULL);
}

/**
 * Append a single character to result string
 */
static int	append_char_to_result(char c, char **result)
{
    char	c_str[2];
    char	*temp;

    c_str[0] = c;
    c_str[1] = '\0';
    temp = *result;
    *result = ft_strjoin(*result, c_str);
    free(temp);
    return (*result != NULL);
}

/**
 * Special handling for heredoc variable expansion
 */
char	*expand_heredoc_content(t_shell *shell, char *content)
{
    char	*result;
    int		i;

    result = ft_strdup("");
    if (!content || !result)
        return (NULL);
    i = 0;
    while (content[i])
    {
        if (content[i] == '$' && content[i + 1] && (ft_isalnum(content[i + 1])
                || content[i + 1] == '?' || content[i + 1] == '_'))
        {
            if (!extract_and_expand_var(content, &i, &result, shell))
            {
                free(result);
                return (NULL);
            }
        }
        else
        {
            if (!append_char_to_result(content[i], &result))
                return (NULL);
            i++;
        }
    }
    return (result);
}

/**
 * Check if heredoc should expand variables based on delimiter
 * (true if delimiter has no quotes)
 */
int	should_expand_heredoc(char *delimiter)
{
    int	i;

    if (!delimiter)
        return (1);
    i = 0;
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
char	*process_heredoc_content(t_shell *shell, char *content, char *delimiter)
{
    if (!should_expand_heredoc(delimiter))
        return (ft_strdup(content));
    return (expand_heredoc_content(shell, content));
}
