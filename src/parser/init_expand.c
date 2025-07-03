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

/**
 * init_parsing_indices - Initializes the indices for parsing
 * @i: Pointer to the index variable to initialize
 * @start: Pointer to the start index variable to initialize
 *
 * This function sets both indices to 0, preparing them for parsing operations.
 */

void	init_parsing_indices(int *i, int *start)
{
	(*i) = 0;
	(*start) = 0;
}

/**
 * count_dollars - Counts the number of dollar signs in a string
 * @args: The string to search for dollar signs
 *
 * Returns the count of dollar signs in the string.
 */

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

/**
 * expand_arguments - Expands the arguments in a token
 * @data: The shell data structure containing environment variables
 * @tokens: The token to expand
 * @str: Pointer to the string to store the expanded result
 *
 * This function processes the token's value, expanding any variables found.
 * Returns 0 on success, -1 on failure.
 */

int	expand_arguments(t_shell *data, t_token *tokens, char **str)
{
	char	*result;
	int		i;
	int		start;

	init_parsing_indices(&i, &start);
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

/**
 * expand_variables - Expands variables in the input string
 * @data: The shell data structure containing environment variables
 * @input: The input string to expand
 * @redir: The redirection structure containing quoting information
 *
 * This function checks for dollar signs in the input string and expands them
 * if necessary, returning a new string with the expanded values.
 * Returns NULL on failure.
 */

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
	if (expand_single_variable(data, input, &str) == -1)
		return (NULL);
	return (str);
}

/**
 * expand_status - Counts the number of exit status expansions in a string
 * @args: The string to check for exit status expansions
 *
 * This function counts how many times the exit status variable ($?) appears
 * in the string and returns that count.
 * Returns the count of exit status expansions.
 */

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
