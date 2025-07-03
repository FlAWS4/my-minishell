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

/**
 * Checks if the variable is an environment variable
 * @data: Pointer to the shell data structure
 * @var: The variable to check
 *
 * This function checks if the variable starts with '$' and is a valid
 * environment variable. It returns 1 if it exists, 0 if it does not,
 * and -1 on error (e.g., memory allocation failure).
 */

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

/**
 * Expands the value of the environment variable and joins it with the args
 * @data: Pointer to the shell data structure
 * @args: The original arguments string
 * @dollar: The variable to expand (e.g., "$VAR")
 *
 * This function expands the value of the environment variable and replaces
 * it in the args string. It returns the new string or NULL on error.
 */

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

/**
 * Removes the dollar sign from the beginning of the args string
 * @args: The original arguments string
 *
 * This function finds the first occurrence of a dollar sign in the args string,
 * removes it, and returns the substring before the dollar sign. If there is no
 * dollar sign at the beginning, it returns NULL.
 */

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

/**
 * Checks if the variable exists in the environment and expands it if necessary
 * @data: Pointer to the shell data structure
 * @args: The original arguments string
 *
 * This function checks if the variable (e.g., "$VAR") exists in the environment.
 * If it does, it expands it and returns the new string. If it does not exist,
 * it removes the dollar sign and returns the modified string.
 */

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

/**
 * Adds the variable to the result string
 * @result: Pointer to the result string
 * @var: The variable to add
 *
 * This function appends the variable to the result string. If the result string
 * is NULL, it duplicates the variable. If it is not NULL, it joins the variable
 * with the existing result string. Returns 0 on success, -1 on error.
 */

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
