/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pzavada <pzavada@student.42prague.com>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/25 12:21:50 by pzavada           #+#    #+#             */
/*   Updated: 2026/07/29 16:30:09 by pzavada          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <limits.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
# include <sys/time.h>
# include <errno.h>

typedef struct h_data
{
	int					started;
	int					stop;
	int					c_num;
	unsigned long long	start_t;
	pthread_mutex_t		lock;
	pthread_mutex_t		print_lock;
	pthread_cond_t		cond;
}	t_data;

typedef struct h_dongle
{
	int					id;
	int					flag;
	int					q_size;
	int					*qeue;
	unsigned long long	used_time;
	unsigned long long	cooldown;
	pthread_mutex_t		mutex;
	pthread_cond_t		d_cond;
}	t_dongle;

typedef struct h_coder
{
	int					id;
	int					compiles;
	int					key;
	int					comp_time;
	int					debug_time;
	int					refac_time;
	int					burnout_time;
	unsigned long long	last_comp;
	unsigned long long	q_time;
	pthread_mutex_t		c_mut;
	t_dongle			*l_don;
	t_dongle			*r_don;
	t_data				*init;
}	t_coder;

// === INIT ===
t_data				*data_init(int num);
t_dongle			*dongle_arr(int c_down, int num);
t_coder				*coder_arr(int *data, t_dongle *arr, t_data *init);
void				dongle_init(t_dongle *x, int num, int c_down);
void				coder_init(int *data, t_dongle *d, int id, t_coder *coder);
// === UTILS ===
void				clean_exit(int *mem, t_dongle *d, t_coder *c, pthread_t *t);
void				mut_print(t_coder *c, char *str);
unsigned long long	get_time_ms(void);
int					convert(char *st);
int					parser(int ac, char **av, int *res);
// === MAIN ===
void				*threads_init(void *coder);
pthread_t			*f_clean(t_coder *cds, int n, pthread_t *th, t_dongle *don);
pthread_t			*launch_sim(t_coder *coders, t_dongle *dongles, int num);
// === HEAP ===
void				dongle_manager(t_coder *coder);
void				line(t_coder *c, t_dongle *d,
						struct timespec *x, unsigned long long l);
void				pop(t_coder *cd, t_dongle *don);
void				insert(t_coder *cd, t_dongle *don);
void				compile(t_dongle *first, t_dongle *second, t_coder *cd);
// === HEAP UTILS ===
void				q_swap(t_dongle *don, int a, int b);
void				cmp_lock(t_coder *cd1, t_coder *cd2);
void				signal(t_dongle *first);
int					cmp_coders(t_coder *cd1, t_coder *cd2);
int					c_sleep(int t, t_data *init);
// === MONITOR ===
void				end_sim(t_coder *cd, int flag, int id);
void				monitor(t_coder	*cds);
void				*wait_m(void *arg);
int					comp_ctr(t_coder *temp, int *comps_done, t_coder *cds);
int					is_stopped(t_data *init);

#endif