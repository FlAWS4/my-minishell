/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_gc.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 15:48:25 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 15:48:25 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void	*gc_malloc(t_gc **gc_list, size_t size, int fatal,
	void (*free_array)(void *))
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (NULL);
	}
	if (gc_add(gc_list, ptr, fatal, free_array))
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			free(ptr);
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (free(ptr), NULL);
	}
	return (ptr);
}

int	gc_add(t_gc **gc_list, void *ptr, int fatal,
	void (*free_array)(void *))
{
	t_gc	*new_node;

	if (!ptr)
		return (1);
	new_node = malloc(sizeof(t_gc));
	if (!new_node)
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (fatal == GC_FATAL)
		{
			gc_free_all(gc_list);
			exit(EXIT_FAILURE);
		}
		return (1);
	}
	new_node->data = ptr;
	new_node->flag = fatal;
	new_node->free_array = free_array;
	new_node->next = *gc_list;
	*gc_list = new_node;
	return (0);
}

void	gc_free_all(t_gc **gc_list)
{
	t_gc	*current_node;
	t_gc	*next_node;

	if (!gc_list || !*gc_list)
		return ;
	current_node = *gc_list;
	while (current_node)
	{
		next_node = current_node->next;
		if (current_node->free_array)
			current_node->free_array(current_node->data);
		else
			free(current_node->data);
		free(current_node);
		current_node = next_node;
	}
	*gc_list = NULL;
}

char	*gc_substr(t_gc **gc, const char *s, unsigned int start, size_t len)
{
	size_t	i;
	size_t	y;
	char	*sub;

	i = 0;
	if (!s)
		return (NULL);
	if (start >= ft_strlen(s))
		return (gc_strdup(gc, ""));
	y = ft_strlen(s + start);
	if (y < len)
		len = y;
	sub = gc_malloc(gc, len + 1, GC_SOFT, NULL);
	if (!sub)
		return (NULL);
	while (i < len && s[start])
	{
		sub[i] = s[start];
		i++;
		start++;
	}
	sub[i] = '\0';
	return (sub);
}