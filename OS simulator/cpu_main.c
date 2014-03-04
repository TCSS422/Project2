/*
 * cpu_main.c : Executes the running simulated process, and handles context switches
 * 				and interrupts.
 *
 *  Created on: Mar 4, 2014
 *      Author: Group10
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RUN 1
#define INTERRUPT 0

typedef struct process_str {
	int proc_type;      // code for process type, e.g. 0=compute, 1=i0, 2=keyboard, etc.
	int	no_steps;		// number of time steps before resetting to 0 - number of instructions
	int no_requests;	// number of requests that will be generated during the process run
	int * requests;		// an array of requests, each request (e.g. io service) is issued at a specific
						// time step. These are the synchronous events that result in traps being issued.
						// You can get fancy and make this an array of RequestTypeStr which contains the
						// step number when issued, and a request type (e.g. 0=io, 1=sync, etc.)
} ProcessStr;

typedef struct pcb_str {
	int	pid;
	int next_step;		// this is the step count that the CPU had gotten to when this process was
						// prempted (like a PC register value)
	int state;			// e.g. 0=running, 1=ready, 2=interrupted, 3=blocked
	ProcessStr proc;    // pointer to the actual process
	int waiting_on;		// which queue is it in if it is waiting on something (blocked)
	int owns;			// which mutex lock does it own
	// anything else you need
} PCBStr;

int main(int argc, char * argv[])
{
	PCBStr * current_pcb;
	ProcessStr * current_process;
	while(RUN) {
		// sources of interrupts: timer, I/O devices, system calls

		while (!INTERRUPT) {
			// increment pc
			current_pcb->next_step = (current_pcb->next_step + 1) % current_process->no_steps;

			// switch statement that determines what to do based on instruction in pc

		}
		// handle interrupts
	}
}
