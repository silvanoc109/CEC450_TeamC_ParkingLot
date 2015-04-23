
/* LEFT TO BE DONE

- check address of sysOutBite and SysInByte
- semaphore/message queue for critical aerea parking lot count
- max cars in parking >> not letting them pass when you reach that number
- when parking lot  = 0 not letting car out
- add watch dogs (too much time gate opened for example)
 */

#include <vxWorks.h> /* Always include this as the first thing in every program */
#include <stdio.h> /* we use printf */
#include <sysLib.h> /* we use sysClk... */
#include <taskLib.h> /* we use tasks... */
#include <msgQLib.h> /* we use message queues */
#include <time.h> /* we use timespec */

/*constants of limit of messages*/
#define MAX_MESSAGES 10
#define MAX_MESSAGE_LENGTH 10

/*function prototypes*/
void setActuatorValues(int iActuators);
void gateIn(void);
void gateOut(void);
void openGate(int inOrOut);
void closeGate(int inOrOut);
int getSensorValues();
void inputWatcher();
void initiateProcess();
void simulator();

/*global variables for message queues*/
MSG_Q_ID gateInQueue;
MSG_Q_ID gateOutQueue;

int iActuators;
int parkingLotCount = 50; 
int oldSensors;
int newSensors = 0;
int newCarOut;
int oldCarOut;
int oldCarIn;
int newCarIn;

void setActuatorValues(int iActuators)
{ 
	sysOutByte(0x184,0x01); /* Re enable outputs */
	sysOutByte(0x181,(iActuators & 0x007F)); /*We only have 6 actuators*/
}

void gateIn(void)
{

	char msgBuf[MAX_MESSAGE_LENGTH];

	while(1)
	{
		/*obtain new values of sensors*/
		/*	newSensors = getSensorValues(); */

		/* receieve message from gete in queue*/
		if(msgQReceive(gateInQueue, msgBuf, MAX_MESSAGE_LENGTH,
				WAIT_FOREVER) == ERROR){
			printf("msgQReceive - gate in queue\n");
		}else{

			if(newSensors != oldSensors)
			{
				/*IN*/
				oldCarIn = oldSensors & 0x0010;
				newCarIn = newSensors & 0x0010;
				if(oldCarIn != newCarIn)   
				{

					/* A car has arrived or a car has left */

					if(newCarIn > oldCarIn)  /* A car has arrived*/
					{
						/*check lot count*/
						openGate(1); /* 1 = in*/
						printf("A CAR WANTS TO ENTER THE LOT \n");
						taskDelay(10);
					}
					else /* a car has succesfullly entered*/
					{
						closeGate(1); /* 1 = in*/
						printf("A CAR ENTERED THE LOT!\n");
						/*semaphore*/
						parkingLotCount++;

						taskDelay(10);
					}
				}


			}

			/*set current value to sensors as the old value*/
			/*	oldSensors = newSensors;*/


		}

	}

}

void gateOut(void)
{

	char msgBuf[MAX_MESSAGE_LENGTH];

	while(1)
	{
		/*obtain new values of sensors*/
		/*newSensors = getSensorValues();*/

		/* receieve message from gete out queue*/
		if(msgQReceive(gateOutQueue, msgBuf, MAX_MESSAGE_LENGTH,
				WAIT_FOREVER) == ERROR){
			printf("msgQReceive - gate in queue\n");
		}else{

			if(newSensors != oldSensors)
			{		
				/*OUT*/
				oldCarOut = oldSensors & 0x0020;
				newCarOut = newSensors & 0x0020;
				if(oldCarOut != newCarOut)
				{

					/* A car has arrived or a car has left of the barrier */

					if(newCarOut > oldCarOut)  /* A car has arrived to the gate*/
					{
						openGate(0); /*0 = out*/
						printf("A CAR WANTS TO LEAVE THE LOT \n");
						taskDelay(10);
					}
					else
					{
						closeGate(0); /*0=Out*/
						printf("A CAR LEFT THE LOT!\n");
						parkingLotCount--;
						taskDelay(10);
					}
				}
			}
			/*set current value to sensors as the old value*/
			/*oldSensors = newSensors;*/

		}
	}
}

