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
	int* data[];
} queue;

#endif /* QUEUE_H_ */
