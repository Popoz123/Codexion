/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heap_utils.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/30 15:17:45 by pzavada           #+#    #+#             */
/*   Updated: 2026/08/06 13:06:30 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

void	q_swap(t_dongle *don, int a, int b);
void	signal(t_dongle *first);
void	cmp_lock(t_coder *cd1, t_coder *cd2);
int		cmp_coders(t_coder *cd1, t_coder *cd2);
int		c_sleep(int t, t_data *init);

void	q_swap(t_dongle *don, int a, int b)
{
	int	temp;

	temp = don->qeue[a];
	don->qeue[a] = don->qeue[b];
	don->qeue[b] = temp;
}

int	cmp_coders(t_coder *cd1, t_coder *cd2)
{
	int	res;

	res = 0;
	cmp_lock(cd1, cd2);
	if (cd1->key == 2)
	{
		if (cd1->last_comp == cd2->last_comp)
			res = cd1->id < cd2->id;
		else if (cd1->last_comp < cd2->last_comp)
			res = 1;
	}
	else
	{
		if (cd1->q_time == cd2->q_time)
			res = cd1->id < cd2->id;
		else if (cd1->q_time < cd2->q_time)
			res = 1;
	}
	pthread_mutex_unlock(&cd1->c_mut);
	pthread_mutex_unlock(&cd2->c_mut);
	return (res);
}

void	cmp_lock(t_coder *cd1, t_coder *cd2)
{
	if (cd1->id < cd2->id)
	{
		pthread_mutex_lock(&cd1->c_mut);
		pthread_mutex_lock(&cd2->c_mut);
	}
	else
	{
		pthread_mutex_lock(&cd2->c_mut);
		pthread_mutex_lock(&cd1->c_mut);
	}
}

void	signal(t_dongle *first)
{
	pthread_mutex_lock(&first->mutex);
	first->flag = 0;
	first->used_time = get_time_ms();
	if (first->q_size)
		pthread_cond_broadcast(&first->d_cond);
	pthread_mutex_unlock(&first->mutex);
}

int	c_sleep(int t, t_data *init)
{
	unsigned long long	start;
	unsigned long long	end;

	start = get_time_ms();
	end = start + t;
	while (get_time_ms() < end)
	{
		if (is_stopped(init))
			return (0);
		usleep(100);
	}
	return (1);
}
