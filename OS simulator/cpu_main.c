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
