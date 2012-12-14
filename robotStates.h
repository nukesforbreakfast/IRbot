#ifndef ROBOTSTATES_H_INCLUDED
#define ROBOTSTATES_H_INCLUDED
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>



#define MOVEFORWARD_OR				0b10100000
#define MOVEFORWARD_AND				0b10101111
#define STOPMOVING_OR				0b11110000
#define STOPMOVING_AND				0b00001111


#define ROTATERIGHT_OR				0b10010000
#define ROTATERIGHT_AND				0b10011111
#define ROTATELEFT_OR				0b01100000
#define ROTATELEFT_AND				0b01101111


#define MOTORDIR_DIR				PORTE_DIR
#define MOTORDIR_OUT				PORTE_OUT

#define PWMPORT_DIR					PORTE_DIR
#define PWMPORT_OUT					PORTE_OUT

#define PWMTIMER_CTRLA				TCE0_CTRLA
#define PWMTIMER_CTRLB				TCE0_CTRLB
#define PWMTIMER_CTRLD				TCE0_CTRLD
#define PWMTIMER_CTRLE				TCE0_CTRLE
#define PWMTIMER_PER 				TCE0_PER
#define PWMTIMER_CC1 				TCE0_CCC
#define PWMTIMER_CC2 				TCE0_CCD

#define PWMTIMER_INTCTRLA			TCE0_INTCTRLA
#define PWMTIMER_INTCTRLB 			TCE0_INTCTRLB

#define SONAR1ENABLE_DIR 			PORTF_DIR
#define SONAR1ENABLE_OUT	 		PORTF_OUT
#define SONAR1OUTPORT_DIR 			PORTF_DIR
#define SONAR1OUTPORT_PINCTRL 		PORTF_PIN0CTRL //1st pin

#define TIMERSONAR1_CTRLA 			TCF0_CTRLA
#define TIMERSONAR1_CTRLB 			TCF0_CTRLB
#define TIMERSONAR1_CTRLD			TCF0_CTRLD
#define TIMERSONAR1_CTRLE 			TCF0_CTRLE
#define TIMERSONAR1_PER  			TCF0_PER
#define TIMERSONAR1_CCA				TCF0_CCA
#define TIMERSONAR1_CCA_vect 		TCF0_CCA_vect

#define TIMERSONAR1_INTCTRLA 		TCF0_INTCTRLA
#define TIMERSONAR1_INTCTRLB 		TCF0_INTCTRLB


#define SONAR2ENABLE_DIR 			PORTF_DIR
#define SONAR2ENABLE_OUT	 		PORTF_OUT
#define SONAR2OUTPORT_DIR 			PORTF_DIR
#define SONAR2OUTPORT_PINCTRL 		PORTF_PIN4CTRL //5th pin

#define TIMERSONAR2_CTRLA 			TCF1_CTRLA
#define TIMERSONAR2_CTRLB 			TCF1_CTRLB
#define TIMERSONAR2_CTRLD			TCF1_CTRLD
#define TIMERSONAR2_CTRLE 			TCF1_CTRLE
#define TIMERSONAR2_PER  			TCF1_PER
#define TIMERSONAR2_CCA 			TCF1_CCA
#define TIMERSONAR2_CCA_vect 		TCF1_CCA_vect

#define TIMERSONAR2_INTCTRLA 		TCF1_INTCTRLA
#define TIMERSONAR2_INTCTRLB 		TCF1_INTCTRLB

/*causey's defines*/
#define SERVO_PWM				TCE0
#define SERVO_PWM_PORT			PORTE
#define SERVO_PWM_OVF_VECT		TCE0_OVF_vect
#define IR_PW_CAPTURE			TCC1
#define IR_PW_CAPTURE_VECT		TCC1_CCA_vect
#define IR_INPUT_PORT			PORTD
#define PW_TIMEOUT				TCC0
#define PW_TIMEOUT_OVF_VECT		TCC0_OVF_vect
#define TRANSMIT_PORT			PORTF
#define TRANSMIT_OSCILLATOR		TCF0
#define TRANSMIT_TIMER			TCF1

/*extern variables go here*/
extern volatile unsigned char timeOutFlag;
extern volatile unsigned char sonarFlag1;
extern volatile unsigned char sonarFlag2;
extern volatile unsigned char stopRotateTimerFlag;
extern volatile unsigned char stopRotateSonarFlag1;
extern volatile unsigned char stopRotateSonarFlag2;
extern volatile int scanVar;

typedef struct
{
	unsigned char prevState;
	unsigned char nextState;
	char direction;
	char globalTimeoutDirection;
	unsigned int rotateQuantity;
	unsigned char signalAcquiredFlag; // changed from uint8_t and  (learn to spell n00b)==>sinalAcquiredFlag
}returnPackage;


void setRTC(int);

void enableSonar();

void setupMotors();

void rotateState(returnPackage*);

void movingState(returnPackage*);

/*
*This is the function to handle the servo scanning.
*/
void scanState(returnPackage*);

/*
*This is the function to handle attempting to acquire the signal
*/
void acquireState();

/*
* This is a function to set up LED transmitters
*/
void setupTransmit();


#endif // ROBOTSTATES_H_INCLUDED
