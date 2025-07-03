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

/**
 * expand_single_variable - Expands a single variable in the input string
 * @data: The shell data structure containing environment variables
 * @input: The input string potentially containing variables to expand
 * @str: Pointer to the string where the result will be stored
 *
 * This function iterates through the input string, looking for dollar signs
 * indicating variables. It expands these variables and constructs a new string.
 * Returns 0 on success, -1 on failure.
 */

int	expand_single_variable(t_shell *data, char *input, char **str)
{
	char	*result;
	int		i;
	int		start;

	init_parsing_indices(&i, &start);
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

/**
 * multiple_expand - Expands multiple variables in a token's value
 * @data: The shell data structure containing environment variables
 * @tokens: The token containing the value to expand
 * @args: Pointer to the string where the result will be stored
 *
 * This function iterates through the token's value, looking for dollar signs
 * indicating variables. It expands these variables and constructs a new string.
 * Returns 0 on success, -1 on failure.
 * This function is designed to handle cases where multiple variables
 * may be present in the token's value, allowing for complex expansions.
 */

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
