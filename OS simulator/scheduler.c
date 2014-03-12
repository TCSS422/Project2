/*
 *	Dawn Rocks
 *	Maya Osbourne
 *	Mike Baxter Peter Pentescu
 *
 *	TCSS422 Operating Systems
 *	Project 2 - Simulated OS
 */

#include <process.h>



PCBStr PriorityScheduler(PCBStr ** all_pcbs, int num_processes, int current_pcb)
{
	//Priority can be decided based on memory requirements, time requirements or other
	//resource retirement
	int i = 0;
	PCBStr pcbToRun;
	int maxPriority;


	for(i = 0; i < num_processes; i ++)
	{
		//for loop through the table - find the max priority
		if(all_pcbs[i]->priority > maxPriority && i != current_pcb)
		{
			maxPriority = all_pcbs[i]->priority;
			pcbToRun = all_pcbs[i];
		}
	}
	//Take care of highest priority first, if multiple with same, take care of
	// on a first come first serve basis
	return pcbToRun;
}

//This one uses a queue
PCBStr RoundRobinScheduler(PCBStr ** all_pcbs, int num_processes, int current_pcb)
{
	PCBStr pcbToRun;
	int i = 0;
	int quantum = ; 	//decide on a quantum number

	//allow process to run while quantum time is still going

	//when quantum time is up -
	//if process is done before time is up then be done with that process
	//else add back to the end of the list and run the next, repeat(while loop)
	return pcbToRun;
}

PCBStr LotteryScheduler(PCBStr ** all_pcbs, int num_processes, int current_pcb)
{

	PCBStr pcbToRun;
	int i = 0;
	int o = 0;
	int j = 0;
	int winningPcb;
	int totalTickets = 0;
	int * lotteryArray;
	int rand;
	for(j = 0; j < num_processes; j++)
	{
		totalTickets += all_pcbs[j]->priority;
	}
	lotteryArray = (PCBStr *)malloc(sizeof(PCBStr)) * totalTickets;
	rand = rand_r(totalTickets);
	//need to figure out priority for processes and who gets what amount of tickets.
	// make lottery table
	j = 0;
	for(i = 0; i < num_processes; i++)
	{
		// go through each process and assign a lottery ticket for each priority level(priority of X would recieve X amount of tickets)
		//add to lottery table
		for(o = 0; o < all_pcbs[i]->priority; o++)
		{
			lotteryArray[j] = all_pcbs[i];
			j++;
		}
	}

	winningPcb = lotteryArray[rand];
	free(lotteryArray);
	 //do a rand() based on the total tickets and pick winner
	return pcbToRun;
}
