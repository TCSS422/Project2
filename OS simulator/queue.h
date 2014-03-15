/*
 * queue.h
 *
 *  Created on: Mar 8, 2014
 * 	Mike Baxter
 *  Peter Pentescu
 *  Maya Osbourne
 *  Dawn Rocks
 */

#ifndef QUEUE_H_
#define QUEUE_H_

typedef struct {
	int position;
	int size;
	int* data;
} queue;

void buildQueue(int newsize, queue * newQueue);
int addToEnd(queue * target, int newData);
int getFirstItem(queue * target);
int destroyQueue(queue * target);
int hasData(queue * target);

#endif /* QUEUE_H_ */
