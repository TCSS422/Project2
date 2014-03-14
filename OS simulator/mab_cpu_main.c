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
#include <unistd.h>
#include <string.h>
#include <termios.h>

#include "mab_process.h"
#include "queue.h"
#include "scheduler.h"


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

#define TRUE 1
#define FALSE 0
#define NULL ((void *) 0)

#define RUN 1
#define INTERRUPT 0

// Note that keyboard is device 0
// #define NUM_DEVICES 2
#define KB_DEVICE 0
#define IO_DEVICE 1
// #define NUM_MUTEXES 3
// #define NUM_SHARED_MEM_LOCS 4
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
int global_interrupt_state = 0;
// Global signal to all threads - if it's FALSE then everybody should wrap things up and exit
int global_run_state = TRUE;

int num_devices;

int num_mutexes;

int num_mem_locs;

//PCB returned from scheduler to run.
PCBStr pcb_to_run;

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

// User input
void get_input();

// cleans something
void draino(void)
{
	char c;
	while( (c=fgetc(stdin)) !='\n' ) ;
}

int main(int argc, char * argv[])
{
	PCBStr * current_pcb;
	ProcessStr * current_process;

	PCBStr ** all_pcbs;

	get_input();

	num_processes = num_keyboard_processes + num_io_processes
				  + (2 * num_pc_processes) + num_compute_processes;
	num_devices = num_keyboard_processes + num_io_processes + num_compute_processes;
	num_mutexes = num_mem_locs = num_pc_processes;

	// initialize process table
	all_pcbs = (PCBStr **)malloc(num_processes * sizeof(PCBStr *));

	// fill process table
	int i;
	int proc_id = 0;
	for (i = 0; i < num_keyboard_processes; i++) {
		PCBStr *p = make_process(proc_id, KEYBOARD);
		p->device = KB_DEVICE;
		all_pcbs[proc_id] = p;
		proc_id++;
	}

	for (i = 0; i < num_io_processes; i++) {
		PCBStr *p = make_process(proc_id, IO);
		p->device = IO_DEVICE;
		all_pcbs[proc_id] = p;
		proc_id++;
	}

	int num_pairs = 0;
	for (i = 0; i < num_pc_processes; i++) {
		PCBStr *p = make_process(proc_id, PRODUCER);
		p->mutex = num_pairs;
		p->mem_loc = num_pairs;
		all_pcbs[proc_id] = p;
		proc_id++;

		PCBStr *c = make_process(proc_id, CONSUMER);
		c->mutex = num_pairs;
		c->mem_loc = num_pairs;
		all_pcbs[proc_id] = c;
		proc_id++;
		num_pairs++;
	}

	for (i = 0; i < num_compute_processes; i++) {
		PCBStr *p = make_process(proc_id, COMPUTE);
		p->device = IO_DEVICE;
		all_pcbs[proc_id] = p;
		proc_id++;
	}

	current_pcb = all_pcbs[0];
	current_process = current_pcb->proc;

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
	queue process_blocked_on_devices[num_devices];
	for (int i = 0; i < num_devices; i++)
	{
		buildQueue(BLOCK_QUEUE_LENGTH, process_blocked_on_devices[i]);
	}
	printf("OK, created the device blocking queue\n");

	// Initialize mutexes and blocked-process queue for mutexes
	int mutexes[num_mutexes];		// keeps track of current mutex holder, -1 = nobody
	queue processes_blocked_on_mutexes[num_mutexes];	// keeps track of who's waiting to grab a mutex
	for (int i = 0; i < num_mutexes; i++)
	{
		mutexes[i] = NOBODY_HOLDS_MUTEX;
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_mutexes[i]);
	}
	printf("OK, created the mutex blocking queue\n");

	// Initialize shared memory locations and blocked-process queue for same
	int shared_mem[num_mem_locs];
	queue processes_blocked_on_shared_mem[num_mem_locs];
	for (int i = 0; i < num_mem_locs; i++)
	{
		shared_mem[i] = 0;	// all shared memory will start at zero
		buildQueue(BLOCK_QUEUE_LENGTH, processes_blocked_on_shared_mem[i]);
	}
	printf("OK, created the shared memory\n");

	while (TRUE) {
		printf("got to the first step\n");
		usleep(100000); // Run 10 steps/second, slowed down enough to be capable of human comprehension
		printf("whoop whoop\n");
		// sources of interrupts: timer, I/O devices, system calls

		while (global_interrupt_state == 0 && current_pcb->state == RUNNING) {

			// read instruction
			int instruction = current_process->requests[current_pcb->next_step];
			// int argument = current_process->requests[current_pcb->next_step + 1];

			// switch statement that determines what to do based on instruction in pc
			switch(instruction)
			{
			case INSTRUCTION_OUTPUT:	// aka "system call"
				// We're blocked waiting for the output operation to finish:
				current_pcb->state = BLOCKED;
				// We need to add ourselves to the queue for the device:
				addToEnd(process_blocked_on_devices[current_pcb->device], current_pcb->pid);
				// If we're not waiting on a keyboard input, then fire up a one-shot thread that will
				// fire an interrupt back after a short time.
				if (current_pcb->device != KB_DEVICE)
				{
					pthread_t temp_input_thread;
					pthread_create(&temp_input_thread, NULL, io_interrupt_fp, NULL);
				}
				break;
			case INSTRUCTION_MUTEX_LOCK:
				// Take the mutex:
				if (mutexes[current_pcb->mutex] == NOBODY_HOLDS_MUTEX)
					mutexes[current_pcb->mutex] = current_pcb->pid;
				// Block and wait for the mutex to be free:
				else
				{
					current_pcb->state = BLOCKED;
					addToEnd(processes_blocked_on_mutexes[current_pcb->mutex], current_pcb->pid);
				}
				break;
			case INSTRUCTION_MUTEX_UNLOCK:
				if (mutexes[current_pcb->mutex] == current_pcb->pid)
				{
					mutexes[current_pcb->mutex] = NOBODY_HOLDS_MUTEX;
					// If anybody's waiting on this:
					if (processes_blocked_on_mutexes[current_pcb->mutex].position >= 0)
					{
						// find out who they are
						int waiting_process = getFirstItem(processes_blocked_on_mutexes[current_pcb->mutex]);
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
				if (shared_mem[current_pcb->mem_loc] == 0)
				{
					int waiting_process = getFirstItem(processes_blocked_on_shared_mem[current_pcb->mem_loc]);
					// and wake them up...
					all_pcbs[waiting_process]->state = RUNNING;
				}
				// Actually increment the memory location
				shared_mem[current_pcb->mem_loc]++;
				break;
			case INSTRUCTION_DEC_SHARED_MEM:
				// If shared memory location is zero:
				//   Block and wait
				// If shared memory is not zero:
				//	 Decrement and continue
				if (shared_mem[current_pcb->mem_loc] == 0)
				{
					current_pcb->state = BLOCKED;
					addToEnd(processes_blocked_on_shared_mem[current_pcb->mem_loc], current_pcb->pid);
				}
				else
				{
					shared_mem[current_pcb->mem_loc]--;
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
				current_pcb->next_step = (current_pcb->next_step + 1) % current_process->no_steps;
		}

		// handle interrupts
		// =================

		pthread_mutex_lock(&interrupt_mutex);
		// bitmask on timer interrupt
		if (global_interrupt_state & TIMER_INTERRUPT)
		{
			global_interrupt_state -= TIMER_INTERRUPT;
			printf("Timer interrupt received!\n");
			// all we need to do is run the scheduler
			run_scheduler = TRUE;
		}

		// bitmask on keyboard interrupt
		if (global_interrupt_state & KEYBOARD_INTERRUPT)
		{
			global_interrupt_state -= KEYBOARD_INTERRUPT;
			printf("Keyboard interrupt received!\n");
			// Wake up the next process waiting on the keyboard
			if (process_blocked_on_devices[0].position != -1)
			{
				int waiting_process = getFirstItem(process_blocked_on_devices[0]);
				// and wake it up... (note that we aren't immediately running the scheduler, it just is
				// eligible to be scheduled!)
				all_pcbs[waiting_process]->state = RUNNING;
			}
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

		printf("\n Please enter total number of keyboard processes to run: ");
		scanf("%d", &num_keyboard_processes);
		FLUSH;
		printf("\n Please enter total number of I/O bound processes to run: ");
		scanf("%d", &num_io_processes);
		FLUSH;
		printf("\n Please enter total number of p/c processes to run: ");
		scanf("%d", &num_pc_processes);
		FLUSH;

		printf("\n Please select the scheduling algorithm to use: ");
		printf("\n 1. Round Robin");
		printf("\n 2. Lottery");
		printf("\n 3. Priority\n");
		scanf("%d", &scheduler_choice);

}

// Timer interrupt thread function - sleep for one half second, set timer interrupt on, repeat as long as global
// run state is true.
// TODO: pass in defined sleep time
void timer_interrupt()
{
	while(global_run_state == TRUE)
	{

		usleep(5000000);	// 5 seconds
		pthread_mutex_lock(&interrupt_mutex);
		if (!(global_interrupt_state & TIMER_INTERRUPT))
			global_interrupt_state += TIMER_INTERRUPT;
		pthread_mutex_unlock(&interrupt_mutex);
		printf("TICK TOCK MOTHAFUCKA\n");
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
		printf("WHY YOU GOTTA TYPE LIKE THAT?\n");
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
