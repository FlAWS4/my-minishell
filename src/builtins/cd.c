/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:33:17 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:34:33 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Update PWD and OLDPWD environment variables
 */
static void	update_pwd_vars(t_shell *shell, char *old_pwd)
{
    t_env	*env_node;
    char	cwd[1024];

    if (getcwd(cwd, sizeof(cwd)))
    {
        env_node = shell->env;
        while (env_node)
        {
            if (ft_strcmp(env_node->key, "PWD") == 0)
            {
                free(env_node->value);
                env_node->value = ft_strdup(cwd);
            }
            else if (ft_strcmp(env_node->key, "OLDPWD") == 0)
            {
                free(env_node->value);
                env_node->value = ft_strdup(old_pwd);
            }
            env_node = env_node->next;
        }
    }
}

/**
 * Change to home directory
 */
static int	cd_to_home(t_shell *shell)
{
    char	*home_dir;
    char	old_pwd[1024];
    char	error_msg[1024];

    home_dir = get_env_value(shell->env, "HOME");
    if (!home_dir)
    {
        print_error("cd", "HOME not set");
        return (1);
    }
    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
        return (1);
    if (chdir(home_dir) != 0)
    {
        ft_strlcpy(error_msg, home_dir, sizeof(error_msg));
        ft_strlcat(error_msg, ": cannot change directory", sizeof(error_msg));
        print_error("cd", error_msg);
        return (1);
    }
    update_pwd_vars(shell, old_pwd);
    return (0);
}

/**
 * Expand tilde in path to home directory
 */
static char	*expand_tilde(t_shell *shell, char *path)
{
    char	*home;
    
    if (!path || path[0] != '~')
        return (ft_strdup(path));
    
    home = get_env_value(shell->env, "HOME");
    if (!home)
        return (ft_strdup(path));
    
    if (path[1] == '\0')  // Just "~"
        return (ft_strdup(home));
    
    if (path[1] == '/')   // "~/something"
        return (ft_strjoin(home, path + 1));
    
    return (ft_strdup(path));
}

/**
 * Change to specified directory
 */
static int	cd_to_dir(t_shell *shell, char *dir)
{
    char	old_pwd[1024];
    char	*expanded_path;
    char	error_msg[1024];
    int		ret;

    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
        return (1);
    expanded_path = expand_tilde(shell, dir);
    ret = chdir(expanded_path);
    if (ret != 0)
    {
        ft_strlcpy(error_msg, dir, sizeof(error_msg));
        if (access(expanded_path, F_OK) != 0)
            ft_strlcat(error_msg, ": no such file or directory", sizeof(error_msg));
        else if (access(expanded_path, R_OK) != 0)
            ft_strlcat(error_msg, ": permission denied", sizeof(error_msg));
        else
            ft_strlcat(error_msg, ": not a directory", sizeof(error_msg));       
        print_error("cd", error_msg);
        free(expanded_path);
        return (1);
    }
    update_pwd_vars(shell, old_pwd);
    free(expanded_path);
    return (0);
}

/**
 * Built-in cd command
 */
int	builtin_cd(t_shell *shell, t_cmd *cmd)
{
    int		args_count;
    int		i;

    args_count = 0;
    i = 0;
    while (cmd->args[i])
    {
        args_count++;
        i++;
    }
    if (args_count == 1)
        return (cd_to_home(shell));
    else if (args_count > 2)
    {
        print_error("cd", "too many arguments");
        return (1);
    }
    return (cd_to_dir(shell, cmd->args[1]));
}
