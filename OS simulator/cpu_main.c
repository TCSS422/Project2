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

#define TRUE 1
#define FALSE 0

#define RUN 1
#define INTERRUPT 0

// Note that keyboard is device 0
#define NUM_DEVICES 2
#define NUM_MUTEXES 3
#define NUM_SHARED_MEM_LOCS 4
#define BLOCK_QUEUE_LENGTH 10

#define NOBODY_HOLDS_MUTEX -1

// possible interrupt state values
// note: if more than one interrupt is received then these act as bitmasks
// ex: both timer and io interrupts are received, then global_interrupt_state = 5
// (TIMER_INTERRUPT + IO_INTERRUPT)
// IO KEY TIMER
// 1  0   1      = 5 (IO and timer)
// 1  1   1      = 7 (all lit up)
// 0  0   1      = 1 (just timer)
#define UNINTERRUPTED 0
#define TIMER_INTERRUPT 1
#define KEYBOARD_INTERRUPT 2
#define IO_INTERRUPT 4

int global_interrupt_state;

int main(int argc, char * argv[])
{
	// TODO: this is temporary, just to have placeholders for
	PCBStr * current_pcb;
	ProcessStr * current_process;

	PCBStr ** all_pcbs;
	int run_scheduler = 0;

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

		while (global_interrupt_state == 0 && current_pcb->state == RUNNING) {
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
		// =================

		// bitmask on timer interrupt
		if (global_interrupt_state & TIMER_INTERRUPT)
		{
			global_interrupt_state -= TIMER_INTERRUPT;
			printf("Timer interrupt received!");
			// all we need to do is run the scheduler
			run_scheduler = TRUE;
		}

		// bitmask on keyboard interrupt
		if (global_interrupt_state & KEYBOARD_INTERRUPT)
		{
			global_interrupt_state -= KEYBOARD_INTERRUPT;
			printf("Keyboard interrupt received!\n");
			// Wake up the next process waiting on the keyboard
			int waiting_process = getFirstItem(process_blocked_on_devices[0]);
			// and wake it up... (note that we aren't immediately running the scheduler, it just is
			// eligible to be scheduled!)
			all_pcbs[waiting_process]->state = RUNNING;
		}

		// bitmask on IO interrupt
		if (global_interrupt_state & IO_INTERRUPT)
		{
			global_interrupt_state -= IO_INTERRUPT;
			printf("IO device interrupt received!\n")
			// Find the next process waiting on the IO device
			int waiting_process = getFirstItem(process_blocked_on_devices[1]);
			// and wake it up...
			all_pcbs[waiting_process]->state = RUNNING;
		}

		// See if we need to run the scheduler - either we just blocked on some system call,
		// or we hit the timer interrupt.
		if (run_scheduler || current_pcb->state == BLOCKED)
		{
			run_scheduler = FALSE;
			// SCHEDULER MAGIC...
			// current_pcb = newly_scheduled_pcb;
			// current_process = newly_scheduled_process;
		}
	}
}
