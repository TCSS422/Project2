/*
 * queue.c
 *
 *  Created on: Mar 8, 2014
 *      Author: Peter
 *
 *  A queue of ints.
 */

#include "queue.h"
#include <stdlib.h>

// basically a constructor:
void buildQueue(int newsize, queue newQueue){
	newQueue.position = -1;
	newQueue.size = newsize;
	newQueue.data = malloc(sizeof(int) * newsize);
}

int addToEnd(queue target, int newData)
{
	if (target.position < target.size - 1)
	{
		target.position++;
		target.data[target.position] = newData;
		return 0;
	}
	else return 1;	// whoops, full
}

int getFirstItem(queue target)
{
	if (target.position >= 0)
	{
		int returndata = target.data[0];
		target.position--;
		for (int i = 0; i < target.size; i++)
		{
			target.data[i] = target.data[i + 1];	// terribly inefficient.
			// TODO: make it all nice and modulusy
		}
		return returndata;
	}
	else return -1;
}

int destroyQueue(queue target)
{
	target.position = -1;
	target.size = -1;
	free(target.data);
	return 0;
}
