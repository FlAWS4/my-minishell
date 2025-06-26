/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   create_expander.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:00:43 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:13:41 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	**copy_var_value(char **tab)
{
	char	**copy;
	int		i;

	if (!tab || !*tab)
		return (NULL);
	i = 0;
	while (tab[i])
		i++;
	copy = malloc(sizeof(char *) * (i + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (tab[i])
	{
		copy[i] = copy_value(tab[i]);
		if (!copy[i])
		{
			free_array(copy);
			return (NULL);
		}
		i++;
	}
	copy[i] = NULL;
	return (copy);
}

char	**copy_var_name(char **tab)
{
	char	**copy;
	int		i;

	if (!tab || !*tab)
		return (NULL);
	i = 0;
	while (tab[i])
		i++;
	copy = malloc(sizeof(char *) * (i + 1));
	if (!copy)
		return (NULL);
	i = 0;
	while (tab[i])
	{
		copy[i] = copy_name(tab[i]);
		if (!copy[i])
		{
			free_array(copy);
			return (NULL);
		}
		i++;
	}
	copy[i] = NULL;
	return (copy);
}

char	*copy_name(char *str)
{
	int		i;
	char	*env_name;

	if (!str)
		return (NULL);
	i = find_equal(str);
	if (i < 0)
	{
		env_name = ft_strdup(str);
		if (!env_name)
			return (NULL);
		return (env_name);
	}
	env_name = ft_substr(str, 0, i);
	if (!env_name)
		return (NULL);
	return (env_name);
}

char	*copy_value(char *str)
{
	char	*env_value;
	int		i;
	int		start;
	int		total;
	int		len;

	if (!str)
		return (NULL);
	i = find_equal(str);
	if (i < 0)
	{
		env_value = ft_strdup("");
		if (!env_value)
			return (NULL);
		return (env_value);
	}
	start = i + 1;
	total = ft_strlen(str);
	len = total - start;
	env_value = ft_substr(str, start, len);
	if (!env_value)
		return (NULL);
	return (env_value);
}

char	*remove_and_replace(char *str, char *expanded)
{
	char	*dollar_pos;
	char	*result;
	char	*before_dollar;
	int		i;

	dollar_pos = NULL;
	dollar_pos = ft_strchr(str, '$');
	if (!dollar_pos)
		return (NULL);
	i = dollar_pos - str;
	before_dollar = ft_substr(str, 0, i);
	if (!before_dollar)
		return (NULL);
	result = ft_strjoin(before_dollar, expanded);
	if (!result)
		return (free(before_dollar), free(expanded), NULL);
	return (free(before_dollar), free(expanded), result);
}
