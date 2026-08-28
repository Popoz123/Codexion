/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/11 17:24:07 by pzavada           #+#    #+#             */
/*   Updated: 2026/07/29 16:48:11 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void				mut_print(t_coder *c, char *str);
void				clean_exit(int *mem, t_dongle *d, t_coder *c, pthread_t *t);
unsigned long long	get_time_ms(void);
int					convert(char *st);
int					parser(int ac, char **av, int *res);

int	convert(char *st)
{
	int			i;
	long long	res;

	i = 0;
	res = 0;
	if (strlen(st) >= 11)
		return (-1);
	while (st[i])
	{
		if (!(st[i] >= '0' && st[i] <= '9'))
			return (-1);
		res = res * 10 + (st[i] - '0');
		if (res > INT_MAX)
			return (-1);
		i++;
	}
	return ((int)res);
}

int	parser(int ac, char **av, int *res)
{
	int	i;
	int	x;

	i = 1;
	if (ac != 9)
		return (0);
	while (i <= 7)
	{
		x = convert(av[i]);
		if (x == -1)
			return (0);
		res[i - 1] = x;
		i++;
	}
	if (res[0] == 0 || res[0] > 200)
		return (0);
	if (strcmp(av[8], "fifo") == 0)
		res[7] = 1;
	else if (strcmp(av[8], "edf") == 0)
		res[7] = 2;
	else
		return (0);
	return (1);
}

void	clean_exit(int *mem, t_dongle *d, t_coder *coders, pthread_t *thr)
{
	int	i;

	i = -1;
	if (d || thr)
	{
		pthread_join(thr[mem[0]], NULL);
		while (++i < mem[0])
		{
			pthread_join(thr[i], NULL);
			pthread_mutex_destroy(&d[i].mutex);
			pthread_cond_destroy(&d[i].d_cond);
			free(d[i].qeue);
		}
		free(d);
		free(thr);
	}
	free(mem);
	if (coders && coders->init)
	{
		pthread_mutex_destroy(&coders->init->lock);
		pthread_mutex_destroy(&coders->init->print_lock);
		pthread_cond_destroy(&coders->init->cond);
		free(coders->init);
		free(coders);
	}
}

unsigned long long	get_time_ms(void)
{
	struct timeval		x;

	gettimeofday(&x, NULL);
	return ((x.tv_sec * 1000) + (x.tv_usec / 1000));
}

void	mut_print(t_coder *c, char *str)
{
	unsigned long long	time;

	if (is_stopped(c->init))
		return ;
	pthread_mutex_lock(&c->init->print_lock);
	if (is_stopped(c->init))
	{
		pthread_mutex_unlock(&c->init->print_lock);
		return ;
	}
	time = get_time_ms() - c->init->start_t;
	printf("%llu %i %s\n", time, c->id, str);
	pthread_mutex_unlock(&c->init->print_lock);
}
