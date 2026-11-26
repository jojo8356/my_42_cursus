/* ** heap.c - binary min-heap over t_waiter, ordered by priority ** */
#include "heap.h"

int waiter_less(t_waiter *a, t_waiter *b)
{
	if (a->priority != b->priority)
		return a->priority < b->priority;
	if (a->seq != b->seq)
		return a->seq < b->seq;
	return a->id < b->id;
}

static void heap_sift_up(t_dongle *d, int idx)
{
	int parent;

	while (idx > 0)
	{
		parent = (idx - 1) / 2;
		if (waiter_less(d->heap[idx], d->heap[parent]) != 0)
		{
			t_waiter *tmp = d->heap[idx];

			d->heap[idx] = d->heap[parent];
			d->heap[parent] = tmp;
			idx = parent;
		}
		else
			break;
	}
}

static void heap_sift_down(t_dongle *d, int idx)
{
	int n = d->heap_size;

	while (1)
	{
		int left = 2 * idx + 1;
		int right = 2 * idx + 2;
		int smallest = idx;
		t_waiter *tmp;

		if (left < n && waiter_less(d->heap[left], d->heap[smallest]) != 0)
			smallest = left;
		if (right < n && waiter_less(d->heap[right], d->heap[smallest]) != 0)
			smallest = right;
		if (smallest == idx)
			break;
		tmp = d->heap[idx];
		d->heap[idx] = d->heap[smallest];
		d->heap[smallest] = tmp;
		idx = smallest;
	}
}

void heap_push(t_dongle *d, t_waiter *w)
{
	if (d->heap_size >= d->heap_cap)
		return;
	d->heap[d->heap_size] = w;
	d->heap_size++;
	heap_sift_up(d, d->heap_size - 1);
}

t_waiter *heap_peek(t_dongle *d)
{
	if (d->heap_size == 0)
		return NULL;
	return d->heap[0];
}

t_waiter *heap_pop(t_dongle *d)
{
	t_waiter *top;
	t_waiter *last;

	if (d->heap_size == 0)
		return NULL;
	top = d->heap[0];
	last = d->heap[d->heap_size - 1];
	d->heap_size--;
	d->heap[0] = last;
	heap_sift_down(d, 0);
	return top;
}

void heap_remove(t_dongle *d, t_waiter *w)
{
	int i;

	for (i = 0; i < d->heap_size; i++)
	{
		if (d->heap[i] == w)
			break;
	}
	if (i >= d->heap_size)
		return;
	d->heap[i] = d->heap[d->heap_size - 1];
	d->heap_size--;
	heap_sift_down(d, i);
	heap_sift_up(d, i);
}
