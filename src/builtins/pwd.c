/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/17 02:05:42 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
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
{
    if (!shell)
        return (NULL);
    return (get_env_value(shell->env, "PWD"));
}

/**
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
 * Built-in pwd command
 * Prints the current working directory
 */
int	builtin_pwd(t_shell *shell, t_cmd *cmd)
{
    char	cwd[PATH_MAX];
    
    if (!cmd || !cmd->args)
        return (1);
    if (handle_pwd_args(cmd))
        return (1);
    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putendl_fd(cwd, STDOUT_FILENO);
        return (0);
    }
    else
        return (handle_pwd_fallback(shell));
}
