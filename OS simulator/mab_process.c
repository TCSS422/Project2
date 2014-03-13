/*
 * process.c  : Implementation of process functions.
 *
 *  Created on: Mar 8, 2014
 *      Author: Group10
 */

#include <stdlib.h>

#include "mab_process.h"

#define HI_PRIORITY  3
#define MD_PRIORITY 2
#define LO_PRIORITY  1

// these functions are of no concern to an outside caller
void init_array(int *);
void add_io_system_calls(int *, int);
void add_pc_system_calls(int *, int, int);

// most process initialization can be generalized
PCBStr * make_process(int proc_id, int type) {
	PCBStr *p;
	ProcessStr *proc;
	int *r;
	int i, steps;
	p = malloc(sizeof(PCBStr));
	p->pid = proc_id;
	p->state = READY;
	proc = malloc(sizeof(ProcessStr));
	p->proc = proc;
	proc->proc_type = type;
	proc->no_steps = rand() % NUM_INSTRUCTIONS; // random number of steps within range
	proc->no_requests = NUM_INSTRUCTIONS;
	r = malloc(sizeof(NUM_INSTRUCTIONS));
	proc->requests = r;
	init_array(r);

	// complete initialization per process type
	switch (type)
	{
		steps = proc->no_steps;
		case COMPUTE:
			p->priority = LO_PRIORITY;
			add_io_system_calls(r, steps);
			break;
		case IO:
			p->priority = HI_PRIORITY;
			add_io_system_calls(r, steps);
			break;
		case KEYBOARD:
			p->priority = HI_PRIORITY;
			add_io_system_calls(r, steps);
			break;
		case PRODUCER:
			p->priority = MD_PRIORITY;
			add_pc_system_calls(r, steps, type);
			break;
		case CONSUMER:
			p->priority = MD_PRIORITY;
			add_pc_system_calls(r, steps, type);
			break;
		default:
			break;
	}
	return p;
}

// construct an array of nop default values
void init_array(int * a) {
	int i;
	for (i = 0; i < NUM_INSTRUCTIONS; i++) {
		a[i] = INSTRUCTION_NOP;
	}
}

// for random number of steps place instructions randomly
void add_io_system_calls(int * a, int steps) {
	int i;
	for (i = 0; i < steps; i++) {
		a[rand() % NUM_INSTRUCTIONS] = INSTRUCTION_OUTPUT;
	}
}

// for random number of steps place instructions randomly
void add_pc_system_calls(int * a, int steps, int type) {
	int i, random;
	for (i = 0; i < steps; i++) {
		random = rand() % (NUM_INSTRUCTIONS - 3); // - 3 for looking ahead
		if ((a[random] == INSTRUCTION_NOP
		     && a[random + 1] == INSTRUCTION_NOP
		     && a[random + 2] == INSTRUCTION_NOP)) {
			a[random] = INSTRUCTION_MUTEX_LOCK;
			if (type == PRODUCER) {
				a[random + 1] = INSTRUCTION_INC_SHARED_MEM;
			} else {
				a[random + 1] = INSTRUCTION_DEC_SHARED_MEM;
			}
			a[random + 2] = INSTRUCTION_MUTEX_UNLOCK;
		} else {
			i--;
		}
	}

}

// maybe we can use this later, if not we can remove

/*
char * getProcess(int val)
{
	char type[N];

	switch(val)
	{
	case COMPUTE:
		type = "Calculator";
		break;
	case IO:
		type = "IO";
		break;
	case KEYBOARD:
		type = "Keyboard";
		break;
	case PRODUCER:
		type = "Producer";
		break;
	case CONSUMER:
		type = "Consumer";
		break;
	default:
		type = "Unknown";
		break;
	}
	return type;
}

char * getState(int val)
{
	char type[N];

	switch(val)
	{
	case RUNNING:
		type = "RUNNING";
		break;
	case READY:
		type = "RUNNABLE";
		break;
	case BLOCKED:
		type = "BLOCKED";
		break;
	default:
		type = "UNKNOWN";
		break;
	}
	return type;
}
*/
