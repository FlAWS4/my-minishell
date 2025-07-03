/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   check_for_expand.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:03:32 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:05:46 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * expand_exit_status - Expands the exit status variable
 * @result: Pointer to the result string to append to
 * @var: The variable containing the exit status
 *
 * This function appends the exit status to the result string.
 * Returns 0 on success, -1 on failure.
 */

static char	*extract_variable(char *value, int *i)
{
	char	*var;
	int		len;
	int		start;

	start = *i;
	(*i)++;
	if (value[*i] && value[*i] == '?')
	{
		(*i)++;
		var = ft_strdup("$?");
		if (!var)
			return (NULL);
		return (var);
	}
	while (value[*i] && (ft_isalnum(value[*i]) || value[*i] == '_'))
		(*i)++;
	len = *i - start;
	var = ft_substr(value, start, len);
	if (!var)
		return (NULL);
	return (var);
}

/**
 * expand_exit_status - Expands the exit status variable
 * @result: Pointer to the result string to append to
 * @var: The variable containing the exit status
 *
 * This function appends the exit status to the result string.
 * Returns 0 on success, -1 on failure.
 */

int	add_variable(char **result, char *value, int start, int end)
{
	char	*var;
	int		i;

	if (start >= end)
		return (0);
	var = ft_substr(value, start, end - start);
	if (!var)
		return (-1);
	i = add_text(result, var);
	free(var);
	return (i);
}
/**
 * expand_exit_status - Expands the exit status variable
 * @result: Pointer to the result string to append to
 * @var: The variable containing the exit status
 *
 * This function appends the exit status to the result string.
 * Returns 0 on success, -1 on failure.
 */

int	check_variable(t_shell *data, char **result, char *value, int *i)
{
	char	*var;
	char	*expanded;
	int		res;

	var = extract_variable(value, i);
	if (!var)
		return (-1);
	if (var[1] == '\0')
		res = add_text(result, "$");
	else if (var[1] && (var[1] == '?' || ft_isdigit(var[1])))
		res = expand_exit_status(result, var);
	else if (is_env_variable(data, var))
	{
		expanded = expand_value(data->env, var);
		if (!expanded)
			return (free(var), -1);
		res = add_text(result, expanded);
		free(expanded);
	}
	else if (ft_strcmp(var + 1, "PATH") == 0 && \
	data->default_path && !data->path_was_unset)
		res = add_text(result, data->default_path);
	else
		res = 0;
	return (free(var), res);
}

/**
 * is_valid_string - Checks if a string is a valid variable name
 * @str: The string to check
 * @len: The length of the string
 *
 * This function checks if the string contains only alphanumeric characters or
 * underscores. Returns 1 if valid, 0 otherwise.
 */

static int	is_valid_string(char *str, int len)
{
	int	i;

	i = 0;
	while (i < len)
	{
		if (!(ft_isalnum(str[i]) || str[i] == '_'))
			return (0);
		i++;
	}
	return (1);
}

/**
 * potential_expand - Checks if the next characters in args can be expanded
 * @args: The string to check for potential expansion
 * @i: Pointer to the current index in args
 *
 * This function checks if the next characters in args can be expanded as a
 * variable name. If it finds a valid variable name, it updates the index and
 * returns the length of the variable name. If it finds a '?', it skips it.
 * Returns 0 if no valid variable name is found.
 */

int	potential_expand(char *args, int *i)
{
	int	len;
	int	start;

	if (args[*i + 1] == '?')
	{
		*i = *i + 2;
		return (2);
	}
	(*i)++;
	start = *i;
	while (args[*i] && (ft_isalnum(args[*i]) || args[*i] == '_'))
		(*i)++;
	len = *i - start;
	if (len == 0)
		return (0);
	return (is_valid_string(args + start, len));
}
