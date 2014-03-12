/*
 * cpu_main.c : Executes the running simulated process, and handles context switches
 * 				and interrupts.
 *
 *  Created on: Mar 4, 2014
 *      Author: Group10
 */

/*
 *	Dawn Rocks
 *	Maya Osbourne
 *	Mike Baxter Peter Pentescu
 *
 *	TCSS422 Operating Systems
 *	Project 2 - Simulated OS
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "process.h"
#include <unistd.h>
#include "queue.h"
#include <string.h>
#include "scheduler.h"
#include <termios.h>
//used to cross compile between win and unix based system commands
#ifdef _WIN32
	#define CLEAR system("cls")
	#define FLUSH fflush(stdin)
	#define SLEEP system("PAUSE")

#else
	#define CLEAR system("clear")
	#define FLUSH draino()
	#define SLEEP  getchar()

#endif


void draino(void)
{
	char c;
	while( (c=fgetc(stdin)) !='\n' ) ;
}
#define TRUE 1
#define FALSE 0

#define RUN 1
#define INTERRUPT 0

// Note that keyboard is device 0
#define NUM_DEVICES 2
#define KB_DEVICE 0
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

// total number of processes
int num_processes;

// total number of producer consumer processes
int num_pc_processes;
//total number of i/o processes
int num_io_processes;
//total number of keyboard processes
int num_keyboard_processes;
//total number of compute processes(total processes - all the other input processes)
int num_compute_processes;
//scheduler selection at prompt
int scheduler_choice;		// 1 round robin 2 lottery 3 priority - possibly change to an enum
// Holds which interrupts have been activated
int global_interrupt_state;

// Global signal to all threads - if it's FALSE then everybody should wrap things up and exit
int global_run_state = TRUE;

PCBStr pcb_to_run; //PCB returned from scheduler to run.



// Mutex so we don't modify interrupts in the process of handling them
pthread_mutex_t interrupt_mutex;



// Timer device
void timer_interrupt();
void* timer_interrupt_fp = (*timer_interrupt);

// Keyboard device
void kb_interrupt();
void* kb_interrupt_fp = (*kb_interrupt);

// IO device
void io_interrupt();
void* io_interrupt_fp = (*io_interrupt);

void get_input();

int main(int argc, char * argv[])
{
	// TODO: this is temporary, just to have placeholders for
	PCBStr * current_pcb;
	ProcessStr * current_process;

	PCBStr ** all_pcbs;
	int run_scheduler = 0;

	pthread_mutexattr_t default_mutex_attr;

	pthread_mutexattr_init(&default_mutex_attr);
	pthread_mutex_init(&interrupt_mutex, &default_mutex_attr);

	pthread_t timer_thread;
	pthread_t kb_thread;

	// Set up keyboard input for zero delay
	// Code blatantly copied from google because terminal stuff is black magic
	//struct termios info;
	//	tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin*/
	//	info.c_lflag &= ~ICANON;      /* disable canonical mode*/
	//	info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available*/
	//	info.c_cc[VTIME] = 0;         /* no timeout*/
	//	tcsetattr(0, TCSANOW, &info); /* set immediately*/
	// end copy paste job

	// Start up our devices.
	printf("Creating timer pthread\n");
	pthread_create(&timer_thread, NULL, timer_interrupt_fp, NULL);
	printf("Creating kb pthread\n");
	pthread_create(&kb_thread, NULL, kb_interrupt_fp, NULL);
	printf("OK, created some threads...\n");

	// Initialize blocked-process queue for devices
	queue process_blocked_on_devices[NUM_DEVICES];
	for (int i = 0; i < NUM_DEVICES; i++)
	{
		buildQueue(BLOCK_QUEUE_LENGTH, process_blocked_on_devices[i]);
	}
	printf("OK, created the device blocking queue");

	// Initialize mutexes and blocked-process queue for mutexes
	int mutexes[NUM_MUTEXES];		// keeps track of current mutex holder, -1 = nobody
	queue processes_blocked_on_mutexes[NUM_MUTEXES];	// keeps track of who's waiting to grab a mutex
	for (int i = 0; i < NUM_MUTEXES; i++)
	{
		mutexes[i] = NOBODY_HOLDS_MUTEX;
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_mutexes[i]);
	}
	printf("OK, created the mutex blocking queue");

	// Initialize shared memory locations and blocked-process queue for same
	int shared_mem[NUM_SHARED_MEM_LOCS];
	queue processes_blocked_on_shared_mem[NUM_SHARED_MEM_LOCS];
	for (int i = 0; i < NUM_SHARED_MEM_LOCS; i++)
	{
		shared_mem[i] = 0;	// all shared memory will start at zero
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_shared_mem[i]);
	}

	while (TRUE) {
		usleep(100000); // Run 10 steps/second, slowed down enough to be capable of human comprehension
		printf("whoop whoop");
		// sources of interrupts: timer, I/O devices, system calls

		while (global_interrupt_state == 0 && current_pcb->state == RUNNING) {

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
				// If we're not waiting on a keyboard input, then fire up a one-shot thread that will
				// fire an interrupt back after a short time.
				if (argument != KB_DEVICE)
				{
					pthread_t temp_input_thread;
					pthread_create(&temp_input_thread, NULL, io_interrupt, NULL);
				}
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
				break;
			}

			// If we didn't just block, then increment PC to next instruction. We're doing this at the end to make
			// sure we don't skip over blocked instructions when we come back to them.
			if (current_pcb->state != BLOCKED)
				// increment pc
				current_pcb->next_step = (current_pcb->next_step + 2) % current_process->no_steps;
		}

		// handle interrupts
		// =================

		pthread_mutex_lock(&interrupt_mutex);
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
			printf("IO device interrupt received!\n");
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
			pcb_to_run = * all_pcbs[scheduler(scheduler_choice, all_pcbs, num_processes, current_pcb->pid)];

		}
		pthread_mutex_unlock(&interrupt_mutex);
	}
}

