/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:53:52 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Try to get PWD from environment if getcwd fails
 * This handles the case where the current directory was deleted
 */
static char *get_fallback_pwd(t_shell *shell)
{
    if (!shell)
        return (NULL);
    
    return (get_env_value(shell->env, "PWD"));
}

/**
 * Built-in pwd command
 * Prints the current working directory
 */
int	builtin_pwd(t_shell *shell, t_cmd *cmd)
{
    char	cwd[PATH_MAX];
    char    *pwd;
    
    if (!cmd || !cmd->args)
        return (1);
        
    if (cmd->args[1] != NULL)
    {
        if (cmd->args[1][0] == '-' && cmd->args[1][1] != '\0')
        {
            // Only -L and -P are valid options for pwd, but we ignore them
            if (ft_strcmp(cmd->args[1], "-L") != 0 && ft_strcmp(cmd->args[1], "-P") != 0)
            {
                print_error("pwd", "invalid option");
                return (1);
            }
        }
        else if (cmd->args[2] != NULL) // Allow a single option but not multiple args
        {
            print_error("pwd", "too many arguments");
            return (1);
        }
    }

    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putendl_fd(cwd, STDOUT_FILENO);
        return (0);
    }
    else
    {
        // Try to use PWD from environment as fallback
        pwd = get_fallback_pwd(shell);
        if (pwd)
        {
            ft_putendl_fd(pwd, STDOUT_FILENO);
            return (0);
        }
        
        // If all else fails, print error with specific errno message
        ft_putstr_fd("pwd: ", STDERR_FILENO);
        ft_putstr_fd(strerror(errno), STDERR_FILENO);
        ft_putstr_fd("\n", STDERR_FILENO);
        return (1);
    }
}