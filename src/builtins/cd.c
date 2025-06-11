/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:33:17 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/11 03:09:44 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "minishell.h"

/**
 * Update PWD environment variable
 */
static void	update_pwd(t_shell *shell, char *cwd, int *pwd_exists)
{
    t_env	*env_node;

    env_node = shell->env;
    while (env_node)
    {
        if (ft_strcmp(env_node->key, "PWD") == 0)
        {
            free(env_node->value);
            env_node->value = ft_strdup(cwd);
            *pwd_exists = 1;
            break ;
        }
        env_node = env_node->next;
    }
}

/**
 * Update OLDPWD environment variable
 */
static void	update_oldpwd(t_shell *shell, char *old_pwd, int *oldpwd_exists)
{
    t_env	*env_node;

    env_node = shell->env;
    while (env_node)
    {
        if (ft_strcmp(env_node->key, "OLDPWD") == 0)
        {
            free(env_node->value);
            env_node->value = ft_strdup(old_pwd);
            *oldpwd_exists = 1;
            break ;
        }
        env_node = env_node->next;
    }
}

/**
 * Update PWD and OLDPWD environment variables
 * Creates them if they don't exist
 */
static void	update_pwd_vars(t_shell *shell, char *old_pwd)
{
    char	cwd[PATH_MAX];
    int		pwd_exists;
    int		oldpwd_exists;

    pwd_exists = 0;
    oldpwd_exists = 0;
    if (!getcwd(cwd, sizeof(cwd)))
        return ;
    update_pwd(shell, cwd, &pwd_exists);
    update_oldpwd(shell, old_pwd, &oldpwd_exists);
    if (!pwd_exists)
        set_env_var(&shell->env, "PWD", cwd);
    if (!oldpwd_exists)
        set_env_var(&shell->env, "OLDPWD", old_pwd);
}

/**
 * Get previous directory and check if it exists
 */
static int	get_prev_dir(t_shell *shell, char **prev_dir, char *old_pwd)
{
    *prev_dir = get_env_value(shell->env, "OLDPWD");
    if (!*prev_dir)
    {
        display_error(ERROR_CD, NULL, "OLDPWD not set");
        return (1);
    }
    if (getcwd(old_pwd, PATH_MAX) == NULL)
    {
        display_error(ERROR_CD, NULL, "Could not get current directory");
        free(*prev_dir);
        return (1);
    }
    return (0);
}

/**
 * Change to previous directory when "cd -" is used
 */
static int	cd_to_previous(t_shell *shell)
{
    char	*prev_dir;
    char	old_pwd[PATH_MAX];

    if (get_prev_dir(shell, &prev_dir, old_pwd))
        return (1);
    if (chdir(prev_dir) != 0)
    {
        display_error(ERROR_CD, prev_dir, strerror(errno));
        free(prev_dir);
        return (1);
    }
    ft_putendl_fd(prev_dir, STDOUT_FILENO);
    update_pwd_vars(shell, old_pwd);
    free(prev_dir);
    return (0);
}

/**
 * Change to home directory
 */
static int	cd_to_home(t_shell *shell)
{
    char	*home_dir;
    char	old_pwd[PATH_MAX];

    home_dir = getenv("HOME");
    if (!home_dir)
    {
        display_error(ERROR_CD, "cd", "HOME not set");
        return (1);
    }
    if (getcwd(old_pwd, PATH_MAX) == NULL)
    {
        display_error(ERROR_CD, "cd", "Could not get current directory");
        return (1);
    }
    if (chdir(home_dir) != 0)
    {
        display_error(ERROR_CD, home_dir, strerror(errno));
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
 * Expand tilde for home directory in path
 */
static char	*expand_tilde_home(char *path, char *home)
{
    char	*result;

    if (path[1] == '\0')
    {
        result = ft_strdup(home);
        free(home);
        return (result);
    }
    if (path[1] == '/')
    {
        result = ft_strjoin(home, path + 1);
        free(home);
        return (result);
    }
    free(home);
    return (ft_strdup(path));
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
    return (expand_tilde_home(path, home));
}

/**
 * Prepare for changing directory
 */
static int	prepare_cd(t_shell *shell, char *dir, char *old_pwd, 
                    char **expanded_path)
{
    if (getcwd(old_pwd, PATH_MAX) == NULL)
    {
        display_error(ERROR_CD, "cd", "Could not get current directory");
        return (1);
    }
    *expanded_path = expand_tilde(shell, dir);
    if (!*expanded_path)
    {
        display_error(ERROR_MEMORY, "cd", "Memory allocation failed");
        return (1);
    }
    return (0);
}

/**
 * Change to specified directory
 */
static int	cd_to_dir(t_shell *shell, char *dir)
{
    char	old_pwd[PATH_MAX];
    char	*expanded_path;
    int		ret;

    if (prepare_cd(shell, dir, old_pwd, &expanded_path))
        return (1);
    ret = chdir(expanded_path);
    if (ret != 0)
    {
        if (errno == ENOENT)
            display_error(ERROR_CD, dir, "No such file or directory");
        else if (errno == EACCES)
            display_error(ERROR_CD, dir, "Permission denied");
        else if (errno == ENOTDIR)
            display_error(ERROR_CD, dir, "Not a directory");
        else
            display_error(ERROR_CD, dir, strerror(errno));
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
        display_error(ERROR_CD, "cd", "too many arguments");
        return (1);
    }
    
    return (cd_to_dir(shell, cmd->args[1]));
}
