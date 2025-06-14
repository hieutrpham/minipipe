/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: trupham <trupham@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 16:22:33 by trupham           #+#    #+#             */
/*   Updated: 2025/06/12 17:27:33 by trupham          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/pipex.h"

static void	prepare_fd(int *pips, int fd, int idx)
{
	if (idx == 1)
	{
		close(pips[0]);
		if (dup2(fd, STDIN_FILENO) < 0)
			exit_error("dup");
		if (dup2(pips[1], STDOUT_FILENO) < 0)
			exit_error("dup");
		close(fd);
		close(pips[1]);
	}
	else
	{
		close(pips[1]);
		if (dup2(fd, STDOUT_FILENO) < 0)
			exit_error("dup");
		if (dup2(pips[0], STDIN_FILENO) < 0)
			exit_error("dup");
		close(fd);
		close(pips[0]);
	}
}

static pid_t	run_process1(char **av, char **env, int *pips)
{
	int		fd;
	pid_t	child1;
	char	**cmd;
	char	*bin;

	if (!check_infile(av[1]))
		return (1);
	child1 = fork();
	if (child1 == -1)
		exit_error("fork");
	if (child1 == 0)
	{
		fd = open(av[1], O_RDONLY);
		if (fd < 0)
			exit_error(av[1]);
		prepare_fd(pips, fd, 1);
		cmd = build_exec_argv(av[2]);
		bin = get_binary_path(cmd[0], env);
		if (!bin)
			exit_free(cmd, bin, 127);
		execve(bin, cmd, env);
		exit_free(cmd, bin, 1);
	}
	return (child1);
}

static pid_t	run_process2(char **av, char **env, int *pips)
{
	int		fd;
	pid_t	child2;
	char	**cmd;
	char	*bin;

	child2 = fork();
	if (child2 == -1)
		exit_error("fork");
	if (child2 == 0)
	{
		fd = open(av[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			exit_error(av[4]);
		prepare_fd(pips, fd, 0);
		cmd = build_exec_argv(av[3]);
		bin = get_binary_path(cmd[0], env);
		if (!bin)
			exit_free(cmd, bin, 127);
		execve(bin, cmd, env);
		exit_free(cmd, bin, 1);
	}
	return (child2);
}

int	main(int ac, char **av, char **env)
{
	int		status;
	int		pips[2];
	pid_t	child1;
	pid_t	child2;

	if (ac != 5)
		exit_error("usage");
	if (pipe(pips) < 0)
		exit_error("pipe");
	child1 = run_process1(av, env, pips);
	child2 = run_process2(av, env, pips);
	close(pips[0]);
	close(pips[1]);
	waitpid(child1, &status, 0);
	waitpid(child2, &status, 0);
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (1);
}
