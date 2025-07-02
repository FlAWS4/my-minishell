/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_utils.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/26 21:51:14 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 00:52:18 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	is_env_variable(t_shell *data, char *var)
{
	char	**env_copy;
	int		i;

	env_copy = copy_var_name(data->env);
	if (!env_copy)
		return (-1);
	i = 0;
	var++;
	while (env_copy[i])
	{
		if (!ft_strcmp(env_copy[i], var))
		{
			free_array(env_copy);
			return (1);
		}
		i++;
	}
	free_array(env_copy);
	return (0);
}

char	*expand_and_join(t_shell *data, char *args, char *dollar)
{
	char	*expanded;
	char	*result;

	result = NULL;
	expanded = expand_value(data->env, dollar);
	if (!expanded)
		return (NULL);
	result = remove_and_replace(args, expanded);
	if (!result)
		return (NULL);
	return (result);
}

char	*remove_dollar(char *args)
{
	char	*result;
	char	*dollars_pos;
	int		i;

	dollars_pos = ft_strchr(args, '$');
	i = dollars_pos - args;
	if (!i)
		return (NULL);
	result = ft_substr(args, 0, i);
	return (result);
}

char	*is_exist(t_shell *data, char *args)
{
	char	*str;
	char	*dollar;
	int		result;

	str = NULL;
	dollar = ft_strchr(args, '$');
	result = is_env_variable(data, dollar);
	if (result)
	{
		str = expand_and_join(data, args, dollar);
		if (!str)
			return (NULL);
		return (str);
	}
	else if (result == 0)
		str = remove_dollar(args);
	else if (result == -1)
		return (NULL);
	return (str);
}

int	add_text(char **result, char *var)
{
	char	*tmp;

	if (!*result)
	{
		*result = ft_strdup(var);
		if (!*result)
			return (-1);
		return (0);
	}
	tmp = ft_strjoin(*result, var);
	if (!tmp)
		return (-1);
	free(*result);
	*result = tmp;
	return (0);
}
