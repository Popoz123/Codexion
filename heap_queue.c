/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_queue.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/17 18:52:56 by pzavada           #+#    #+#             */
/*   Updated: 2026/07/29 16:49:17 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	dongle_manager(t_coder *coder);
void	line(t_coder *coder, t_dongle *dongle,
			struct timespec *dl, unsigned long long lt);
void	insert(t_coder *cd, t_dongle *don);
void	pop(t_coder *cd, t_dongle *don);
void	compile(t_dongle *first, t_dongle *second, t_coder *cd);

void	dongle_manager(t_coder *coder)
{
	struct timespec		dl;
	unsigned long long	line_t;
	t_dongle			*first;
	t_dongle			*second;

	line_t = 0;
	first = coder->r_don;
	second = coder->l_don;
	if (coder->id % 2 == 0)
	{
		first = coder->l_don;
		second = coder->r_don;
	}
	while (!is_stopped(coder->init))
	{
		pthread_mutex_lock(&coder->c_mut);
		coder->q_time = get_time_ms();
		pthread_mutex_unlock(&coder->c_mut);
		line(coder, first, &dl, line_t);
		pthread_mutex_lock(&coder->c_mut);
		coder->q_time = get_time_ms();
		pthread_mutex_unlock(&coder->c_mut);
		line(coder, second, &dl, line_t);
		compile(first, second, coder);
	}
}

void	compile(t_dongle *first, t_dongle *second, t_coder *cd)
{
	int	x;

	x = cd->id;
	mut_print(cd, "is compiling");
	pthread_mutex_lock(&cd->c_mut);
	cd->last_comp = get_time_ms();
	pthread_mutex_unlock(&cd->c_mut);
	if (!c_sleep(cd->comp_time, cd->init))
		return ;
	pthread_mutex_lock(&cd->c_mut);
	cd->compiles--;
	pthread_mutex_unlock(&cd->c_mut);
	signal(first);
	signal(second);
	mut_print(cd, "is debugging");
	if (!c_sleep(cd->debug_time, cd->init))
		return ;
	mut_print(cd, "is refactoring");
	if (!c_sleep(cd->refac_time, cd->init))
		return ;
}

void	line(t_coder *c, t_dongle *d,
			struct timespec *dl, unsigned long long l)
{
	pthread_mutex_lock(&d->mutex);
	insert(c, d);
	l = get_time_ms();
	while (d->flag || d->qeue[0] != c->id || l < d->used_time + d->cooldown)
	{
		if (is_stopped(c->init))
		{
			pthread_mutex_unlock(&d->mutex);
			return ;
		}
		if (!d->flag && d->qeue[0] == c->id && l < d->used_time + d->cooldown)
		{
			dl->tv_nsec = ((d->used_time + d->cooldown) % 1000) * 1000000;
			dl->tv_sec = (d->used_time + d->cooldown) / 1000;
			pthread_cond_timedwait(&d->d_cond, &d->mutex, dl);
		}
		else
			pthread_cond_wait(&d->d_cond, &d->mutex);
		l = get_time_ms();
	}
	pop(c, d);
	d->flag = 1;
	mut_print(c, "has taken a dongle");
	pthread_mutex_unlock(&d->mutex);
}

void	insert(t_coder *cd, t_dongle *don)
{
	int		i;
	t_coder	*all_cds;

	i = don->q_size;
	don->qeue[don->q_size] = cd->id;
	don->q_size++;
	all_cds = cd - (cd->id - 1);
	while (i > 0)
	{
		if (cmp_coders(cd, (all_cds + don->qeue[(i - 1) / 2] - 1)))
		{
			q_swap(don, i, (i - 1) / 2);
			i = (i - 1) / 2;
		}
		else
			break ;
	}
}

void	pop(t_coder *cd, t_dongle *don)
{
	int		i;
	int		left;
	int		best;
	t_coder	*s;

	i = 0;
	s = cd - (cd->id - 1);
	don->q_size--;
	don->qeue[0] = don->qeue[don->q_size];
	if (don->q_size <= 1)
		return ;
	while (2 * i + 1 < don->q_size)
	{
		left = 2 * i + 1;
		best = left;
		if (left + 1 < don->q_size
			&& cmp_coders(s + don->qeue[left + 1] - 1, s + don->qeue[left] - 1))
			best = left + 1;
		if (!cmp_coders(s + don->qeue[best] - 1, s + don->qeue[i] - 1))
			break ;
		q_swap(don, i, best);
		i = best;
	}
}
