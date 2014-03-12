/*
 * scheduler.h
 *
 *  Created on: Mar 11, 2014
 *      Author: mike
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

// functions
int scheduler(int sched_policy, PCBStr ** all_pcbs, int num_processes, int curr_process);
int PriorityScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process);
int RoundRobinScheduler(PCBStr ** all_pcbs, int num_processes, int curr_process);
int LotteryScheduler(int num_processes);


#endif /* SCHEDULER_H_ */
