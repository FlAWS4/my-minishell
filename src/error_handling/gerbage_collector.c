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

/**
 * allocate_managed_memory - Allocates memory and 
 * registers it with the memory manager
 * @memory_manager: Pointer to the memory management linked list
 * @size: Number of bytes to allocate
 * @cleanup_mode: Whether failure is fatal (MEM_ERROR_FATAL) or 
 * can be recovered from (MEM_ERROR_RECOVERABLE)
 * @free_func: Custom function to free this 
 * memory if needed (NULL for simple free)
 *
 * This function allocates memory and adds it to the memory management system.
 * If allocation fails, it either exits the program (fatal mode) or returns NULL.
 *
 * Returns: Pointer to allocated memory, or NULL on failure in non-fatal mode
 */
void	*allocate_managed_memory(t_memory_node **memory_manager, \
	size_t size, int cleanup_mode, void (*free_func)(void *))
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (cleanup_mode == MEM_ERROR_FATAL)
		{
			release_all_memory(memory_manager);
			exit(EXIT_FAILURE);
		}
		return (NULL);
	}
	if (track_memory_allocation(memory_manager, ptr, cleanup_mode, free_func))
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (cleanup_mode == MEM_ERROR_FATAL)
		{
			free(ptr);
			release_all_memory(memory_manager);
			exit(EXIT_FAILURE);
		}
		return (free(ptr), NULL);
	}
	return (ptr);
}

/**
 * track_memory_allocation - Adds allocated memory 
 * to the memory management system
 * @memory_manager: Pointer to the memory management linked list
 * @ptr: Pointer to the allocated memory to track
 * @cleanup_mode: Whether failure is fatal (MEM_ERROR_FATAL) or 
 * can be recovered from (MEM_ERROR_RECOVERABLE)
 * @free_func: Custom function to free this memory 
 * if needed (NULL for simple free)
 *
 * This function creates a new node in the memory manager linked list to track
 * the provided pointer for later cleanup.
 *
 * Returns: 0 on success, 1 on failure
 */
int	track_memory_allocation(t_memory_node **memory_manager, void *ptr,
	int cleanup_mode, void (*free_func)(void *))
{
	t_memory_node	*new_node;

	if (!ptr)
		return (1);
	new_node = malloc(sizeof(t_memory_node));
	if (!new_node)
	{
		ft_putstr_fd(ERROR_MALLOC, STDERR_FILENO);
		if (cleanup_mode == MEM_ERROR_FATAL)
		{
			release_all_memory(memory_manager);
			exit(EXIT_FAILURE);
		}
		return (1);
	}
	new_node->ptr = ptr;
	new_node->cleanup_mode = cleanup_mode;
	new_node->free_func = free_func;
	new_node->next = *memory_manager;
	*memory_manager = new_node;
	return (0);
}

/**
 * release_all_memory - Frees all memory tracked by the memory manager
 * @memory_manager: Pointer to the memory management linked list
 *
 * This function walks through the entire linked list of tracked memory,
 * freeing each allocation using either its custom free function or the
 * standard free, then freeing the tracking node itself.
 */
void	release_all_memory(t_memory_node **memory_manager)
{
	t_memory_node	*current_node;
	t_memory_node	*next_node;

	if (!memory_manager || !*memory_manager)
		return ;
	current_node = *memory_manager;
	while (current_node)
	{
		next_node = current_node->next;
		if (current_node->free_func)
			current_node->free_func(current_node->ptr);
		else
			free(current_node->ptr);
		free(current_node);
		current_node = next_node;
	}
	*memory_manager = NULL;
}

char	*join_managed_strings(t_memory_node **memory_manager,
	const char *s1, const char *s2)
{
	size_t	i;
	size_t	y;
	char	*result;

	i = 0;
	y = 0;
	if (!s1 || !s2)
		return (NULL);
	result = allocate_managed_memory(memory_manager, \
		ft_strlen(s1) + ft_strlen(s2) + 1, MEM_ERROR_RECOVERABLE, NULL);
	if (!result)
		return (NULL);
	while (s1[i])
	{
		result[i] = s1[i];
		i++;
	}
	while (s2[y])
	{
		result[i] = s2[y];
		i++;
		y++;
	}
	result[i] = '\0';
	return (result);
}

char	*create_managed_string_copy(t_memory_node **memory_manager,
	const char *s1)
{
	char	*dest;
	size_t	len;

	if (!s1)
		return (NULL);
	len = ft_strlen(s1) + 1;
	dest = allocate_managed_memory(memory_manager, len, \
		MEM_ERROR_RECOVERABLE, NULL);
	if (!dest)
		return (NULL);
	ft_memcpy(dest, s1, len);
	return (dest);
}