void openGate(int inOrOut)
{

	if(inOrOut == 1) /*1  = gate in */
	{
		printf("GATE IN\n");
		/*ACTUATOR BARRIER UP*/
		iActuators = (iActuators | 0x01); /*In barrier up bit is 0x01 is that one. We made the or to not change the other bits*/
		setActuatorValues(iActuators);
		/*WHEN SENSOR BARRIER UP LIGHT GREEN ON RED LIGHT OFF*/

		/*DO NOTHING >> waiting until the barrier is up*/
		/*THIS IS A PROBLEM - THIS IS LOOP NEVER RUNS - LIGHT SHOULD SWITCH AFTER DELAY */
		while((newSensors & 0x0001) != 1)
		{

		}

		iActuators = (iActuators & 0xFFFB); /*RED LIGHT OFF*//* we want the bit 0x04 in 0. that is FFF[1011] = FFFB*/
		iActuators = (iActuators | 0x0008); /*GREEN LIGHT ON*//*we want the bit 0x08 in 1.  */
		setActuatorValues(iActuators);	
	}
	if(inOrOut == 0)  /* 0 = gate out*/
	{
		printf("GATE OUT\n");
		/*ACTUATOR BARRIER UP*/
		iActuators = (iActuators | 0x02); /*OUT barrier up bit is 0x02 is that one. We made the or to not change the other bits*/
		setActuatorValues(iActuators);


		/*WHEN SENSOR BARRIER UP LIGHT RED OFF, GREEN ON*/
		/*THIS IS A PROBLEM - THIS IS LOOP NEVER RUNS - LIGHT SHOULD SWITCH AFTER DELAY */
		while((newSensors & 0x0002) != 1)
		{
			/*DO NOTHING >> waiting until the barrier is up*/
		}

		/*PROBLEM - THIS NEVER RUNS!!!*/

		/*ATTEMPTING TO SWITCH LIGHTS ON OUT GATE OPENING*/
		iActuators = (iActuators & 0xFFEF); /*RED LIGHT OFF*//* we want the bit 0x0010 in 0. that is FF[1110]F > FFFE */
		iActuators = (iActuators | 0x0020); /*GREEN LIGHT ON*//*we want the bit 0x20 in 1. 00[0010]0 */
		setActuatorValues(iActuators);

	}

}

void closeGate(int inOrOut)
{
	if(inOrOut == 1)
	{
		printf("GATE IN\n");		
		/*LIGHT GREEN OFF, RED ON*/
		iActuators = (iActuators & 0xFFF7); /*GREEN LIGHT OFF*//*we want the bit 0x08 in 0. 1111 0111 > F7  */
		iActuators = (iActuators | 0x0004); /*RED LIGHT ON*//* we want the bit 0x04 in 1. that is 000[0100] = 0004*/

		/*ACTUATOR BARRIER DOWN*/
		iActuators = (iActuators & 0xFFFE); /*Out barrier up bit is 0x01. We want that one to be 0. FFF[1110] > FFFE}*/
		setActuatorValues(iActuators);
	}
	if(inOrOut == 0)
	{
		printf("GATE OUT\n");
		/*LIGHT GREEN OFF, RED ON*/
		iActuators = (iActuators & 0xFFDF); /*GREEN LIGHT OFF*//*we want the bit 0x20 in 0. FF[1101]F > FFDF */
		iActuators = (iActuators | 0x0010); /*RED LIGHT ON*//* we want the bit 0x0010 in 1. that is 00[0001]0 > 0010 */

		/*ACTUATOR BARRIER DOWN*/
		iActuators = (iActuators & 0xFFFD); /*Out barrier up bit is 0x02. We want that one to be 0. FFF[1101]}*/
		setActuatorValues(iActuators);
	}

}

int getSensorValues()
{ 
	int iSensors = 0;
	sysOutByte(0x184,0x00); /* Disable outputs */
	iSensors = (sysInByte(0x182) << 8) | (sysInByte(0x180));
	sysOutByte(0x184,0x01); /* Re enable outputs */
	return iSensors;
}

/*TASK 0*/
void inputWatcher()
{ 

	char message[MAX_MESSAGE_LENGTH];

	while (1)
	{
		newSensors = getSensorValues(); /*read the sensors*/

		if(newSensors != oldSensors){

			/*create message and send it through the queue*/
			sprintf(message, "CHANGE %d", newSensors);

			if((msgQSend(gateInQueue, message, MAX_MESSAGE_LENGTH, 
					WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
				printf("msgQSend to gate in failed\n");

			/*both tasks wont work together - even with this message queue*/

			if((msgQSend(gateOutQueue, message, MAX_MESSAGE_LENGTH, 
					WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
				printf("msgQSend to gate out failed\n");

		}
		/*
if(newSensors != oldSensors){
gateIn();
gateOut();
}
		 */

		taskDelay(6); /*Delay 6/60 -> 1/10 of a second*/
		oldSensors = newSensors; /*update the sensors*/

	}
}

void initiateProcess()
{ 
	sysOutByte(0x180,0x00); /* Zero out the four channels */
	sysOutByte(0x181,0x00);
	sysOutByte(0x182,0x00);
	sysOutByte(0x183,0x00);
	sysOutByte(0x184,0x01); /* Enable Output */
	iActuators = 0;
}

void simulator()
{ 
	initiateProcess();
	oldSensors = getSensorValues();

	/*create both message queues*/
	if ((gateInQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for read to gate in queue\n");

	if ((gateOutQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for update to gate out queue\n");

	if((taskSpawn("TASK 0",70,0x100,2000,(FUNCPTR)inputWatcher,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning input watcher task.\n");
	}

	/*TASK 1 = gate in*/
	if((taskSpawn("GATE IN",70,0x100,2000,(FUNCPTR)gateIn,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning gateIn task.\n");
	}

	/*TASK 2 = gate out*/
	if((taskSpawn("GATE OUT",70,0x100,2000,(FUNCPTR)gateOut,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning gateOut task.\n");
	}

}

/*
void deleteAll()
{ 
	td(TASK);
	td(GATE IN);
	td(GATE OUT);
}
 */
