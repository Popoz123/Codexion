/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   monitor.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/04 15:06:41 by pzavada           #+#    #+#             */
/*   Updated: 2026/07/29 16:26:52 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

//check burnout and comp 1ms, when reached broadcast and set stop = 1
// fix the fact that the program ends if one coder finishes
void	end_sim(t_coder *cd, int flag, int id);
void	monitor(t_coder	*cds);
void	*wait_m(void *arg);
int		comp_ctr(t_coder *temp, int *comps_done, t_coder *cds);
int		is_stopped(t_data *init);

void	monitor(t_coder	*cds)
{
	int		i;
	int		comps_done;
	t_coder	*temp;

	while (1)
	{
		i = 0;
		comps_done = 0;
		while (i < cds->init->c_num)
		{
			temp = (cds + i);
			pthread_mutex_lock(&temp->c_mut);
			if (!comp_ctr(temp, &comps_done, cds))
				return ;
			pthread_mutex_unlock(&temp->c_mut);
			i++;
		}
		if (comps_done == cds->init->c_num)
		{
			end_sim(cds, 0, 0);
			return ;
		}
		usleep(1000);
	}
}

int	comp_ctr(t_coder *temp, int *comps_done, t_coder *cds)
{
	if (temp->compiles <= 0)
		(*comps_done)++;
	if (temp->last_comp + temp->burnout_time < get_time_ms())
	{
		end_sim(cds, 1, temp->id);
		pthread_mutex_unlock(&temp->c_mut);
		return (0);
	}
	return (1);
}

void	end_sim(t_coder *cd, int flag, int id)
{
	int					i;
	unsigned long long	time;

	i = 0;
	pthread_mutex_lock(&cd->init->lock);
	cd->init->stop = 1;
	time = cd->init->start_t;
	pthread_mutex_unlock(&cd->init->lock);
	if (flag)
	{
		pthread_mutex_lock(&cd->init->print_lock);
		printf("%llu %i burned out\n",
			get_time_ms() - time, (cd + (id - 1))->id);
		pthread_mutex_unlock(&(cd + (id - 1))->init->print_lock);
	}
	while (i < cd->init->c_num)
	{
		pthread_mutex_lock(&(cd + i)->l_don->mutex);
		pthread_cond_broadcast(&(cd + i)->l_don->d_cond);
		pthread_mutex_unlock(&(cd + i)->l_don->mutex);
		i++;
	}
}

void	*wait_m(void *arg)
{
	t_coder	*coder;

	coder = (t_coder *)arg;
	pthread_mutex_lock(&coder->init->lock);
	while (!coder->init->started)
		pthread_cond_wait(&coder->init->cond, &coder->init->lock);
	pthread_mutex_unlock(&coder->init->lock);
	monitor(coder);
	return (NULL);
}

int	is_stopped(t_data *init)
{
	int	val;

	pthread_mutex_lock(&init->lock);
	val = init->stop;
	pthread_mutex_unlock(&init->lock);
	return (val);
}
