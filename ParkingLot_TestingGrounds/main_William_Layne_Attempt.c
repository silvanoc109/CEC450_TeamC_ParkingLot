/* TO-DO:
    -check output port for bytes
    -semaphore
    -watchdog
*/

#include <vxWorks.h> /* Always include this as the first thing in every program */
#include <stdio.h> /* we use printf */
#include <sysLib.h> /* we use sysClk... */
#include <taskLib.h> /* we use tasks... */
/*Sensor inputs*/
#define IN_BARRIER_DOWN_S   0x01
#define	IN_BARRIER_UP_S     0x02
#define	OUT_BARRIER_DOWN_S	0x04
#define	OUT_BARRIER_UP_S	0x08
#define	IN_CAR	            0x10
#define	OUT_CAR             0x20
/*Actuator outputs*/
#define IN_BARRIER_UP_A	    0x01
#define	OUT_BARRIER_UP_A	0x04
#define	IN_RED	            0x08
#define	IN_GREEN	        0x10
#define	OUT_RED	            0x10
#define	OUT_GREEN	        0x20
/*grouped sensors*/
#define IN_SENSORS          0x13
#define OUT_SENSORS         0x2A
/*constants of limit of messages*/
#define MAX_MESSAGES        10
#define MAX_MESSAGE_LENGTH  33


/*function prototypes*/
void inGate(void);
void outGate(void);
void setActuatorValues(int iActuators);
int getSensorValues();
void initiateProcess(void);
int main();

/*enumeration for the states of the system*/
enum STATE {
    WAITING,
    OPENING,
    OPENED,
    CLOSING
    };
/*global variables*/
int iActuators;
int newSensors = 0;
int oldSensors;
/*global variables for message queues*/
MSG_Q_ID inGateQueue;
MSG_Q_ID outGateQueue;
/*global variables for state enumerations*/
enum STATE inState;
enum STATE outState;

