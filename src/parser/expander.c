/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 21:34:06 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 18:35:29 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	expand_exit_status(char **result, char *var)
{
	int		res;
	char	*status;

	if (var[1] == '?')
	{
		status = ft_itoa(g_exit_status);
		if (!status)
			return (-1);
		res = add_text(result, status);
		free(status);
	}
	else
		res = add_text(result, var + 2);
	return (res);
}

int	join_quoted_str(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (token->type == WORD && token->next && token->next->type == WORD
			&& (token->double_quote || token->single_quote)
			&& (token->next->double_quote || token->next->single_quote)
			&& !token->space_after && !token->next->space_before)
		{
			str = ft_strjoin(token->value, token->next->value);
			if (!str)
				return (1);
			free(token->value);
			token->value = str;
			remove_useless_token(tokens, token->next);
		}
		else
			token = token->next;
	}
	return (0);
}

int	join_no_space(t_token **tokens)
{
	t_token	*token;
	char	*str;

	token = *tokens;
	while (token && token->next && token->next->type != T_EOF)
	{
		if (token->type == WORD && token->next->type == WORD
			&& !token->space_after && !token->next->space_before)
		{
			str = ft_strjoin(token->value, token->next->value);
			if (!str)
				return (1);
			free(token->value);
			token->value = str;
			if (token->next->single_quote || token->next->double_quote)
				token->quoted_outside = 1;
			remove_useless_token(tokens, token->next);
			continue ;
		}
		token = token->next;
	}
	return (0);
}

static int	check_token_value(t_shell *data, t_token **tokens)
{
	int	result;

	result = expantion(data, *tokens);
	if (result != 0)
		return (result);
	return (0);
}

int	expand_token(t_shell *data)
{
	t_token	*token;
	t_token	*next_token;
	int		result;

	token = data->tokens;
	while (token && token->type != T_EOF)
	{
		next_token = token->next;
		result = check_token_value(data, &token);
		if (result < 0)
			return (1);
		if (result > 0)
		{
			while (token->next && token->next != next_token)
				token = token->next;
		}
		token = next_token;
	}
	return (0);
}
void	init_for_norm(int *i, int *start)
{
	(*i) = 0;
	(*start) = 0;
}

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
static void	insert_token_bis(t_token *current, t_token *new_token)
{
	new_token->next = current->next;
	new_token->previous = current;
	if (current->next)
		current->next->previous = new_token;
	current->next = new_token;
}

static int	insert_token(t_token *current, char **words)
{
	t_token	*new_token;
	t_token	*token;
	int		i;

	token = current;
	current->ar = 1;
	i = 1;
	while (words[i])
	{
		new_token = create_token(WORD, words[i]);
		if (!new_token)
			return (-1);
		new_token->space_before = 1;
		token->space_after = 1;
		insert_token_bis(token, new_token);
		token = new_token;
		i++;
	}
	return (0);
}

int	split_tokens(t_token *current, char *str)
{
	char	**words;
	int		word_count;

	word_count = count_words_split(str, ' ');
	if (word_count <= 1)
	{
		current->value = ft_strdup(str);
		if (!current->value)
			return (-1);
		return (0);
	}
	words = ft_split(str, ' ');
	if (!words)
		return (-1);
	current->value = ft_strdup(words[0]);
	if (!current->value)
		return (free_array(words), -1);
	if (insert_token(current, words) == -1)
		return (free_array(words), -1);
	return (free_array(words), 1);
}
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
