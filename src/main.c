/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mshariar <mshariar@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 02:37:15 by mshariar          #+#    #+#             */
/*   Updated: 2025/05/13 00:45:21 by mshariar         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int g_signal;

int main(int agrc, char **argv, char **envp)
{
    t_shell shell;
    char	*input;
	
	(void)agrc; //temporary
	(void)argv; //temporary
	g_signal = 0;
	shell.env = init_env(envp);
	shell.cmd = NULL;
	shell.exit_status = 0;
	setup_signals();
	while(1)
	{
		input = readline("minishell> ");
		if(!input)
		{
			printf("exit\n");
			break;
		}
		if (*input)
			add_history(input);
			/*
			
			to do:
			
			*/
		free(input);
		// to do: free all the allocated memory(maybe a garbage collector)
	}
	return (0);
}