//prompts for information to start the simulated operating system
//need to get total number of processes, number of keyboard processes, number of I/O bound processes and PC processes
//remaining will be compute bound
void get_input()
{
	num_compute_processes = 0;
	while(num_compute_processes < 1)
	{
		CLEAR;
		FLUSH;
			printf("\n Please enter total number of processes to run: ");
			scanf("%d", &num_processes);
			FLUSH;
			num_compute_processes = num_processes;
			printf("\n Please enter total number of keyboard processes to run: ");
			scanf("%d", &num_keyboard_processes);
			FLUSH;
			num_compute_processes = num_processes - num_keyboard_processes;
			printf("\n Please enter total number of I/O bound processes to run: ");
			scanf("%d", &num_io_processes);
			FLUSH;
			num_compute_processes = num_processes - num_io_processes;
			printf("\n Please enter total number of p/c processes to run: ");
			scanf("%d", &num_pc_processes);
			num_compute_processes = num_processes - num_pc_processes;
			FLUSH;
			printf("\n Please select the scheduling algorithm to use: ");
			printf("\n 1. Round Robin");
			printf("\n 2. Lottery");
			printf("\n 3. Priority");
			scanf("%d", &scheduler_choice);	//can possibly do an enum here.
			if(num_compute_processes < 1)
			{
				printf("\n ERROR! You did not enter enough total processes. Please re-enter process information. ");
			}
	}



}

// Timer interrupt thread function - sleep for one half second, set timer interrupt on, repeat as long as global
// run state is true.
// TODO: pass in defined sleep time
void timer_interrupt()
{
	while(global_run_state == TRUE)
	{
		printf("TICK TOCK MOTHAFUCKA\n");
		usleep(5000000);	// 5 seconds
		pthread_mutex_lock(&interrupt_mutex);
		if (!(global_interrupt_state & TIMER_INTERRUPT))
			global_interrupt_state += TIMER_INTERRUPT;
		pthread_mutex_unlock(&interrupt_mutex);

	}
}

// Keyboard interrupt thread function - whenever the user types a key, set keyboard interrupt on, repeat as long
// as global run state is true
void kb_interrupt()
{
	while(global_run_state == TRUE)
	{
		getchar();	// Blocks when there is no input
		pthread_mutex_lock(&interrupt_mutex);
		if (!(global_interrupt_state & KEYBOARD_INTERRUPT))
			global_interrupt_state += KEYBOARD_INTERRUPT;
		pthread_mutex_unlock(&interrupt_mutex);
		printf("WHY YOU GOTTA TYPE LIKE THAT?");
	}
}

// This one is a bit different... instead of looping forever, we fire up a new instance of this thread whenever
// we need to wait for an i/o interrupt.
void io_interrupt()
{
	usleep(1000000);	// 1 second
	pthread_mutex_lock(&interrupt_mutex);
	if (!(global_interrupt_state & IO_INTERRUPT))
		global_interrupt_state += IO_INTERRUPT;
	pthread_mutex_unlock(&interrupt_mutex);
}
