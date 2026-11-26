/* ** codexion.c - entry point: parse, init, spawn threads, join, cleanup ** */
#include "codexion.h"
#include "utils.h"
#include "log.h"
#include "dongle.h"
#include "coder.h"
#include "monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int parse_args(int argc, char **argv, t_sim *sim)
{
	long val;

	if (argc != 9)
		return 0;
	if (parse_strict_int(argv[1], &val) == 0 || val < 1)
		return 0;
	sim->n_coders = (int)val;
	if (parse_strict_int(argv[2], &val) == 0 || val < 1)
		return 0;
	sim->tb = val;
	if (parse_strict_int(argv[3], &val) == 0 || val < 0)
		return 0;
	sim->tc = val;
	if (parse_strict_int(argv[4], &val) == 0 || val < 0)
		return 0;
	sim->td = val;
	if (parse_strict_int(argv[5], &val) == 0 || val < 0)
		return 0;
	sim->tr = val;
	if (parse_strict_int(argv[6], &val) == 0 || val < 0)
		return 0;
	sim->n_compiles = val;
	if (parse_strict_int(argv[7], &val) == 0 || val < 0)
		return 0;
	sim->cd = val;
	if (strcmp(argv[8], "fifo") == 0)
		sim->edf = 0;
	else if (strcmp(argv[8], "edf") == 0)
		sim->edf = 1;
	else
		return 0;
	return 1;
}

static void init_sim(t_sim *sim)
{
	int i;

	sim->stop = 0;
	sim->burned_out = 0;
	sim->start_ms = now_ms();
	sim->coders = malloc(sizeof(t_coder *) * (size_t)sim->n_coders);
	sim->dongles = malloc(sizeof(t_dongle *) * (size_t)sim->n_coders);
	pthread_mutex_init(&sim->stop_mutex, NULL);
	pthread_cond_init(&sim->stop_cond, NULL);
	pthread_mutex_init(&sim->log_mutex, NULL);
	for (i = 0; i < sim->n_coders; i++)
	{
		sim->coders[i] = malloc(sizeof(t_coder));
		sim->coders[i]->id = i + 1;
		sim->coders[i]->left = (i - 1 + sim->n_coders) % sim->n_coders;
		sim->coders[i]->right = i;
		sim->coders[i]->compile_count = 0;
		sim->coders[i]->last_compile_start = 0;
		sim->coders[i]->sim = sim;
		pthread_mutex_init(&sim->coders[i]->state_mutex, NULL);
		sim->dongles[i] = malloc(sizeof(t_dongle));
		dongle_init(sim->dongles[i], 4);
	}
}

static void cleanup_sim(t_sim *sim)
{
	int i;

	for (i = 0; i < sim->n_coders; i++)
	{
		pthread_mutex_destroy(&sim->coders[i]->state_mutex);
		free(sim->coders[i]);
	}
	for (i = 0; i < sim->n_coders; i++)
	{
		free(sim->dongles[i]->heap);
		pthread_mutex_destroy(&sim->dongles[i]->mutex);
		pthread_cond_destroy(&sim->dongles[i]->cond);
		free(sim->dongles[i]);
	}
	free(sim->coders);
	free(sim->dongles);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->stop_mutex);
	pthread_cond_destroy(&sim->stop_cond);
}

int main(int argc, char **argv)
{
	t_sim sim;
	pthread_t *threads;
	int i;

	memset(&sim, 0, sizeof(sim));
	if (parse_args(argc, argv, &sim) == 0)
	{
		fprintf(stderr, "Error: invalid arguments\n");
		return 1;
	}
	if (sim.n_compiles == 0)
		return 0;
	init_sim(&sim);
	threads = malloc(sizeof(pthread_t) * (size_t)(sim.n_coders + 1));
	for (i = 0; i < sim.n_coders; i++)
		pthread_create(&threads[i], NULL, coder_routine, sim.coders[i]);
	pthread_create(&threads[sim.n_coders], NULL, monitor_routine, &sim);
	for (i = 0; i <= sim.n_coders; i++)
		pthread_join(threads[i], NULL);
	free(threads);
	cleanup_sim(&sim);
	return 0;
}
