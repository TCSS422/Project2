/*
 * queue.h
 *
 *  Created on: Mar 8, 2014
 *      Author: Peter
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
