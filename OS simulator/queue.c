/*
 * queue.c
 *
 *  Created on: Mar 8, 2014
 *
 *	Mike Baxter
 *  Peter Pentescu
 *  Maya Osbourne
 *  Dawn Rocks
 *  A queue of ints.
 */

#include "queue.h"
#include <stdlib.h>

// basically a constructor:
void buildQueue(int newsize, queue * newQueue){
	newQueue->position = -1;
	newQueue->size = newsize;
	newQueue->data = malloc(sizeof(int) * newsize);

	for (int i = 0; i < newsize; i++) {
		newQueue->data[i] = 0;
	}
}

int addToEnd(queue * target, int newData)
{
	if (target->position < (target->size - 1))
	{
		target->position++;
		target->data[target->position] = newData;
		return 0;
	}
	else return 1;	// whoops, full
}

int hasData(queue * target)
{
	if (target -> position >= 0)
		return 1;
	else return 0;
}

int getFirstItem(queue * target)
{
	int i;
	if (target->position >= 0)
	{
		int returndata = target->data[0];
		(target->position)--;
		for (i = 0; i < target->size; i++)
		{
			target->data[i] = target->data[i + 1];
		}
		return returndata;
	}
	else return -1;
}

int destroyQueue(queue * target)
{
	target->position = -1;
	target->size = -1;
	free(target->data);
	return 0;
}
