
#include <vxWorks.h>		/*vxWorks functionality*/
#include <stdio.h>		/*printf capability*/
#include <sysLib.h>		/*sysClk capability*/
#include <taskLib.h>		/*tasks capability*/
#include <msgQLib.h>		/*message queue capability*/
#include <time.h>		/*timespec capability*/
#include <wdLib.h>		/*we use watchdog */
#include <logLib.h>		/*we use logMessage*/

/*Sensor inputs*/
#define IN_BARRIER_DOWN_S   0x01
#define	IN_BARRIER_UP_S     0x02
#define	OUT_BARRIER_DOWN_S  0x04
#define	OUT_BARRIER_UP_S    0x08
#define	IN_CAR	            0x10
#define	OUT_CAR             0x20

/*Actuator outputs*/
#define IN_BARRIER_UP_A	    0x01
#define	OUT_BARRIER_UP_A    0x02
#define	IN_RED	            0x04
#define	IN_GREEN	    0x08
#define	OUT_RED	            0x10
#define	OUT_GREEN	    0x20

/*grouped sensor values*/
#define IN_SENSORS          0x13
#define OUT_SENSORS         0x2C

/*constants of limit of messages*/
#define MAX_MESSAGES        1	/*do not want consecutive messages so we only allow one*/
#define MAX_MESSAGE_LENGTH  60

/*constant cars in the parking lot*/
#define MAX_CARS        10
#define INITIAL_CARS    0 

/*function prototypes*/
void inGate(void);
void outGate(void);
void setActuatorValues(int iActuators);
int getSensorValues();
void initiateProcess(void);
int main();
void sendInMessage(void);
void sendOutMessage(void);


/*enumeration for the states of the system*/
enum STATE {
    WAITING,
    OPENING,
    OPENED,
    CLOSING
    };

/*global variables*/
int iActuators;
int newSensors;
int oldSensors;
int count;


/*global variables for message queues*/
MSG_Q_ID inGateQueue;
MSG_Q_ID outGateQueue;

/*global variables for state enumerations*/
enum STATE inState;
enum STATE outState;

/*global variable for starting tick*/
int startingTick;

/*semaphore*/
SEM_ID semB;

/*watchdog*/
WDOG_ID wdInGate;
WDOG_ID wdOutGate;