/*This function is the control flow for the IN Gate.
This function waits to receive a message from the control function and then checks the state
The function then checks if the condition for the next state is met. If it is not, nothing happens.
Then it loops back and waits for the next message.*/
void inGate(){
    char msgBuf[MAX_MESSAGE_LENGTH];
    while(1){
        if(msgQReceive(inGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: IN message queue\n");
        } else {
            if(inState == WAITING){ /*if the in state is set to waiting, then check condition for next state*/
                if( (newSensors & (IN_CAR | IN_BARRIER_DOWN_S) == (IN_CAR | IN_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
                    iActuators = iActuators | IN_BARRIER_UP_A;   /*set barrier to up in actuators*/
                    setActuatorValues(iActuators);
                    inState = OPENING;
                    printf("IN: State switched from WAITING to OPENING");
                }
            } else if(inState == OPENING){  /*if the in state is set to opening, then check condition for next state*/
                if( (newSensors & (IN_BARRIER_UP_S | IN_CAR) ) == (IN_CAR | IN_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/
                    iActuators = iActuators & (~IN_RED);         /*clear red actuator bit*/
                    iActuators = iActuators | IN_GREEN;          /*set green to on in actuators*/
                    setActuatorValues(iActuators);
                    inState = OPENED;
                    printf("IN: State switched from OPENING TO OPENED");
                }
            } else if(inState == OPENED){ /*if the in state is set to opened, then check condition for next state*/
                if( (newSensors & (IN_CAR | IN_BARRIER_UP_S)) == (IN_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/
                    iActuators = iActuators | IN_RED;               /*set red light on in actuators*/
                    iActuators = iActuators & ~(IN_GREEN);          /*set green light off in actuators*/
                    iActuators = iActuators & ~(IN_BARRIER_UP_A);   /*set barrier to lower in actuators*/
                    setActuatorValues(iActuators);
                    inState = CLOSING;
                    printf("IN: State switched from OPENING to CLOSING");
                }
            } else if(inState == CLOSING){ /*if the in state is set to closing, then check condition for next state*/
                if((newSensors & (IN_CAR | IN_BARRIER_DOWN_S)) == (IN_BARRIER_DOWN_S)){ /*if there is no car at the barrier, and the barrier is down: continue*/
                    inState = WAITING;
                    printf("IN: State switched from CLOSING to WAITING");
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
    while(1){
        if(msgQReceive(outGateQueue, msgBuf, MAX_MESSAGE_LENGTH, WAIT_FOREVER) == ERROR){
			printf("ERROR: out message queue\n");
        } else {
            if(outState == WAITING){ /*if the out state is set to waiting, then check condition for next state*/
                if( (newSensors & (OUT_CAR | OUT_BARRIER_DOWN_S) == (OUT_CAR | OUT_BARRIER_DOWN_S) ){   /*if there is a car at the barrier and the barrier is down: continue*/
                    iActuators = iActuators | OUT_BARRIER_UP_A;   /*set barrier to up in actuators*/
                    setActuatorValues(iActuators);
                    outState = OPENING;
                    printf("OUT: State switched from WAITING to OPENING");
                }
            } else if(outState == OPENING){  /*if the out state is set to opening, then check condition for next state*/
                if( (newSensors & (OUT_BARRIER_UP_S | OUT_CAR) ) == (OUT_CAR | OUT_BARRIER_UP_S) ){ /*if there is a car at the barrier, and the barrier is up: continue*/
                    iActuators = iActuators & (~OUT_RED);         /*clear red actuator bit*/
                    iActuators = iActuators | OUT_GREEN;          /*set green to on in actuators*/
                    setActuatorValues(iActuators);
                    outState = OPENED;
                    printf("OUT: State switched from OPENING TO OPENED");
                }
            } else if(outState == OPENED){ /*if the out state is set to opened, then check condition for next state*/
                if( (newSensors & (OUT_CAR | OUT_BARRIER_UP_S)) == (OUT_BARRIER_UP_S) ){ /*if there is no car at the barrier, and the barrier is up: continue*/
                    iActuators = iActuators | OUT_RED;               /*set red light on in actuators*/
                    iActuators = iActuators & ~(OUT_GREEN);          /*set green light off in actuators*/
                    iActuators = iActuators & ~(OUT_BARRIER_UP_A);   /*set barrier to lower in actuators*/
                    setActuatorValues(iActuators);
                    outState = CLOSING;
                    printf("OUT: State switched from OPENING to CLOSING");
                }
            } else if(outState == CLOSING){ /*if the in state is set to closing, then check condition for next state*/
                if((newSensors & (OUT_CAR | OUT_BARRIER_DOWN_S)) == (OUT_BARRIER_DOWN_S)){ /*if there is no car at the barrier, and the barrier is down: continue*/
                    outState = WAITING;
                    printf("OUT: State switched from CLOSING to WAITING");
                }
            }
        }
    }
}

/*Thread to watch the sensors and send a message of events to the correct gate*/
void inputWatcher(){
    char message[MAX_MESSAGE_LENGTH];
    while(1){
        newSensors = getSensorValues();
        if(newSensors != oldSensors){   /*if the sensors have changed from the old one then continue*/
                if( ( newSensors & IN_SENSORS ) != ( oldSensors & IN_SENSORS ) ){ /*if in gate sensors have changed:*/

                    sprintf(message, "IN:CHANGE FROM %d TO %d", oldSensors&IN_SENSORS, newSensors&IN_SENSORS); /*create message*/

                    if((msgQSend(inGateQueue, message, MAX_MESSAGE_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
                        printf("msgQSend to gate in failed\n");
                    }
                }
                if ( (newSensors & OUT_SENSORS) != (oldSensors & OUT_SENSORS) ){

                    sprintf(message, "OUT:CHANGE FROM %d TO %d", oldSensors&OUT_SENSORS, newSensors&OUT_SENSORS); /*create message*/

                    if((msgQSend(outGateQueue, message, MAX_MESSAGE_LENGTH, WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
                        printf("msgQSend to gate out failed\n");
                    }
                }

        }
    }
}


/*Set the actuator values by sending the current actuator values to a port*/
void setActuatorValues(int iActuators)
{
	sysOutByte(0x184,0x01); /* Re enable outputs */
	sysOutByte(0x180,(iActuators & 0x003F)); /*TO-DO MAKE SURE THAT THIS PORT IS CORRECT*//*send the lowest 6 bits of iActuators to memory location 0x181*/
}

/*Get the current sensor values by receiving the value from a port*/
int getSensorValues()
{
    int iSensors = 0;
    sysOutByte(0x184,0x00); /* Disable outputs */
    iSensors = (sysInByte(0x180)); /*TO-DO MAKE SURE THAT THIS PORT IS CORRECT*/ /*Receive value from 0x180 as we only need 2 bytes*/
    sysOutByte(0x184,0x01); /* Re enable outputs */
    return iSensors;
}

/*Set all ports to 0 and set actuators to both lights red.*/
void initiateProcess()
{
    sysOutByte(0x180,0x00); /* Zero out the four channels */
    sysOutByte(0x181,0x00);
    sysOutByte(0x182,0x00);
    sysOutByte(0x183,0x00);
    sysOutByte(0x184,0x01); /* Enable Output */
    iActuators = (IN_RED | OUT_RED);
    oldSensors = getSensorValues();
}

int main()
{
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
