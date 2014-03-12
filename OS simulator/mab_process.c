/*
 * process.c  : Implementation of process functions.
 *
 *  Created on: Mar 8, 2014
 *      Author: Group10
 */

#include "process.h"

#define N 15

PCBStr * make_kb_process(int thread_id, INSTRUCTION type) {

}

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
