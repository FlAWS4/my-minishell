/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   init_expand.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/26 00:55:18 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/26 01:06:41 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	init_for_norm(int *i, int *start)
{
	(*i) = 0;
	(*start) = 0;
}

int	count_dollars(char *args)
{
	int	i;
	int	j;

	if (!args)
		return (0);
	i = 0;
	j = 0;
	while (args[i])
	{
		if (args[i] == '$' && args[i + 1] && args[i + 1] != '\0'
			&& (ft_isalnum(args[i + 1]) || args[i + 1] == '_'
				|| args[i + 1] == '?'))
		{
			if (potential_expand(args, &i))
				j++;
		}
		else
			i++;
	}
	return (j);
}

int	expand_arguments(t_shell *data, t_token *tokens, char **str)
{
	char	*result;
	int		i;
	int		start;

	init_for_norm(&i, &start);
	result = ft_strdup("");
	if (!result)
		return (-1);
	while (tokens->value[i])
	{
		if (tokens->value[i] == '$')
		{
			if (add_variable(&result, tokens->value, start, i) == -1)
				return (-1);
			if (check_variable(data, &result, tokens->value, &i) == -1)
				return (-1);
			start = i;
		}
		else
			i++;
	}
	if (add_variable(&result, tokens->value, start, i) == -1)
		return (free(result), -1);
	*str = result;
	return (0);
}

char	*expand_variables(t_shell *data, char *input, t_redir *redir)
{
	int		dollars_nb;
	char	*str;

	dollars_nb = count_dollars(input);
	if (!dollars_nb || ft_strlen(input) < 1 || redir->quoted
		|| redir->quoted_outside)
	{
		str = ft_strdup(input);
		if (!str)
			return (NULL);
		return (str);
	}
	if (multiple_expand_bis(data, input, &str) == -1)
		return (NULL);
	return (str);
}

int	expand_status(char *args)
{
	int	i;
	int	j;

	i = 0;
	j = 0;
	while (args[i])
	{
		if (args[i] == '$' && args[i + 1] && args[i + 1] == '?')
			j++;
		i++;
	}
	return (j);
}
