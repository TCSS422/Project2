/*
 *	Dawn Rocks
 *	Maya Osbourne
 *	Mike Baxter Peter Pentescu
 *
 *	TCSS422 Operating Systems
 *	Project 2 - Simulated OS
 */



int PriorityScheduler(int process_array[][], int num_processes)
{
	//Priority can be descided based on memory requirements, time requirements or other
	//resource retirement
	int i = 0;
	int maxPriority;

	for(i = 0; i < num_processes; i ++)
	{
		//for loop through the table - find the max priority
	}
	//Take care of highest priority first, if multiple with same, take care of
	// on a first come first serve basis

}

//This one uses a queue
int RoundRobinScheduler(queue the_queue, int num_processes)
{
	int i = 0;
	int quantum; 	//decide on a quantum number

	//allow process to run while quantum time is still going

	//when quantum time is up -
	//if process is done before time is up then be done with that process
	//else add back to the end of the list and run the next, repeat(while loop)

}

int LotteryScheduler(int process_array[][], int num_processes)
{

	int i = 0;
	int j= 0;
	int winner;
	int totalProcesses = num_processes;
	int totalTickets = 0;

	//need to figure out priority for processes and who gets what amount of tickets.
	// make lottery table
	for(i = 0; i < num_processes; i++)
	{
		// go through each process and assign a lottery ticket for each priority level(priority of X would recieve X amount of tickets)
		//add to lottery table
	}
	 //do a rand() based on the total tickets and pick winner
}
