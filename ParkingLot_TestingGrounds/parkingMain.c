/* TO-DO:
    -check output port for bytes
    -semaphore done
    -watchdog
    -timestamping - ticks and secs/nsecs
 */

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

		wdStart(wdInGate, sysClkRateGet()*2, (FUNCPTR)sendInMessage, 0);

		if(msgQReceive(inGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: IN message queue\n");
		} else {

			if(inState == WAITING){ /*if the in state is set to waiting, then check condition for next state*/
				if( (newSensors & (IN_CAR | IN_BARRIER_DOWN_S)) == (IN_CAR | IN_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
					if(count < MAX_CARS){

						inState = OPENING; /*set current state, the state is now OPENING*/

						iActuators = iActuators | IN_BARRIER_UP_A;   /*set barrier to up in actuators*/
						setActuatorValues(iActuators);

						printf("IN: State switched from WAITING to OPENING\n");
					
						/*print a time stamp*/
						clock_gettime(CLOCK_REALTIME, &timeStamp);
						printf("IN: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					} else {
						/*parking lot is full, send message and do not allow a state change*/
						clock_gettime(CLOCK_REALTIME, &timeStamp);
						printf("\nSYSTEM: Parking lot is full at Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					}
				}
			} else if(inState == OPENING){  /*if the in state is set to opening, then check condition for next state*/
				if( (newSensors & (IN_BARRIER_UP_S | IN_CAR) ) == (IN_CAR | IN_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/

					inState = OPENED; /*set current state, the state is now OPENED*/

					iActuators = iActuators & (~IN_RED);         /*clear red actuator bit*/
					iActuators = iActuators | IN_GREEN;          /*set green to on in actuators*/
					setActuatorValues(iActuators);

					printf("IN: State switched from OPENING TO OPENED\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(inState == OPENED){ /*if the in state is set to opened, then check condition for next state*/
				if( (newSensors & (IN_CAR | IN_BARRIER_UP_S)) == (IN_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/

					inState = WAITING;	/*set current state, the state is now WAITING*/

					iActuators = iActuators | IN_RED;               /*set red light on in actuators*/
					iActuators = iActuators & ~(IN_GREEN);          /*set green light off in actuators*/
					iActuators = iActuators & ~(IN_BARRIER_UP_A);   /*set barrier to lower in actuators*/
					setActuatorValues(iActuators);

					printf("IN: State switched from OPENING to CLOSING\n");

					semTake(semB,WAIT_FOREVER); 	/*take semaphore to be allowed to increment count*/
					count ++;			/*increment count*/
					semGive(semB);			/*give semaphore*/

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("IN: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					printf("\nSYSTEM: Count %d out of %d \n", count, MAX_CARS);
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

		wdStart(wdOutGate, sysClkRateGet()*2, (FUNCPTR)sendOutMessage, 0);

		if(msgQReceive(outGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: out message queue\n");
		} else {
			if(outState == WAITING){ /*if the out state is set to waiting, then check condition for next state*/
				if( (newSensors & (OUT_CAR | OUT_BARRIER_DOWN_S)) == (OUT_CAR | OUT_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
					if(count > 0){ /*if count is over 0 then there are cars to leave*/
					
						outState = OPENING;	/*set current state, the state is now OPENING*/	

						iActuators = iActuators | OUT_BARRIER_UP_A;   /*set barrier to up in actuators*/
						setActuatorValues(iActuators);
						
						printf("OUT: State switched from WAITING to OPENING\n");

						/*print a time stamp*/
						clock_gettime(CLOCK_REALTIME, &timeStamp);
						printf("OUT: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					} else {
						/*parking lot is empty, send message and do not allow a state change*/
						clock_gettime(CLOCK_REALTIME, &timeStamp);
						printf("\nSYSTEM: Parking lot is empty at tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					}
				}
			} else if(outState == OPENING){  /*if the out state is set to opening, then check condition for next state*/
				if( (newSensors & (OUT_BARRIER_UP_S | OUT_CAR) ) == (OUT_CAR | OUT_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/

					outState = OPENED;	/*set current state, the state is now OPENED*/	

					iActuators = iActuators & (~OUT_RED);         /*clear red actuator bit*/
					iActuators = iActuators | OUT_GREEN;          /*set green to on in actuators*/
					setActuatorValues(iActuators);

					printf("OUT: State switched from OPENING TO OPENED\n");

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
				}
			} else if(outState == OPENED){ /*if the out state is set to opened, then check condition for next state*/
				if( (newSensors & (OUT_CAR | OUT_BARRIER_UP_S)) == (OUT_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/
				
					outState = WAITING;		/*set to waiting*/

					iActuators = iActuators | OUT_RED;               /*set red light on in actuators*/
					iActuators = iActuators & ~(OUT_GREEN);          /*set green light off in actuators*/
					iActuators = iActuators & ~(OUT_BARRIER_UP_A);   /*set barrier to lower in actuators*/
					setActuatorValues(iActuators);

					printf("OUT: State switched from OPENING to WAITING\n");

					semTake(semB,WAIT_FOREVER); 	/*take semaphore to be allowed to increment count*/
					count --;			/*increment count*/
					semGive(semB);			/*give semaphore*/

					/*print a time stamp*/
					clock_gettime(CLOCK_REALTIME, &timeStamp);
					printf("OUT: Tick %d - %d sec %d nsec\n", tickGet() - startingTick, (int) timeStamp.tv_sec, (int) timeStamp.tv_nsec);
					printf("\nSYSTEM: Count %d out of %d \n", count, MAX_CARS);
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

/*command to execute the program*/
int main()
{
	/*declare timespec for time stamp*/
	struct timespec timeStamp;
	timeStamp.tv_sec = 0;
	timeStamp.tv_nsec = 0;

	wdInGate = wdCreate();
	wdOutGate = wdCreate();

	/*initialize the kernel clock to zero*/
	clock_settime(CLOCK_REALTIME, &timeStamp);

	/*get the current tick of the clock*/
	startingTick = tickGet();

	initiateProcess();
	inState = WAITING;
	outState = WAITING;
	semB = semBCreate(0,1); /*FIFO, FULL as ti is for sychronization*/
	count = INITIAL_CARS;

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
