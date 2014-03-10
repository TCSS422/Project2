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
#include <process.h>
#include <unistd.h>
#include <queue.h>

#define RUN 1
#define INTERRUPT 0

#define NUM_DEVICES 2
#define NUM_MUTEXES 3
#define NUM_SHARED_MEM_LOCS 4
#define BLOCK_QUEUE_LENGTH 10

#define NOBODY_HOLDS_MUTEX -1


int main(int argc, char * argv[])
{
	// TODO: this is temporary, just to have placeholders for
	PCBStr * current_pcb;
	ProcessStr * current_process;

	PCBStr ** all_pcbs;

	// Initialize blocked-process queue for devices
	queue process_blocked_on_devices[NUM_DEVICES];
	for (int i = 0; i < NUM_DEVICES; i++)
	{
		buildQueue(BLOCK_QUEUE_LENGTH, process_blocked_on_devices[i]);
	}

	// Initialize mutexes and blocked-process queue for mutexes
	int mutexes[NUM_MUTEXES];		// keeps track of current mutex holder, -1 = nobody
	queue processes_blocked_on_mutexes[NUM_MUTEXES];	// keeps track of who's waiting to grab a mutex
	for (int i = 0; i < NUM_MUTEXES; i++)
	{
		mutexes[i] = NOBODY_HOLDS_MUTEX;
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_mutexes[i]);
	}

	// Initialize shared memory locations and blocked-process queue for same
	int shared_mem[NUM_SHARED_MEM_LOCS];
	queue processes_blocked_on_shared_mem[NUM_SHARED_MEM_LOCS];
	for (int i = 0; i < NUM_SHARED_MEM_LOCS; i++)
	{
		shared_mem[i] = 0;	// all shared memory will start at zero
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_shared_mem[i]);
	}

	while(RUN) {

		// sources of interrupts: timer, I/O devices, system calls

		while (!INTERRUPT && current_pcb->state == RUNNING) {
			// increment pc
			current_pcb->next_step = (current_pcb->next_step + 2) % current_process->no_steps;

			// read instruction
			int instruction = current_process->requests[current_pcb->next_step];
			int argument = current_process->requests[current_pcb->next_step + 1];

			// switch statement that determines what to do based on instruction in pc
			switch(instruction)
			{
			case INSTRUCTION_OUTPUT:	// aka "system call"
				// We're blocked waiting for the output operation to finish:
				current_pcb->state = BLOCKED;
				// We need to add ourselves to the queue for the device:
				addToEnd(process_blocked_on_devices[argument], current_pcb->pid);
				break;
			case INSTRUCTION_MUTEX_LOCK:
				// Take the mutex:
				if (mutexes[argument] == NOBODY_HOLDS_MUTEX)
					mutexes[argument] = current_pcb->pid;
				// Block and wait for the mutex to be free:
				else
				{
					current_pcb->state = BLOCKED;
					addToEnd(processes_blocked_on_mutexes[argument], current_pcb->pid);
				}
				break;
			case INSTRUCTION_MUTEX_UNLOCK:
				if (mutexes[argument] == current_pcb->pid)
				{
					mutexes[argument] = NOBODY_HOLDS_MUTEX;
					// If anybody's waiting on this:
					if (processes_blocked_on_mutexes[argument].position >= 0)
					{
						// find out who they are
						int waiting_process = getFirstItem(processes_blocked_on_mutexes[argument]);
						// and wake them up...
						all_pcbs[waiting_process]->state = RUNNING;
					}
				}
				// If we hold the mutex:
				//   Release the lock and notify everybody waiting on this mutex.
				// If we don't hold the mutex
				//   Put up an error message and continue
				break;
			case INSTRUCTION_INC_SHARED_MEM:
				// If shared memory location is zero:
				//   Notify first process in wait queue
				if (shared_mem[argument] == 0)
				{
					int waiting_process = getFirstItem(processes_blocked_on_shared_mem[argument]);
					// and wake them up...
					all_pcbs[waiting_process]->state = RUNNING;
				}
				// Actually increment the memory location
				shared_mem[argument]++;
				break;
			case INSTRUCTION_DEC_SHARED_MEM:
				// If shared memory location is zero:
				//   Block and wait
				// If shared memory is not zero:
				//	 Decrement and continue
				if (shared_mem[argument] == 0)
				{
					current_pcb->state = BLOCKED;
					addToEnd(processes_blocked_on_shared_mem[argument], current_pcb->pid);
				}
				else
				{
					shared_mem[argument]--;
				}
				break;
			case INSTRUCTION_NOP:
				// Do nothing
				break;
			default:
				printf("Illegal instruction\n");
			}
		}
		// handle interrupts
	}
}
