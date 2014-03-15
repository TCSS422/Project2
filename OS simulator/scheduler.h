/*
 * scheduler.h  : General call to scheduler that returns the next runnable
 * 				  process.
 *
 *  Created on: Mar 11, 2014
 *      Author: Group10
 * 	Mike Baxter
 *  Peter Pentescu
 *  Maya Osbourne
 *  Dawn Rocks
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

// functions
int scheduler(int sched_policy, PCBStr ** all_pcbs, int num_processes, int curr_process);

#endif /* SCHEDULER_H_ */
