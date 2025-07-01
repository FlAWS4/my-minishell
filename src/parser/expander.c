/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:09:52 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	find_equal(char *str)
{
	int	i;

	i = 0;
	if (!str)
		return (-1);
	while (str[i])
	{
		if (str[i] == '=')
			return (i);
		i++;
	}
	return (-1);
}

char	*expand_value(char **env, char *dollar)
{
	int		i;
	char	**copy_env;
	char	**copy_value;
	char	*str;

	i = 0;
	dollar++;
	copy_env = copy_var_name(env);
	copy_value = copy_var_value(env);
	if (!copy_env || !copy_value)
		return (free_array(copy_value), free_array(copy_env), NULL);
	while (copy_env[i])
	{
		if (!ft_strcmp(copy_env[i], dollar))
		{
			str = ft_strdup(copy_value[i]);
			if (!str)
				return (free_array(copy_env), free_array(copy_value), NULL);
			return (free_array(copy_env), free_array(copy_value), str);
		}
		i++;
	}
	return (free_array(copy_env), free_array(copy_value), NULL);
}

static char	*remove_useless_space(char *str)
{
	char	*new_line;
	int		i;
	int		j;

	new_line = malloc(ft_strlen(str) + 1);
	if (!new_line)
		return (NULL);
	i = 0;
	j = 0;
	while (str[i] && is_whitespace(str[i]))
		i++;
	while (str[i])
	{
		while (str[i] && !is_whitespace(str[i]))
			new_line[j++] = str[i++];
		while (str[i] && is_whitespace(str[i]))
			i++;
		if (str[i])
			new_line[j++] = ' ';
	}
	new_line[j] = '\0';
	free(str);
	return (new_line);
}

int	expantion(t_shell *data, t_token *tokens)
{
	char	*str;
	int		dollars;

	if (tokens->single_quote
		|| (tokens->previous && tokens->previous->type == HEREDOC))
		return (0);
	dollars = count_dollars(tokens->value);
	if (dollars <= 0)
		return (0);
	if (expand_arguments(data, tokens, &str) == -1)
		return (-1);
	free(tokens->value);
	if (str && !tokens->double_quote)
	{
		str = remove_useless_space(str);
		if (!str)
			return (-1);
		if (split_tokens(tokens, str) == -1)
			return (free(str), -1);
		return (free(str), 1);
	}
	tokens->value = str;
	return (1);
}
