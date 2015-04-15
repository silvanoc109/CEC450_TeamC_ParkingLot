#include <vxWorks.h> /* Always include this as the first thing in every program */
#include <stdio.h> /* we use printf */
#include <sysLib.h> /* we use sysClk... */
#include <taskLib.h> /* we use tasks... */
#include <msgQLib.h> /* we use message queues */
#include <time.h> /* we use timespec */

/*constants on limit of messages*/
#define MAX_MESSAGES 100
#define MAX_MESSAGE_LENGTH 10

/*global for actuator values*/
int iActuators;

/*ADDED COUNT FOR PARKING LOT*/
int parkingLotCount;

/*global for tick of target on execution*/
int startingTick;

/*global variables for message queues*/
MSG_Q_ID readToUpdateQueue;
MSG_Q_ID updateToDisplayQueue;

void setActuatorValues(int iActuators)
{
	sysOutByte(0x184,0x01); /* Re enable outputs */
	sysOutByte(0x183,(iActuators & 0xFF00) >> 8);
	sysOutByte(0x181,(iActuators & 0x00FF));
}

void calculateAndSetActuators(void)
{
	int currentActuatorValue = 0;
	int incomingValue = 0;
	char msgBuf[MAX_MESSAGE_LENGTH];
	char message[MAX_MESSAGE_LENGTH];

	while(1){

		/* receieve message from read sensor queue*/
		if(msgQReceive(readToUpdateQueue, msgBuf, MAX_MESSAGE_LENGTH,
				WAIT_FOREVER) == ERROR){
			printf("msgQReceive from read to update failed\n");
		}else{

			/*convert from string to integer with value of actuator*/
			incomingValue = atoi(msgBuf);

			/*check if actuator value has changed*/
			/*update and set if and ONLY IF value has changed*/
			if(incomingValue != currentActuatorValue){

				/*set the actuator values*/
				setActuatorValues(iActuators);

				/*create message and send it through the queue*/
				sprintf(message, "%d", incomingValue);

				if((msgQSend(updateToDisplayQueue, message, MAX_MESSAGE_LENGTH,
						WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
					printf("msgQSend from update to display failed\n");

				/*set new value as actuator value*/
				currentActuatorValue = incomingValue;
			} 

		}
	} /*end while loop*/
}

int getSensorValues(void)
{
	int iSensors = 0;
	sysOutByte(0x184,0x00); /* Disable outputs */
	iSensors = (sysInByte(0x182) << 8) | (sysInByte(0x180));
	sysOutByte(0x184,0x01); /* Re enable outputs */

	return iSensors;
}

void displayStatus(int inputs, int iActuators)
{
	int incomingValue = 0;

	/* ADDED FOR FINAL PROJECT CHECK STATUS OF GATES */
	int newInGate = 0;
	int currentInGate = 100;
	int newOutGate = 0;
	int currentOutGate = 100;

	int parkingLotCount = 0;

	struct timespec tstamp;
	char msgBuf[MAX_MESSAGE_LENGTH];
	tstamp.tv_sec = 0;
	tstamp.tv_nsec = 0;

	while(1){

		/* receieve message from update sensor queue*/
		if(msgQReceive(updateToDisplayQueue, msgBuf, MAX_MESSAGE_LENGTH,
				WAIT_FOREVER) == ERROR){
			printf("msgQReceive from update to display failed\n");
		}else{

			/*convert from string to integer with value of actuator*/
			incomingValue = atoi(msgBuf);

			/*update new the in and out values on the sensor*/
			newInGate = incomingValue & 0x0010;
			newOutGate = incomingValue & 0x0020;

			if(newInGate < currentInGate){
				printf("A CAR ENTERED THE LOT!\t");

				/*get current tick of clock and subtract from tick on start*/
				printf("%d tick\t", tickGet() - startingTick);
				parkingLotCount++;
				printf("Cars in the lot: %d\n", parkingLotCount);
			}

			if(newOutGate < currentOutGate){
				printf("A CAR EXITED THE LOT!\t");

				/*get current tick of clock and subtract from tick on start*/
				printf("%d tick\t", tickGet() - startingTick);
				parkingLotCount--;
				printf("Cars in the lot: %d\n", parkingLotCount);
			}

			/*set the current value of the in and out sensors */
			currentInGate = newInGate;
			currentOutGate = newOutGate;

			/*print value of input and actuator being modified*/
			/*
			printf("\rInputs: %04x", incomingValue);
			printf(" Actuators %04x", incomingValue);
			*/

			/*timestamp of change in secs and nsecs*/
			/*
			clock_gettime(CLOCK_REALTIME, &tstamp);
			printf(" - %d sec %d nsec - ", (int) tstamp.tv_sec, (int) tstamp.tv_nsec);
			*/

			/*get current tick of clock and subtract from tick on start*/
			/*printf("%d tick\n", tickGet() - startingTick);*/

		}
	}
}

void inputWatcher(void)
{
	int inputs = 0;
	char message[MAX_MESSAGE_LENGTH];

	/*continue to read sensor values - delay in loop*/
	while (1){
		inputs = getSensorValues();

		/*create message and send it through the queue*/
		sprintf(message, "%d", inputs);

		if((msgQSend(readToUpdateQueue, message, MAX_MESSAGE_LENGTH,
				WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
			printf("msgQSend from read to update failed\n");
	
		taskDelay(6); /*Delay 6/60 -> 1/10 of a second*/
	}
}

void initiateProcess(void)
{
	sysOutByte(0x180,0x00); /* Zero out the four channels */
	sysOutByte(0x181,0x00);
	sysOutByte(0x182,0x00);
	sysOutByte(0x183,0x00);
	sysOutByte(0x184,0x01); /* Enable Output */
	iActuators = 0;
}

/*main function to run for part B*/
void execSimulator(void)
{
	/*declare timespec */
	struct timespec tstamp;

	tstamp.tv_sec = 0;
	tstamp.tv_nsec = 0;

	/* initializes the kernel clock to zero */
	clock_settime(CLOCK_REALTIME, &tstamp);

	/*get the current tick of the clock*/
	startingTick = tickGet();

	/*initialization function to clear values*/
	initiateProcess();

	/*create both message queues*/
	if ((readToUpdateQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, 
			MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for read to update queue\n");

	if ((updateToDisplayQueue = msgQCreate(MAX_MESSAGES, MAX_MESSAGE_LENGTH, 
			MSG_Q_FIFO)) == NULL)
		printf("msgQCreate failed for update to display queue\n");

	/*spawn three tasks*/
	/*task that used to listen for input*/
	if((taskSpawn("inputWatcher",70,0x100,2000,(FUNCPTR)inputWatcher,0,0,0,0,0,0,0,0,0,0))
			== ERROR){
		printf("Error spawning input watcher task.");
	}

	/*task used to set actuator values*/
	/*must call calculateAndSetActuators first*/
	if((taskSpawn("setActuators",70,0x100,2000,(FUNCPTR)calculateAndSetActuators,0,0,0,0,0,0,0,0,0,0))
			== ERROR){
		printf("Error spawning calculate and set actuators task.");
	}

	/*task used to display status of system*/
	if((taskSpawn("displayStatus",70,0x100,2000,(FUNCPTR)displayStatus,0,0,0,0,0,0,0,0,0,0))
			== ERROR){
		printf("Error spawning display sensor values task.");
	}

}
