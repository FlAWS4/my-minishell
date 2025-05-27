/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pwd.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:54 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/26 23:41:25 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

/**
 * Built-in pwd command
 */
int	builtin_pwd(t_shell *shell, t_cmd *cmd)
{
    char	cwd[1024];
    
    (void)shell;
    if (cmd->args[1] != NULL)
    {
        if (cmd->args[1][0] == '-' && cmd->args[1][1] != '\0')
            print_error("pwd", "invalid option");
        else
            print_error("pwd", "too many arguments");
        return (1);
    }

    if (getcwd(cwd, sizeof(cwd)))
    {
        ft_putendl_fd(cwd, STDOUT_FILENO);
        return (0);
    }
    else
    {
        print_error("pwd", "getcwd() error");
        return (1);
    }
}
