/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: my42 <my42@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:38:46 by mshariar          #+#    #+#             */
/*   Updated: 2025/06/23 04:42:42 by my42             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	duplicate_existing_vars(t_shell *shell, char **new_env, int size)
{
	int	i;

	i = 0;
	while (i < size)
	{
		new_env[i] = gc_strdup(&shell->gc, shell->env[i]);
		if (!new_env[i])
		{
			gc_free_all(&shell->gc);
			return (0);
		}
		i++;
	}
	return (1);
}

int	env_error(const char *msg, t_gc **gc)
{
	if (msg)
		error("env", NULL, msg);
	if (gc && *gc)
		gc_free_all(gc);
	return (1);
}

static int	duplicate_env_vars(char **envp, t_shell *shell)
{
	int	i;

	i = 0;
	while (envp[i])
	{
		shell->env[i] = gc_strdup(&shell->gc, envp[i]);
		if (!shell->env[i])
			return (env_error("failed to init environment\n", &shell->gc));
		i++;
	}
	shell->env[i] = NULL;
	return (1);
}

char	**get_env(char **envp, t_shell *shell)
{
	int	env_vars;

	env_vars = 0;
	if (!envp || !*envp)
	{
		env_error("invalid environment\n", NULL);
		return (NULL);
	}
	while (envp[env_vars])
		env_vars++;
	shell->env = gc_malloc(&shell->gc, sizeof(char *) * (env_vars + 1),
			GC_FATAL, NULL);
	if (!shell->env)
		return (NULL);
	if (!duplicate_env_vars(envp, shell))
		return (NULL);
	return (shell->env);
}

char	**init_env(char **envp, t_shell *shell)
{
	char	**mini_env;

	if (env_has_path(envp))
	{
		shell->default_path = NULL;
		shell->env = get_env(envp, shell);
		return (shell->env);
	}
	shell->default_path = ft_strdup("/usr/local/bin:/usr/bin:/bin");
	mini_env = init_minimal_env();
	if (!mini_env)
	{
		error("env", NULL, "failed to create minimal environment");
		return (NULL);
	}
	shell->env = get_env(mini_env, shell);
	free_array(mini_env);
	if (!shell->env)
		return (NULL);
	return (shell->env);
}
static void	free_tmp_env_vars(char **tmp, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		free(tmp[i]);
		i++;
	}
}

static void	copy_tmp_to_env(char **dest, char **src, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = NULL;
}

char	**init_minimal_env(void)
{
	char	*tmp[3];
	char	*cwd;
	char	**mini_env;

	cwd = getcwd(NULL, 0);
	if (!cwd)
		return (NULL);
	tmp[0] = ft_strjoin("PWD=", cwd);
	tmp[1] = ft_strdup("SHLVL=1");
	tmp[2] = ft_strdup("_=/usr/bin/env");
	free(cwd);
	if (!tmp[0] || !tmp[1] || !tmp[2])
		return (free_tmp_env_vars(tmp, 3), NULL);
	mini_env = malloc(4 * sizeof(char *));
	if (!mini_env)
		return (free_tmp_env_vars(tmp, 3), NULL);
	copy_tmp_to_env(mini_env, tmp, 3);
	return (mini_env);
}
void	update_env(t_shell *shell, char *var, char *new_value)
{
	int		i;
	size_t	var_len;
	char	*new_var;
	char	*tmp;

	i = 0;
	if (!shell || !shell->env || !var || !new_value)
		return ;
	var_len = ft_strlen(var);
	tmp = gc_strjoin(&shell->gc, var, "=");
	if (!tmp)
		return ;
	new_var = gc_strjoin(&shell->gc, tmp, new_value);
	if (!new_var)
		return ;
	while (shell->env[i])
	{
		if (ft_strncmp(shell->env[i], var, var_len) == 0
			&& shell->env[i][var_len] == '=')
			return ((void)(shell->env[i] = new_var));
		i++;
	}
	shell->env = add_env_var(shell, new_var);
}

int	add_to_env(t_shell *shell, char *new_var)
{
	char	**new_env;
	int		size;

	size = 0;
	if (!new_var)
		return (env_error("minishell: invalid variable\n", NULL));
	while (shell->env && shell->env[size])
		size++;
	new_env = gc_malloc(&shell->gc, sizeof(char *) * (size + 2), GC_SOFT, NULL);
	if (!new_env || !duplicate_existing_vars(shell, new_env, size))
		return (1);
	new_env[size] = gc_strdup(&shell->gc, new_var);
	if (!new_env[size])
		return (env_error("minishell: failed to duplicate new variable\n",
				&shell->gc));
	new_env[size + 1] = NULL;
	shell->env = new_env;
	return (0);
}

static char	**duplicate_env(t_shell *shell, int new_size)
{
	char	**new_env;
	int		i;

	i = 0;
	new_env = gc_malloc(&shell->gc, sizeof(char *) * new_size, GC_SOFT, NULL);
	if (!new_env)
		return (NULL);
	while (shell->env[i] && i < new_size - 1)
	{
		new_env[i] = gc_strdup(&shell->gc, shell->env[i]);
		if (!new_env[i])
			return (NULL);
		i++;
	}
	return (new_env);
}

static int	add_new_var(t_shell *shell, char **new_env, char *new_var, int pos)
{
	new_env[pos] = gc_strdup(&shell->gc, new_var);
	if (!new_env[pos])
	{
		env_error("minishell: failed to duplicate new variable\n", &shell->gc);
		return (0);
	}
	new_env[pos + 1] = NULL;
	return (1);
}

char	**add_env_var(t_shell *shell, char *new_var)
{
	char	**new_env;
	int		env_size;

	if (!shell || !shell->env || !new_var)
	{
		env_error("minishell: invalid parameters to add_env_var\n", NULL);
		return (NULL);
	}
	env_size = 0;
	while (shell->env[env_size])
		env_size++;
	new_env = duplicate_env(shell, env_size + 2);
	if (!new_env)
	{
		env_error("minishell: failed to allocate new environment\n",
			&shell->gc);
		return (NULL);
	}
	if (!add_new_var(shell, new_env, new_var, env_size))
		return (NULL);
	return (new_env);
}
int	env_has_path(char **envp)
{
	int	i;

	i = 0;
	if (!envp || !*envp)
		return (0);
	while (envp[i])
	{
		if (ft_strncmp(envp[i], "PATH=", 5) == 0)
			return (1);
		i++;
	}
	return (0);
}

char	*get_env_value(t_shell *shell, const char *var_name)
{
	int		i;
	size_t	var_len;

	i = 0;
	if (!shell || !shell->env || !var_name || !*var_name)
		return (NULL);
	var_len = ft_strlen(var_name);
	while (shell->env[i])
	{
		if (ft_strncmp(shell->env[i], var_name, var_len) == 0
			&& shell->env[i][var_len] == '=')
			return (shell->env[i] + var_len + 1);
		i++;
	}
	return (NULL);
}

int	find_var_pos(char *var_name, t_shell *shell)
{
	int		i;
	size_t	len;

	i = 0;
	if (!shell || !shell->env || !var_name || !*var_name)
		return (-1);
	len = ft_strlen(var_name);
	while (shell->env[i])
	{
		if (!ft_strncmp(shell->env[i], var_name, len)
			&& (shell->env[i][len] == '=' || shell->env[i][len] == '\0'))
			return (i);
		i++;
	}
	return (-1);
}

void	sort_env_for_export(char **env_copy)
{
	int		i;
	int		j;
	char	*temp;

	i = 0;
	if (!env_copy)
		return ;
	while (env_copy[i + 1])
	{
		j = 0;
		while (env_copy[j + 1])
		{
			if (ft_strcmp(env_copy[j], env_copy[j + 1]) > 0)
			{
				temp = env_copy[j];
				env_copy[j] = env_copy[j + 1];
				env_copy[j + 1] = temp;
			}
			j++;
		}
		i++;
	}
}

int	is_valid_identifier(const char *str)
{
	int	i;

	i = 1;
	if (!str || !str[0])
		return (0);
	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (0);
	while (str[i])
	{
		if (str[i] == '=' || (!ft_isalnum(str[i]) && str[i] != '_'))
			return (0);
		i++;
	}
	return (1);
}
int	builtin_env(t_shell *shell, t_command *cmd)
{
	int	i;

	i = 0;
	if (!shell || !cmd)
		return (error("env", NULL, "internal error"), 1);
	if (!shell->env)
		return (error("env", NULL, "environment not found"), 1);
	if (cmd->args[1])
	{
		error("env", NULL, "options not supported");
		return (1);
	}
	while (shell->env[i])
	{
		if (ft_strchr(shell->env[i], '='))
		{
			ft_putstr_fd(shell->env[i], STDOUT_FILENO);
			ft_putchar_fd('\n', STDOUT_FILENO);
		}
		i++;
	}
	return (0);
}
