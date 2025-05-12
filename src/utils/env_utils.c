/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:34:56 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/12 22:48:24 by mshariar         ###   ########.fr       */
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
