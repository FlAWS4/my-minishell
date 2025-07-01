/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   multiple_expand.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 01:07:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:08:26 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	multiple_expand_bis(t_shell *data, char *input, char **str)
{
	char	*result;
	int		i;
	int		start;

	init_for_norm(&i, &start);
	result = ft_strdup("");
	if (!result)
		return (-1);
	while (input[i])
	{
		if (input[i] == '$')
		{
			if (add_variable(&result, input, start, i) == -1)
				return (free(result), -1);
			if (check_variable(data, &result, input, &i) == -1)
				return (free(result), -1);
			start = i;
		}
		else
			i++;
	}
	if (add_variable(&result, input, start, i) == -1)
		return (free(result), -1);
	*str = result;
	return (0);
}

int	expand_arguments_bis(t_shell *data, char *input, char **str)
{
	char	*tmp;

	tmp = NULL;
	tmp = is_exist(data, input);
	if (!tmp)
	{
		*str = ft_strdup("\n");
		if (!*str)
			return (free(tmp), -1);
	}
	*str = ft_strdup(tmp);
	if (!*str)
		return (free(tmp), -1);
	return (free(tmp), 0);
}

int	multiple_expand(t_shell *data, t_token *tokens, char **args)
{
	char	*result;
	int		i;
	int		start;

	result = NULL;
	i = 0;
	start = 0;
	while (tokens->value[i])
	{
		if (tokens->value[i] == '$')
		{
			if (add_variable(&result, tokens->value, start, i) == -1)
				return (free(result), -1);
			if (check_variable(data, &result, tokens->value, &i) == -1)
				return (free(result), -1);
			start = i;
		}
		else
			i++;
	}
	if (add_variable(&result, tokens->value, start, i) == -1)
		return (free(result), -1);
	*args = result;
	return (0);
}
