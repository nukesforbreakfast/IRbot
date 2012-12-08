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

#define SERVO_PWM TCE0
#define IR_PW_CAPTURE TCC1
#define IR_INPUT_PORT PORTC

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