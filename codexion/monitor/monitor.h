/* ** monitor.h - burnout / completion monitor and stop signaling ** */
#ifndef MONITOR_H
#define MONITOR_H

#include "codexion.h"

void set_stop(t_sim *sim);
int all_done(t_sim *sim);
void *monitor_routine(void *arg);

#endif
