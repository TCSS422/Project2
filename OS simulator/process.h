/*
 * process.h  : ADT that can be extended to simulate different processes.
 *
 *  Created on: Mar 8, 2014
 *      Author: George Mobus (http://faculty.washington.edu/gmobus/Academics/TCSS422/MoodleFiles/Project2.html)
 *      		Group10
 *
 */

#ifndef PROCESS_H_
#define PROCESS_H_

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
	int priority;		// e.g. 3=io-keyboard, 2=producer-consumer, 1=calculator

} PCBStr;

typedef int INSTRUCTION;

#define NUM_INSTRUCTIONS 10000

// instructions
#define INSTRUCTION_OUTPUT 			0		// Input is handled by interrupts, this is output only
#define INSTRUCTION_MUTEX_LOCK 		1
#define INSTRUCTION_MUTEX_UNLOCK 	2
#define INSTRUCTION_INC_SHARED_MEM 	3		// Increments a shared memory location (signals all processes blocking)
#define INSTRUCTION_DEC_SHARED_MEM 	4		// Decrements a shared memory location (blocks if already 0)
#define INSTRUCTION_NOP				5		// stands in for all other instructions (ADD, normal memory access, etc...)

// states
#define RUNNING 	0
#define READY   	1
#define BLOCKED 	2

// types
#define COMPUTE  	0
#define IO       	1
#define KEYBOARD 	2
#define PRODUCER 	3
#define CONSUMER 	4

// functions
char * getProcess(int);
char * getState(int);

#endif /* PROCESS_H_ */
