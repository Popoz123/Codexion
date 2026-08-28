/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:20:57 by pzavada           #+#    #+#             */
/*   Updated: 2026/08/06 13:18:18 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// 1,number_of_coders
// 2,time_to_burnout
// 3,time_to_compile
// 4,time_to_debug
// 5,time_to_refactor
// 6,number_of_compiles_required
// 7,dongle_cooldown
// 8,scheduler

#include "codexion.h"

void		*threads_init(void *coder);
pthread_t	*launch_sim(t_coder *coders, t_dongle *dongles, int num);
pthread_t	*f_clean(t_coder *cds, int num, pthread_t *th, t_dongle *don);

int	main(int ac, char **av)
{
	int			*mem;
	t_dongle	*dongles;
	t_coder		*coders;
	t_data		*start;
	pthread_t	*threads;

	mem = NULL;
	dongles = NULL;
	coders = NULL;
	start = NULL;
	threads = NULL;
	mem = malloc(sizeof(int) * 8);
	if (mem && parser(ac, av, mem))
		dongles = dongle_arr(mem[6], mem[0]);
	if (dongles)
		start = data_init(mem[0]);
	if (start)
		coders = coder_arr(mem, dongles, start);
	if (coders)
		threads = launch_sim(coders, dongles, mem[0]);
	clean_exit(mem, dongles, coders, threads);
	return (0);
}

pthread_t	*launch_sim(t_coder *coders, t_dongle *dongles, int num)
{
	int			i;
	pthread_t	*threads;

	i = 0;
	(void)dongles;
	threads = malloc(sizeof(pthread_t) * (num + 1));
	if (!threads)
		return (NULL);
	while (i < num)
	{
		if (pthread_create(&threads[i], NULL, &threads_init, &coders[i]) != 0)
			return (f_clean(coders, i, threads, dongles));
		i++;
	}
	if (pthread_create(&threads[num], NULL, &wait_m, coders) != 0)
		return (f_clean(coders, i, threads, dongles));
	pthread_mutex_lock(&coders->init->lock);
	coders->init->started = 1;
	coders->init->start_t = get_time_ms();
	pthread_cond_broadcast(&coders->init->cond);
	pthread_mutex_unlock(&coders->init->lock);
	return (threads);
}

void	*threads_init(void *arg)
{
	t_coder				*coder;
	unsigned long long	start;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->init->lock);
	while (!coder->init->started)
		pthread_cond_wait(&coder->init->cond, &coder->init->lock);
	start = coder->init->start_t;
	pthread_mutex_unlock(&coder->init->lock);
	pthread_mutex_lock(&coder->c_mut);
	coder->last_comp = start;
	pthread_mutex_unlock(&coder->c_mut);
	if (coder->id % 2 == 0)
		usleep(100);
	dongle_manager(coder);
	return (NULL);
}

pthread_t	*f_clean(t_coder *cds, int num, pthread_t *th, t_dongle *don)
{
	int	i;

	i = 0;
	pthread_mutex_lock(&cds->init->lock);
	cds->init->stop = 1;
	cds->init->started = 1;
	pthread_mutex_unlock(&cds->init->lock);
	pthread_cond_broadcast(&cds->init->cond);
	while (++i < num)
		pthread_cond_broadcast(&(don + (i - 1))->d_cond);
	i = 0;
	while (++i < num)
		pthread_join(th[i - 1], NULL);
	free(th);
	return (NULL);
}
