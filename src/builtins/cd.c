/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/27 18:46:32 by hchowdhu          #+#    #+#             */
/*   Updated: 2025/06/27 18:46:32 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Display an error message for cd command failures
 */
static void	print_cd_error(char *target)
{
    error("cd", target, strerror(errno));
    g_exit_status = 1;
}


/**
 * Retrieves HOME or OLDPWD environment variable
 */
static char	*get_home_or_oldpwd(t_shell *shell, int is_oldpwd)
{
    char	*value;
    char	*var_name;

    if (is_oldpwd)
        var_name = "OLDPWD";
    else
        var_name = "HOME";
    value = get_env_value(shell, var_name);
    if (!value)
        error("cd", var_name, "not set");
    return (value);
}

/**
 * Determines the target directory for cd
 */
static char	*get_cd_destination(t_shell *shell, char *arg)
{
    char	*target;

    if (!arg || arg[0] == '\0')
        return (get_home_or_oldpwd(shell, 0));
    if (ft_strcmp(arg, "-") == 0)
    {
        target = get_home_or_oldpwd(shell, 1);
        if (target)
            ft_putendl_fd(target, STDOUT_FILENO);
        return (target);
    }
    return (arg);
}
static int update_pwd_vars_with_logical_path(t_shell *shell, char *target)
{
    char	*oldpwd;
    char	*logical_path;
    struct stat path_stat;
    char    *full_path;
    int     is_symlink;

    oldpwd = get_env_value(shell, "PWD");
    is_symlink = 0;
    
    // Construct full path for symlink check BEFORE chdir
    if (target[0] != '/' && oldpwd)
    {
        if (oldpwd[ft_strlen(oldpwd) - 1] == '/')
            full_path = ft_strjoin(oldpwd, target);
        else
        {
            char *temp = ft_strjoin(oldpwd, "/");
            full_path = ft_strjoin(temp, target);
            free(temp);
        }
        
        // Check if target is a symlink
        if (full_path && lstat(full_path, &path_stat) == 0)
            is_symlink = S_ISLNK(path_stat.st_mode);
    }
    else if (lstat(target, &path_stat) == 0)
        is_symlink = S_ISLNK(path_stat.st_mode);
    
    if (is_symlink)
    {
        // For symlinks, construct logical path instead of physical
        if (target[0] == '/')
            logical_path = ft_strdup(target);
        else if (oldpwd)
        {
            if (oldpwd[ft_strlen(oldpwd) - 1] == '/')
                logical_path = ft_strjoin(oldpwd, target);
            else
            {
                char *temp = ft_strjoin(oldpwd, "/");
                logical_path = ft_strjoin(temp, target);
                free(temp);
            }
        }
        else
            logical_path = getcwd(NULL, 0);
            
        free(full_path);
    }
    else
        logical_path = getcwd(NULL, 0);
    
    // Rest of the function remains the same
    if (!logical_path)
    {
        error("getcwd", NULL, strerror(errno));
        g_exit_status = 1;
        return (1);
    }
    
    if (oldpwd)
        update_env(shell, "OLDPWD", oldpwd);
    update_env(shell, "PWD", logical_path);
    free(logical_path);
    return (0);
}

int	builtin_cd(t_shell *shell, t_command *cmd)
{
    char	*target;
    int		count_args;

    count_args = 0;
    if (!shell || !cmd || !cmd->args)
        return (error("cd", NULL, "internal error"), g_exit_status = 1, 1);
    while (cmd->args[count_args])
        count_args++;
    if (count_args > 2)
        return (error("cd", NULL, "too many arguments"), g_exit_status = 1, 1);
    target = get_cd_destination(shell, cmd->args[1]);
    if (!target)
        return (1);
    if (chdir(target) == -1)
        return (print_cd_error(target), 1);
    if (target && ft_strcmp(target, "//") == 0)
    {
        char *oldpwd = get_env_value(shell, "PWD");
        if (oldpwd)
            update_env(shell, "OLDPWD", oldpwd);
        update_env(shell, "PWD", "//");
        return (0);
    }
    return (update_pwd_vars_with_logical_path(shell, target));
}