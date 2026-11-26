/* ** codexion.h - shared types and prototypes for the Codexion project ** */
#ifndef CODEXION_H
#define CODEXION_H

#include <pthread.h>
#include <stddef.h>

typedef struct s_coder t_coder;
typedef struct s_sim t_sim;

typedef struct s_waiter
{
	int id;
	long long priority;
	long seq;
	t_coder *coder;
} t_waiter;

typedef struct s_coder
{
	int id;
	int left;
	int right;
	pthread_mutex_t state_mutex;
	long compile_count;
	long long last_compile_start;
	t_sim *sim;
} t_coder;

typedef struct s_dongle
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int in_use;
	long long released_at;
	t_waiter **heap;
	int heap_size;
	int heap_cap;
	long seq;
} t_dongle;

typedef struct s_sim
{
	int stop;
	int burned_out;
	pthread_mutex_t stop_mutex;
	pthread_cond_t stop_cond;
	pthread_mutex_t log_mutex;
	t_coder **coders;
	t_dongle **dongles;
	int n_coders;
	long long start_ms;
	long tb;
	long tc;
	long td;
	long tr;
	long n_compiles;
	long cd;
	int edf;
} t_sim;

#endif
