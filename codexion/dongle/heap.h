/* ** heap.h - generic min-heap used as a dongle wait queue ** */
#ifndef HEAP_H
#define HEAP_H

#include "codexion.h"

int waiter_less(t_waiter *a, t_waiter *b);
void heap_push(t_dongle *d, t_waiter *w);
t_waiter *heap_pop(t_dongle *d);
t_waiter *heap_peek(t_dongle *d);
void heap_remove(t_dongle *d, t_waiter *w);

#endif
