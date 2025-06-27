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
	else
		res = 0;
	return (free(var), res);
}

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