/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   struct_init.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/02 16:10:19 by pzavada           #+#    #+#             */
/*   Updated: 2026/07/14 18:06:47 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

t_data		*data_init(int num);
t_dongle	*dongle_arr(int c_down, int num);
t_coder		*coder_arr(int *data, t_dongle *arr, t_data *init);
void		dongle_init(t_dongle *x, int num, int c_down);
void		coder_init(int *data, t_dongle *dongles, int id, t_coder *coder);

void	dongle_init(t_dongle *x, int num, int c_down)
{
	x->id = num;
	x->cooldown = c_down;
	x->flag = 0;
	x->used_time = 0;
	x->q_size = 0;
	x->qeue = NULL;
	pthread_mutex_init(&x->mutex, NULL);
	pthread_cond_init(&x->d_cond, NULL);
}

t_dongle	*dongle_arr(int c_down, int num)
{
	t_dongle	*res;
	int			i;

	i = 0;
	res = malloc(sizeof(t_dongle) * num);
	if (!res)
		return (NULL);
	while (i < num)
	{
		dongle_init(&res[i], i, c_down);
		res[i].qeue = malloc(sizeof(int) * num);
		if (!res[i].qeue)
		{
			while (--i >= 0)
			{
				pthread_mutex_destroy(&res[i].mutex);
				pthread_cond_destroy(&res[i].d_cond);
				free(res[i].qeue);
			}
			free(res);
			return (NULL);
		}
		i++;
	}
	return (res);
}

t_data	*data_init(int num)
{
	t_data		*res;

	res = malloc(sizeof(t_data));
	if (!res)
		return (NULL);
	res->started = 0;
	res->stop = 0;
	res->c_num = num;
	res->start_t = 0;
	pthread_mutex_init(&res->lock, NULL);
	pthread_mutex_init(&res->print_lock, NULL);
	pthread_cond_init(&res->cond, NULL);
	return (res);
}

void	coder_init(int *data, t_dongle *dongles, int id, t_coder *coder)
{
	coder->id = id;
	coder->q_time = 0;
	coder->burnout_time = data[1];
	coder->comp_time = data[2];
	coder->debug_time = data[3];
	coder->refac_time = data[4];
	coder->compiles = data[5];
	coder->key = data[7];
	coder->last_comp = get_time_ms();
	coder->l_don = &dongles[id - 1];
	coder->r_don = &dongles[(id) % data[0]];
	pthread_mutex_init(&coder->c_mut, NULL);
}

t_coder	*coder_arr(int *data, t_dongle *arr, t_data *init)
{
	int				i;
	t_coder			*res;

	i = 0;
	res = malloc(sizeof(t_coder) * data[0]);
	if (!res)
		return (NULL);
	while (i < data[0])
	{
		coder_init(data, arr, i + 1, &res[i]);
		res[i].init = init;
		i++;
	}
	return (res);
}
