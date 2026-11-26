/* ** log.h - serialized event logging ** */
#ifndef LOG_H
#define LOG_H

#include "codexion.h"

void log_event(t_sim *sim, int id, const char *msg);

#endif
