/* TO-DO:
    -check output port for bytes
    -semaphore
    -watchdog
    -timestamping - ticks and secs/nsecs
 */

#include <vxWorks.h>		/*vxWorks functionality*/
#include <stdio.h>			/*printf capability*/
#include <sysLib.h>			/*sysClk capability*/
#include <taskLib.h>		/*tasks capability*/
#include <msgQLib.h>		/*message queue capability*/
#include <time.h>		/*timespec capability*/
#include "parkingHeader.h"	/*personal defined header file*/

/*This function is the control flow for the IN Gate.
This function waits to receive a message from the control function and then checks the state
The function then checks if the condition for the next state is met. If it is not, nothing happens.
Then it loops back and waits for the next message.*/
void inGate(){
	char msgBuf[MAX_MESSAGE_LENGTH];

	/*declare timespec for time stamp*/
	struct timespec timeStamp;
	timeStamp.tv_sec = 0;
	timeStamp.tv_nsec = 0;

	while(1){
		if(msgQReceive(inGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: IN message queue\n");
		} else {
			if(inState == WAITING){ /*if the in state is set to waiting, then check condition for next state*/
				if( (newSensors & (IN_CAR | IN_BARRIER_DOWN_S)) == (IN_CAR | IN_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
					iActuators = iActuators | IN_BARRIER_UP_A;   /*set barrier to up in actuators*/
					setActuatorValues(iActuators);
					inState = OPENING;
					printf("IN: State switched from WAITING to OPENING\n");
					
					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(inState == OPENING){  /*if the in state is set to opening, then check condition for next state*/
				if( (newSensors & (IN_BARRIER_UP_S | IN_CAR) ) == (IN_CAR | IN_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/
					iActuators = iActuators & (~IN_RED);         /*clear red actuator bit*/
					iActuators = iActuators | IN_GREEN;          /*set green to on in actuators*/
					setActuatorValues(iActuators);
					inState = OPENED;
					printf("IN: State switched from OPENING TO OPENED\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(inState == OPENED){ /*if the in state is set to opened, then check condition for next state*/
				if( (newSensors & (IN_CAR | IN_BARRIER_UP_S)) == (IN_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/
					iActuators = iActuators | IN_RED;               /*set red light on in actuators*/
					iActuators = iActuators & ~(IN_GREEN);          /*set green light off in actuators*/
					iActuators = iActuators & ~(IN_BARRIER_UP_A);   /*set barrier to lower in actuators*/
					setActuatorValues(iActuators);
					inState = CLOSING;
					printf("IN: State switched from OPENING to CLOSING\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(inState == CLOSING){ /*if the in state is set to closing, then check condition for next state*/
				if((newSensors & (IN_CAR | IN_BARRIER_DOWN_S)) == (IN_BARRIER_DOWN_S)){ /*if there is no car at the barrier, and the barrier is down: continue*/
					inState = WAITING;
					printf("IN: State switched from CLOSING to WAITING\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			}
		}
	}
}

/*This function is the control flow for the O Gate.
This function waits to receive a message from the control function and then checks the state
The function then checks if the condition for the next state is met. If it is not, nothing happens.
Then it loops back and waits for the next message.*/
void outGate(){
	char msgBuf[MAX_MESSAGE_LENGTH];

	/*declare timespec for time stamp*/
	struct timespec timeStamp;
	timeStamp.tv_sec = 0;
	timeStamp.tv_nsec = 0;

	while(1){
		if(msgQReceive(outGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: out message queue\n");
		} else {
			if(outState == WAITING){ /*if the out state is set to waiting, then check condition for next state*/
				if( (newSensors & (OUT_CAR | OUT_BARRIER_DOWN_S)) == (OUT_CAR | OUT_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
					iActuators = iActuators | OUT_BARRIER_UP_A;   /*set barrier to up in actuators*/
					setActuatorValues(iActuators);
					outState = OPENING;
					printf("OUT: State switched from WAITING to OPENING\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(outState == OPENING){  /*if the out state is set to opening, then check condition for next state*/
				if( (newSensors & (OUT_BARRIER_UP_S | OUT_CAR) ) == (OUT_CAR | OUT_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/
					iActuators = iActuators & (~OUT_RED);         /*clear red actuator bit*/
					iActuators = iActuators | OUT_GREEN;          /*set green to on in actuators*/
					setActuatorValues(iActuators);
					outState = OPENED;
					printf("OUT: State switched from OPENING TO OPENED\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(outState == OPENED){ /*if the out state is set to opened, then check condition for next state*/
				if( (newSensors & (OUT_CAR | OUT_BARRIER_UP_S)) == (OUT_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/
					iActuators = iActuators | OUT_RED;               /*set red light on in actuators*/
					iActuators = iActuators & ~(OUT_GREEN);          /*set green light off in actuators*/
					iActuators = iActuators & ~(OUT_BARRIER_UP_A);   /*set barrier to lower in actuators*/
					setActuatorValues(iActuators);
					outState = CLOSING;
					printf("OUT: State switched from OPENING to CLOSING\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(outState == CLOSING){ /*if the in state is set to closing, then check condition for next state*/
				if((newSensors & (OUT_CAR | OUT_BARRIER_DOWN_S)) == (OUT_BARRIER_DOWN_S)){ /*if there is no car at the barrier, and the barrier is down: continue*/
					outState = WAITING;
					printf("OUT: State switched from CLOSING to WAITING\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			}
		}
	}
}

/*Thread to watch the sensors and send a message of events to the correct gate*/
void inputWatcher(){
	char message[MAX_MESSAGE_LENGTH];
	while(1){
		newSensors = getSensorValues(); /*consider the read sensor values new*/
		if(newSensors != oldSensors){   /*if the sensors have changed from the old one then continue*/
			if( ( newSensors & IN_SENSORS ) != ( oldSensors & IN_SENSORS ) ){ /*if in gate sensors have changed:*/

				sprintf(message, "IN:CHANGE FROM %d TO %d\n", oldSensors & IN_SENSORS, newSensors & IN_SENSORS); /*create message*/

				if((msgQSend(inGateQueue, message, MAX_MESSAGE_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
					printf("msgQSend to gate in failed\n");
				}
			}
			if ( (newSensors & OUT_SENSORS) != (oldSensors & OUT_SENSORS) ){

				sprintf(message, "OUT:CHANGE FROM %d TO %d\n", oldSensors & OUT_SENSORS, newSensors & OUT_SENSORS); /*create message*/

				if((msgQSend(outGateQueue, message, MAX_MESSAGE_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
					printf("msgQSend to gate out failed\n");
				}
			}

		}

		taskDelay(6); /*one tenth of a second*/
		oldSensors = newSensors; /*consider the read sensor values old*/
	}
}


/*Set the actuator values by sending the current actuator values to a port*/
void setActuatorValues(int iActuators)
{
	sysOutByte(0x184,0x01); /* Re enable outputs */
	sysOutByte(0x181,(iActuators & 0x003F)); /*send the lowest 6 bits of iActuators to memory location 0x181*/
}

/*Get the current sensor values by receiving the value from a port*/
int getSensorValues()
{
	int iSensors = 0;
	sysOutByte(0x184,0x00); /* Disable outputs */
	iSensors = (sysInByte(0x180)); /*Receive value from 0x180 as we only need 2 bytes*/
	sysOutByte(0x184,0x01); /* Re enable outputs */
	return iSensors;
}

/*Set all ports to 0 and set actuators to both lights red*/
void initiateProcess()
{
	sysOutByte(0x180,0x00); /* Zero out the four channels */
	sysOutByte(0x181,0x00);
	sysOutByte(0x182,0x00);
	sysOutByte(0x183,0x00);
	sysOutByte(0x184,0x01); /* Enable Output */

	/*set actuators to both lights red*/
	iActuators = (IN_RED | OUT_RED);
	setActuatorValues(iActuators);

	/*the first sensor values, considered old, are obtained*/
	oldSensors = getSensorValues();
}

/*command to execute the program*/
int main()
{
	/*declare timespec for time stamp*/
	struct timespec timeStamp;
	timeStamp.tv_sec = 0;
	timeStamp.tv_nsec = 0;

	/*initialize the kernel clock to zero*/
	clock_settime(CLOCK_REALTIME, &timeStamp);

	/*get the current tick of the clock*/
	startingTick = tickGet();

	initiateProcess();
	inState = WAITING;
	outState = WAITING;

	/*create both message queues*/
	if ((inGateQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for read to gate in queue\n");

	if ((outGateQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for update to gate out queue\n");

	if((taskSpawn("INPUT_WATCHER",70,0x100,2000,(FUNCPTR)inputWatcher,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning input watcher task.\n");
	}

	/*TASK 1 = gate in*/
	if((taskSpawn("IN_GATE",70,0x100,2000,(FUNCPTR)inGate,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning in gate task.\n");
	}

	/*TASK 2 = gate out*/
	if((taskSpawn("OUT_GATE",70,0x100,2000,(FUNCPTR)outGate,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning out gate task.\n");
	}
}
