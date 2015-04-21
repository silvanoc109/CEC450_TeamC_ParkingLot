
/* LEFT TO BE DONE

- check address of sysOutBite and SysInByte
- semaphore/message queu for critical aerea parking lot count
- max cars in parking >> not letting them pass when you reach that number
- when parking lot  = 0 not letting car out
- add watch dogs (too much time gate opened for example)
*/






#include <taskLib.h> /* we use tasks... */

int iActuators;
int parkingLotCount = 50; 
int oldSensors = getSensorValues();
int newSensors = 0;



void setActuatorValues(int iActuators)
{ 
	sysOutByte(0x184,0x01); /* Re enable outputs */
	SysOutByte(0x181,(iActuators & 0x007F)); /*We only have 6 actuators*/
}

void gateIn(void)
{
	while(1)
	{
		if(newSensors != oldSensors)
		{
			/*IN*/
			int oldCarIn = oldSensors & 0x0010;
			int newCarIn = newSensors & 0x0010;
			if(oldCarIn != newCarIn)   /* A car has arrived or a car has left */
			{
				if(newCarIn > oldCarIn)  /* A car has arrived*/
				{
				  /*check lot count*/
					openGate(1); /* 1 = in*/
					printf("A CAR WANTS TO ENTER THE LOT \t");
				}
				else /* a car has succesfullly entered*/
				{
					closeGate(1); /* 1 = in*/
					printf("A CAR ENTERED THE LOT!\t");
					/*semaphore*/
					parkingLotCount++;
				}
			}
			

		}
	}
	
}
void gateOut(void)
{

	
	while(1)
	{
		if(newSensors != oldSensors)
		{		
			/*OUT*/
			int oldCarOut = oldSensors & 0x0020;
			int newCarOut = newSensors & 0x0020;
			if(oldCarOut != newCarIn)   /* A car has arrived or a car has left of the barrier */
			{
				if(newCarOut > oldCarOut)  /* A car has arrived to the gate*/
				{
					openGate(0); /*0 = out*/
					printf("A CAR WANTS TO LEAVE THE LOT \t");
				}
				else
				{
					closeGate(0); /*0=Out*/
					printf("A CAR LEFT THE LOT!\t");
					parkingLotCount--;
				}
			}
		}
	}
}

void openGate(int inOrOut)
{
	if(inOrOut == 1) /*1  = gate in */
	{
		printf("GATE IN");
		/*ACTUATOR BARRIER UP*/
		iActuators = (iActuators | 0x01); /*In barrier up bit is 0x01 is that one. We made the or to not change the other bits*/
		/*WHEN SENSOR BARRIER UP LIGHT GREEN ON RED LIGHT OFF*/
		while((newSensors & 0x0002) != 1)
		{
			/*DO NOTHING >> waiting until the barrier is up*/
		}
		iActuators = (iActuators & 0xFFFB); /*RED LIGHT OFF*//* we want the bit 0x04 in 0. that is FFF[1011] = FFFB*/
		iActuators = (iActuators | 0x0008); /*GREEN LIGHT ON*//*we want the bit 0x08 in 1.  */
		
	}
	if(inOrOut == 0)  /* 0 = gate out*/
	{
		printf("GATE OUT");
		/*ACTUATOR BARRIER UP*/
		iActuators = (iActuators | 0x02); /*OUT barrier up bit is 0x02 is that one. We made the or to not change the other bits*/
		/*WHEN SENSOR BARRIER UP LIGHT RED OFF, GREEN ON*/
		while(newSensors & 0x0002) != 1)
		{
			/*DO NOTHING >> waiting until the barrier is up*/
		}
		iActuators = (iActuators & 0xFFEF); /*RED LIGHT OFF*//* we want the bit 0x0010 in 0. that is FF[1110]F > FFFE */
		iActuators = (iActuators | 0x0020); /*GREEN LIGHT ON*//*we want the bit 0x20 in 1. 00[0010]0 */
	}
	
}
void closeGate(int inOrOut)
{
	if(inOrOut == 1)
	{
		printf("GATE IN");		
		/*LIGHT GREEN OFF, RED ON*/
		iActuators = (iActuators & 0xFFF7); /*GREEN LIGHT OFF*//*we want the bit 0x08 in 0. 1111 0111 > F7  */
		iActuators = (iActuators | 0x0004); /*RED LIGHT ON*//* we want the bit 0x04 in 1. that is 000[0100] = 0004*/
	
		/*ACTUATOR BARRIER DOWN*/
		iActuators = (iActuators & 0xFFFE); /*Out barrier up bit is 0x01. We want that one to be 0. FFF[1110] > FFFE}*/

	}
	if(inOrOut == 0)
	{
		printf("GATE OUT");
		/*LIGHT GREEN OFF, RED ON*/
		iActuators = (iActuators & 0xFFDF); /*GREEN LIGHT OFF*//*we want the bit 0x20 in 0. FF[1101]F > FFDF */
		iActuators = (iActuators | 0x0010); /*RED LIGHT ON*//* we want the bit 0x0010 in 1. that is 00[0001]0 > 0010 */
				
		/*ACTUATOR BARRIER DOWN*/
		iActuators = (iActuators & 0xFFFD); /*Out barrier up bit is 0x02. We want that one to be 0. FFF[1101]}*/

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
	while (1)
	{
		newSensors = getSensorValues(); /*read the sensors*/
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
	if((taskSpawn("TASK 0",70,0x100,2000,(FUNCPTR)inputWatcher,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning input watcher task.");
	}
	/*TASK 1 = gate in*/
	if((taskSpawn("GATE IN",70,0x100,2000,(FUNCPTR)gateIn,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning gateIn task.");
	}
	/*TASK 2 = gate out*/
	if((taskSpawn("GATE OUT",70,0x100,2000,(FUNCPTR)gateIn,0,0,0,0,0,0,0,0,0,0))== ERROR)
	{
		printf("Error spawning gateOut task.");
	}
}