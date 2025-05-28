/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   export_utils.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 20:45:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/28 23:25:51 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Sort environment variables
 */
static void	sort_env_array(t_env **env_array, int count)
{
    int		i;
    int		j;
    t_env	*temp;

    i = 0;
    while (i < count - 1)
    {
        j = 0;
        while (j < count - i - 1)
        {
            if (ft_strcmp(env_array[j]->key, env_array[j + 1]->key) > 0)
            {
                temp = env_array[j];
                env_array[j] = env_array[j + 1];
                env_array[j + 1] = temp;
            }
            j++;
        }
        i++;
    }
}

/**
 * Print a single environment variable in export format
 */
static void	print_env_var(t_env *env)
{
    ft_putstr_fd("declare -x ", 1);
    ft_putstr_fd(env->key, 1);
    if (env->value)
    {
        ft_putstr_fd("=\"", 1);
        ft_putstr_fd(env->value, 1);
        ft_putstr_fd("\"", 1);
    }
    ft_putstr_fd("\n", 1);
}

/**
 * Print environment variables in sorted order
 */
void	print_sorted_env(t_shell *shell)
{
    t_env	**env_array;
    t_env	*curr;
    int		count;
    int		i;

    if (!shell || !shell->env)
        return;
    count = count_env_vars(shell->env);
    env_array = malloc(sizeof(t_env *) * count);
    if (!env_array)
        return;
    i = 0;
    curr = shell->env;
    while (curr)
    {
        env_array[i++] = curr;
        curr = curr->next;
    }
    sort_env_array(env_array, count);
    i = 0;
    while (i < count)
        print_env_var(env_array[i++]);
    free(env_array);
}
