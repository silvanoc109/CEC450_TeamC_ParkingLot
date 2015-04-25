#include "parkingHeader.h"	/*personal defined header file*/


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

/*This funnction sends a message to the in gate to verify the current state*/
void sendInMessage(){
	char message[MAX_MESSAGE_LENGTH];
	sprintf(message, "IN: TIMEOUT - BARRIER HAS NOT DETECTED CHANGE"); /*create message*/
	printf("\n%s\n", message);
	if((msgQSend(inGateQueue, message, MAX_MESSAGE_LENGTH, 0, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
		printf("ERROR: %s\n", strerror(errnoGet()));
	}	
}

/*This funnction sends a message to the out gate to verify the current state*/
void sendOutMessage(){
	char message[MAX_MESSAGE_LENGTH];
	sprintf(message, "OUT: TIMEOUT - BARRIER HAS NOT DETECTED CHANGE"); /*create message*/
	printf("\n%s\n", message);
	if((msgQSend(outGateQueue, message, MAX_MESSAGE_LENGTH, 0, MSG_PRI_NORMAL)) == ERROR){ /*send message to in gate*/
		printf("msgQSend to gate out failed\n");
	}
}
