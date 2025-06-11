/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2025/06/11 00:29:49 by mshariar         ###   ########.fr       */
=======
/*   Updated: 2025/06/02 04:53:52 by my42             ###   ########.fr       */
>>>>>>> main
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
<<<<<<< HEAD
 * Check if option is valid for pwd command
 * Only -L and -P are valid options (which we ignore)
 */
static int	is_valid_pwd_option(char *option)
{
    if (!option || option[0] != '-' || option[1] == '\0')
        return (0);
    if (ft_strcmp(option, "-L") == 0 || ft_strcmp(option, "-P") == 0)
        return (1);
    return (0);
}

/**
 * Handle pwd arguments validation
 * Returns 0 if arguments are valid, 1 otherwise
 */
static int	handle_pwd_args(t_cmd *cmd)
{
    if (!cmd->args[1])
        return (0);
    
    if (cmd->args[1][0] == '-' && cmd->args[1][1] != '\0')
    {
        if (!is_valid_pwd_option(cmd->args[1]))
        {
            display_error(ERROR_PWD, cmd->args[1], "invalid option");
            return (1);
        }
    }
    
    if (cmd->args[2] != NULL)
    {
        display_error(ERROR_PWD, NULL, "too many arguments");
        return (1);
    }
    
    return (0);
}

/**
 * Try to get PWD from environment if getcwd fails
 * This handles the case where the current directory was deleted
 */
static char	*get_fallback_pwd(t_shell *shell)
=======
 * Try to get PWD from environment if getcwd fails
 * This handles the case where the current directory was deleted
 */
static char *get_fallback_pwd(t_shell *shell)
>>>>>>> main
{
    if (!shell)
        return (NULL);
    
    return (get_env_value(shell->env, "PWD"));
}

/**
<<<<<<< HEAD
 * Handle pwd command when getcwd fails
 */
static int	handle_pwd_fallback(t_shell *shell)
{
    char	*pwd;
    
    pwd = get_fallback_pwd(shell);
    if (pwd)
    {
        ft_putendl_fd(pwd, STDOUT_FILENO);
        free(pwd);
        return (0);
    }
    
    display_error(ERROR_PWD, NULL, strerror(errno));
    return (1);
}

/**
=======
>>>>>>> main
 * Built-in pwd command
 * Prints the current working directory
 */
int	builtin_pwd(t_shell *shell, t_cmd *cmd)
{
    char	cwd[PATH_MAX];
<<<<<<< HEAD
=======
    char    *pwd;
>>>>>>> main
    
    if (!cmd || !cmd->args)
        return (1);
        
<<<<<<< HEAD
    if (handle_pwd_args(cmd))
        return (1);
=======
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
>>>>>>> main

    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putendl_fd(cwd, STDOUT_FILENO);
        return (0);
    }
    else
<<<<<<< HEAD
        return (handle_pwd_fallback(shell));
}
=======
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
>>>>>>> main
