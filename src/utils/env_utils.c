/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:56 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/13 21:26:16 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env *create_env_node(char *key, char *value)
{
	t_env *new_node;

	new_node = malloc(sizeof(t_env));
	if (!new_node)
		return (NULL);
	new_node->key = strdup(key);
	if (!new_node->key)
    {
        free(new_node);
        return (NULL);
    }
    new_node->value = strdup(value);
    if (!new_node->value)
    {
        free(new_node->key);
        free(new_node);
        return (NULL);
    }  
    new_node->next = NULL;
    return (new_node);
}

void add_env_var(t_env **env_list, t_env *new_node)
{
	t_env *current;

	if (!*env_list)
	{
		*env_list = new_node;
		return ;
	}
	current = *env_list;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void split_env_string(char *str, char **key, char **value)
{
	int i;
	int len;

	i = 0;
	while(str[i] && str[i] != '=')
		i++;
	*key = malloc(i + 1);
	if (!*key)
		return ;
	len = i;
	i = 0;
	while (i < len)
	{
		(*key)[i] = str[i];
		i++;
	}
	(*key)[i] = '\0';
	if (str[len] == '=')
		*value = ft_strdup(&str[len + 1]);
	else
		*value = ft_strdup("");
}

t_env *init_env(char **envp)
{
	t_env *env_list;
	char *key;
	char *value;
	int i;

	env_list = NULL;
	i = 0;
	while (envp[i])
	{
		split_env_string(envp[i], &key, &value);
		add_env_var(&env_list, create_env_node(key, value));
		free(key);
		free(value);
		i++;
	}
	return (env_list);
}

char *get_env_value(t_env *env_list, const char *key)
{
	t_env *current;

	current = env_list;
	while (current)
	{
		if (strcmp(current->key, key) == 0)
			return (current->value);
		current = current->next;
	}
	return (NULL);
}


/*
t_env *init_env(char **envp)
{
	t_env *env_list;
	char *key;
	char *value;
	int i;
	int j;
	
	env_list = NULL;
	i = 0;
	while (envp[i])
	{
		j = 0;
		while (envp[i][j] && envp[i][j] != '=')
			j++;
		key = malloc(j + 1);
		if (!key)
			return (NULL);
		j = 0;
		while (envp[i][j] && envp[i][j] != '=')
		{
			key[j] = envp[i][j];
			j++;
		}
		key[j] = '\0';
		value = ft_strdup(envp[i] + j + 1);
		add_env_var(&env_list, create_env_node(key, value));
		i++;
	}
	return (env_list);
}
*