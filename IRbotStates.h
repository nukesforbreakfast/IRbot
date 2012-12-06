/*
 * IRbotStates.h
 *
 * Created: 12/2/2012 2:02:52 PM
 *  Author: Nukesforbreakfast
 */ 

#ifndef IRBOTSTATES_H_
#define IRBOTSTATES_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>
#include <stdlib.h>

extern volatile int stateVar;
extern volatile int degreeVar;
extern volatile int degreeSideVar;

/*
*This is the function to handle the servo scanning.
*/
void scanState();

/*
*This is the function to handle attempting to acquire the signal
*/
void acquireState();

/*
* This is a fucntion to set up LED transmitters
*/
void setupTransmit();

#endif /* IRBOTSTATES_H_ */