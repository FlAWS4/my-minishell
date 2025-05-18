/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cd.c                                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hchowdhu <hchowdhu@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:33:17 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/18 20:24:56 by hchowdhu         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Helper to update environment variables (simple version)
static void	update_env(char *name, char *value)
{
    setenv(name, value, 1);
}

int	ft_cd(char **args)
{
    char	*path;
    char	cwd[4096];
    char	oldpwd[4096];

    if (getcwd(oldpwd, sizeof(oldpwd)) == NULL)
    {
        perror("cd: getcwd");
        return (1);
    }
    // No argument or "cd ~" goes to HOME
    if (!args[1] || strcmp(args[1], "~") == 0)
        path = getenv("HOME");
    else
        path = args[1];

    if (chdir(path) != 0)
    {
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return (1);
    }
    // Update OLDPWD and PWD
    update_env("OLDPWD", oldpwd);
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        update_env("PWD", cwd);
    return (0);
}
