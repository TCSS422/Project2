/*
 * scheduler.c  : Implementation of scheduler functions.
 *
 *  Created on: Mar 11, 2014
 *      Author: Group10
 */

#include <stdlib.h>

#include "mab_process.h"
#include "scheduler.h"

// these functions are of no concern to an outside caller
int PriorityScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process);
int RoundRobinScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process);
int LotteryScheduler(int num_processes);

// use variable to call function specific to policy
int scheduler(int sched_policy, PCBStr ** all_pcbs, int num_processes, int curr_process)
{
	switch(sched_policy)
	{
	case 1:
		return RoundRobinScheduler(all_pcbs, num_processes, curr_process);
		break;
	case 2:
		return LotteryScheduler(num_processes);
		break;
	case 3:
		return PriorityScheduler(all_pcbs, num_processes, curr_process);
		break;
	}
	return 0; // shouldn't ever get here
}

// io-keyboard tasks are highest priority, followed by p-c memory management,
// then background processes
int PriorityScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process)
{
	int i;
	int maxPriority = -1;
	int nextProcess = -1;

	for(i = 0; i < num_processes; i++)
	{
		// Check against current process so that same high priority process
		// doesn't indefinitely. The next high priority process will get a turn,
		// or a lower priority process will.
		if (all_pcbs[i] -> priority > maxPriority && i != curr_process)
		{
			maxPriority = all_pcbs[i] -> priority;
			nextProcess = i;
		}
	}
	return nextProcess;
}

// simple queue
int RoundRobinScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process)
{
	int front = 0;
	int tail = num_processes - 1;
	int head = curr_process;

	head++; // advance head, wrap around if necessary
	if (head > tail) {
		head = front;
	}
	// if nothing else, calculator process will always be READY
	while (all_pcbs[head] -> state != READY) {
		head++;
		if (head > tail) {
			head = front;
		}
	}
	return head;
}

// simply return a random
int LotteryScheduler(int num_processes)
{
	// this seems too easy, but for simulation we'll take it
	return rand() % num_processes - 1;
}
