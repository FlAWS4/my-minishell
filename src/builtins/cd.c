/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:33:17 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/02 04:26:29 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Update PWD and OLDPWD environment variables
 * Creates them if they don't exist
 */
static void	update_pwd_vars(t_shell *shell, char *old_pwd)
{
    char	cwd[PATH_MAX];
    int     pwd_exists;
    int     oldpwd_exists;

    pwd_exists = 0;
    oldpwd_exists = 0;
    
    if (!getcwd(cwd, sizeof(cwd)))
        return;
        
    // First check if the variables exist
    t_env *env_node = shell->env;
    while (env_node)
    {
        if (ft_strcmp(env_node->key, "PWD") == 0)
        {
            free(env_node->value);
            env_node->value = ft_strdup(cwd);
            pwd_exists = 1;
        }
        else if (ft_strcmp(env_node->key, "OLDPWD") == 0)
        {
            free(env_node->value);
            env_node->value = ft_strdup(old_pwd);
            oldpwd_exists = 1;
        }
        env_node = env_node->next;
    }
    
    // Create variables if they don't exist
    if (!pwd_exists)
        set_env_var(&shell->env, "PWD", cwd);
    if (!oldpwd_exists)
        set_env_var(&shell->env, "OLDPWD", old_pwd);
}

/**
 * Change to previous directory when "cd -" is used
 */
static int	cd_to_previous(t_shell *shell)
{
    char	*prev_dir;
    char	old_pwd[PATH_MAX];
    
    prev_dir = get_env_value(shell->env, "OLDPWD");
    if (!prev_dir)
    {
        print_error("cd", "OLDPWD not set");
        return (1);
    }
    
    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
    {
        print_error("cd", "Could not get current directory");
        return (1);
    }
    
    if (chdir(prev_dir) != 0)
    {
        display_error(ERR_NOT_FOUND, "cd", prev_dir);
        return (1);
    }
    
    // Print the directory we changed to
    ft_putendl_fd(prev_dir, STDOUT_FILENO);
    update_pwd_vars(shell, old_pwd);
    return (0);
}

/**
 * Change to home directory
 */
static int	cd_to_home(t_shell *shell)
{
    char	*home_dir;
    char	old_pwd[PATH_MAX];

    home_dir = get_env_value(shell->env, "HOME");
    if (!home_dir)
    {
        print_error("cd", "HOME not set");
        return (1);
    }
    
    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
    {
        print_error("cd", "Could not get current directory");
        return (1);
    }
    
    if (chdir(home_dir) != 0)
    {
        display_error(ERR_NOT_FOUND, "cd", home_dir);
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
    char    *result;
    
    if (!path || path[0] != '~')
        return (ft_strdup(path));
    
    home = get_env_value(shell->env, "HOME");
    if (!home)
        return (ft_strdup(path));
    
    if (path[1] == '\0')  // Just "~"
        return (ft_strdup(home));
    
    if (path[1] == '/')   // "~/something"
    {
        result = ft_strjoin(home, path + 1);
        return (result);
    }
    
    return (ft_strdup(path));
}

/**
 * Change to specified directory
 */
static int	cd_to_dir(t_shell *shell, char *dir)
{
    char	old_pwd[PATH_MAX];
    char	*expanded_path;
    int		ret;

    if (getcwd(old_pwd, sizeof(old_pwd)) == NULL)
    {
        print_error("cd", "Could not get current directory");
        return (1);
    }
    
    expanded_path = expand_tilde(shell, dir);
    if (!expanded_path)
    {
        print_error("cd", "Memory allocation failed");
        return (1);
    }
    
    ret = chdir(expanded_path);
    if (ret != 0)
    {
        if (access(expanded_path, F_OK) != 0)
            display_error(ERR_NOT_FOUND, "cd", dir);
        else if (access(expanded_path, R_OK) != 0)
            display_error(ERR_PERMISSION, "cd", dir);
        else
            display_error(ERR_NOT_FOUND, "cd", dir);
            
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
    if (!cmd->args[1])
        return (cd_to_home(shell));
        
    if (ft_strcmp(cmd->args[1], "-") == 0)
        return (cd_to_previous(shell));
        
    if (cmd->args[2])
    {
        print_error("cd", "too many arguments");
        return (1);
    }
    
    return (cd_to_dir(shell, cmd->args[1]));
}
