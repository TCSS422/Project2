/*
 * process.c  : Implementation of process functions.
 *
 *  Created on: Mar 8, 2014
 *      Author: Group10
 */

#include <stdlib.h>
#include <stdio.h>
#include "mab_process.h"

#define HI_PRIORITY  3
#define MD_PRIORITY  2
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
	proc->no_steps = NUM_INSTRUCTIONS;
	proc->no_requests = NUM_REQUESTS;
	r = malloc(sizeof(int) * NUM_INSTRUCTIONS);
	proc->requests = r;
	init_array(r);

	steps = proc->no_steps;
	// complete initialization per process type
	switch (type)
	{

		case COMPUTE:
			p->priority = LO_PRIORITY;
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
	for (int i = 0; i < p->proc->no_steps; i++)
		printf("%d", p->proc->requests[i]);
	printf("\n");
	return p;
}

// fill an array of nop default values
void init_array(int * a) {
	int i;
	for (i = 0; i < NUM_INSTRUCTIONS; i++) {
		a[i] = INSTRUCTION_NOP;
		//printf("nop\n");
	}
}

// for number of steps, place instructions randomly
void add_io_system_calls(int * a, int steps) {
	int i;
	for (i = 0; i < NUM_REQUESTS; i++)
	{
		a[rand() % steps] = INSTRUCTION_OUTPUT;
	}
}

// for number of steps place instructions randomly
void add_pc_system_calls(int * a, int steps, int type) {
	int i, random;
	for (i = 0; i < NUM_REQUESTS; i++) {
		random = rand() % steps;
		if (a[random] == INSTRUCTION_NOP) {
			if (type == PRODUCER) {
				a[random] = INSTRUCTION_INC_SHARED_MEM;
			} else {
				a[random] = INSTRUCTION_DEC_SHARED_MEM;
			}
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
