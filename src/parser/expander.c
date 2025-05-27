/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 22:00:01 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Expand a single variable
 */
char	*expand_one_var(t_shell *shell, char *str, int *i)
{
    char	*var_name;
    char	*var_value;
    char	*parts[2];
    char	*result;

    (*i)++;
    if (!str[*i] || !(str[*i] == '?' || ft_isalnum(str[*i]) || str[*i] == '_'))
        return (str);
    var_name = get_var_name(&str[*i]);
    if (!var_name)
        return (str);
    var_value = get_var_value(shell, var_name);
    parts[0] = ft_substr(str, 0, *i - 1);
    parts[1] = ft_strdup(&str[*i + ft_strlen(var_name)]);
    if (!var_value || !parts[0] || !parts[1])
    {
        free_expansion_parts(var_name, var_value, parts);
        return (str);
    }
    result = ft_strjoin(parts[0], var_value);
    result = ft_strjoin_free(result, parts[1]);
    *i = *i - 1 + ft_strlen(var_value);
    free_expansion_parts(var_name, var_value, parts);
    free(str);
    return (result);
}

/**
 * Free memory allocated during expansion
 */
void	free_expansion_parts(char *name, char *value, char **parts)
{
    free(name);
    free(value);
    free(parts[0]);
    free(parts[1]);
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
    if (!result)
        return (NULL);
    while (result[i])
    {
        if (result[i] == '\'' && !in_double_quote)
            in_single_quote = !in_single_quote;
        else if (result[i] == '\"' && !in_single_quote)
            in_double_quote = !in_double_quote;
        else if (result[i] == '$' && !in_single_quote && result[i + 1] &&
                (ft_isalnum(result[i + 1]) || result[i + 1] == '?' ||
                result[i + 1] == '_'))
            result = expand_one_var(shell, result, &i);
        i++;
    }
    return (result);
}
