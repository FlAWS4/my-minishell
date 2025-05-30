/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/19 21:28:44 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in env command
 */
int	builtin_env(t_shell *shell)
{
    t_env	*env;

    env = shell->env;
    while (env)
    {
        ft_putstr_fd(env->key, 1);
        ft_putstr_fd("=", 1);
        ft_putstr_fd(env->value, 1);
        ft_putstr_fd("\n", 1);
        env = env->next;
    }
    return (0);
}
