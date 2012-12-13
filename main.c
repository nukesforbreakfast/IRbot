/*
 */

//#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

//used for storing CCX register values
volatile unsigned int compareRegistervalue= 0;

// used in RTC ISR and moving state. 0=move, 2=stop
volatile unsigned char timeOutFlag= 0;

// used in TIMERSONAR1 ISR, and in moving state. state 3(0=move, 1=stop), state 2(0= stop, 1=rotate)
volatile unsigned char sonarFlag1= 0;

// used in TIMERSONAR2 ISR, and in moving state. state 3(0=move, 1=stop), state 2(0= stop, 1=rotate)
volatile unsigned char sonarFlag2= 0;

// used in RTC ISR and rotate state. 0=move, 1= timeout stop
volatile unsigned char stopRotateTimerFlag= 0;

// used in TIMERSONAR1 ISR, and in rotate state. 0=move, 1= sonar stop
volatile unsigned char stopRotateSonarFlag1= 0;

// used in TIMERSONAR2 ISR, and in rotate state. 0=move, 1= sonar stop
volatile unsigned char stopRotateSonarFlag2= 0;

// 0=start, 1=scanning, 2= rotating, 3= moving
returnPackage robotStateVar;

volatile int accum = 0; //used for LED's

volatile int scanVar; //used to control the function of the scan state

volatile int turn = 0; //used to handle servo scanning

volatile int swivels = 0; //used to count how many times we have swiveled

volatile int pulses = 0; //uses to count how many correct pulses we have recieved.

ISR(TIMERSONAR1_CCA_vect)
{
	compareRegistervalue= TIMERSONAR1_CCA;

	switch(robotStateVar.nextState)
	{
		case 2://rotate state
		case 3://moving state
			if(compareRegistervalue <= 870)
			{
				sonarFlag1= 2;
				stopRotateSonarFlag1= 0;

			}
			else if(compareRegistervalue > 1740)
			{
				sonarFlag1= 0;
				stopRotateSonarFlag1= 2;
			}
			break;
		default:
			//return;
			break;
	}
}


ISR(TIMERSONAR2_CCA_vect)
{
	compareRegistervalue= TIMERSONAR2_CCA;

	switch(robotStateVar.nextState)
	{
		case 2://rotate state
		case 3://moving state
			if(compareRegistervalue <= 200)
			{
				sonarFlag2= 4;
				stopRotateSonarFlag2= 0;

			}
			else if(compareRegistervalue > 400)
			{
				sonarFlag2= 0;
				stopRotateSonarFlag2= 4;
			}
			break;
		default:
			//return;
			break;
	}		
}


ISR(TCD0_OVF_vect)
{
	switch(robotStateVar.nextState)
	{
		case 2://rotate state
		stopRotateTimerFlag++;
		break;
		case 3://moving state
		timeOutFlag++;
		break;
		default:
		//return;
		break;
	}	
}


/*

ISR(RTC_OVF_vect)
{
	if(robotStateVar.nextState == 3)
	{
		timeOutFlag= 1;
	}
}


ISR(RTC_COMP_vect)
{
	if(robotStateVar.nextState == 2)
	{
		stopRotateTimerFlag= 1;
	}
	
	
	switch(robotStateVar.nextState)
	{
	    case 2://rotate state
            stopRotateTimerFlag= 1;
            break;
	    case 3://moving state
            timeOutFlag= 1;
            break;
        default:
			//return;
            break;
	}
	
}
*/


ISR(PORTJ_INT0_vect)
{

	/*int pushbutton= PORTJ_IN;
	switch(pushbutton)
	{
		case 1:
			//PORTH_OUT= RTC_CNT;
			//break;
		case 2:
			//PORTH_OUT= 0;
			//break;
		case 4:
			PORTH_OUT= MOTORDIR_OUT;
			break;
		case 8:
			//robotStateVar.nextState= 1;
			//break;
		default:
			PORTH_OUT= 0;
			break;
	}*/
}

ISR(PORTJ_INT1_vect)
{

	/*
	int pushbutton= PORTJ_IN;
	switch(pushbutton)
	{
		case 16:

			break;
		case 32:

			break;
		case 64:

			break;
		case 128:
			break;
		default:
			break;
	}
	PORTH_OUT= 0;
	*/
	;
}

/*
* Servo PWM overflow interrupt vector
* used to swivel the servo back and forth
*/
ISR(SERVO_PWM_OVF_VECT)
{
	switch(robotStateVar.nextState)
	{
		case 1: //we are in the scan state
		if(turn) //if its true that we need to turn back the other way
		{
			if(!(SERVO_PWM.CCA <= 350)) //make sure we aren't going under the minimum value
			{
				SERVO_PWM.CCA -= 5;
				//PORTH_OUT = TCE0_CCA/10;
			}
			else //if we are going under the minimum value we need to turn the other way
			{
				turn = 0; //false
				++swivels;
			}
		}
		else //else we are still turning original direction
		{
			if(!(SERVO_PWM.CCA >= 1150)) //make sure we aren't going over the maximum value
			{
				TCE0_CCA += 5;
				//PORTH_OUT = TCE0_CCA/10;
			}
			else //if we are going over the maximum value we need to turn the other way.
			{
				turn = 1; //true
			}
		}

		if(swivels > 1)
		{
			scanVar = 3; //we got no signal, indicate to the function as such
			swivels = 0; //reset swivels
		}
		break;

		default: //we are in any other state
		break;
	}
}

/*
* Interrupt for handling IR_PW_CAPTURE capture complete event
* If we get a pulse we need to stop servo movement and
* calculate how much we need to turn and send that to
* the turning state
*/
ISR(IR_PW_CAPTURE_VECT)
{
	switch(robotStateVar.nextState)
	{
		case 1: //we are in scanState
		//we first need to stop the servo where it is and hold its position
		//do this by turning off the overflow interrupt for TCE0
		SERVO_PWM.INTCTRLA = 0x00; //all interrupts off
		if(pulses == 0) //if this if the first pulse
		{
			PW_TIMEOUT.CTRLA = TC_CLKSEL_DIV1024_gc; //turn on the timeout timer running at sysclk/1024 = 31.250Khz
		}
		scanVar = 1; //indicate we got a pulse and need to verify consistency
		if(SERVO_PWM.CCA > 500 && SERVO_PWM.CCA < 700) //if the pulse width is within the expected width
		{
			++pulses;
			if(pulses > 9) //if we have gotten 10 consistent pulses;
			{
				scanVar = 2;
				pulses = 0; //reset pulses
			}
		}
		break;

		default: //we are in any other state
		break;
	}
}

/*
* Interrupt for handling the PW_TIMEOUT and saying we did not get
* a valid sequence of pulses.
*/
ISR(PW_TIMEOUT_OVF_VECT)
{
	switch(robotStateVar.nextState)
	{
		case 1: //we are in scan state
		IR_PW_CAPTURE.CTRLA = TC_CLKSEL_OFF_gc; //turn off the pulse width capture
		PW_TIMEOUT.CTRLA = TC_CLKSEL_OFF_gc; //turn the timeout counter off
		pulses = 0; //reset pulses
		scanVar = 3; //indicate we did not get a consistent sequence of pulses
		break;
		
		default: //any other state
		break;
	}
}

int main(void)
{
	unsigned long sClk, pClk;
	cli();

	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);

	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; //set PMIC to on.


	/*
    PORTJ_DIR= 0x00;
    PORTJ_INTCTRL= 0x00; // pushbuttons interrupts low
    PORTJ_INT0MASK= 0xFF;
    PORTJ_INT1MASK= 0x00;

    //PORTJ_PIN0CTRL= 0x01;
    PORTJ_PIN1CTRL= 0x01;
    PORTJ_PIN2CTRL= 0x01;
    PORTJ_PIN3CTRL= 0x01;
    PORTJ_PIN4CTRL= 0x01;
    PORTJ_PIN5CTRL= 0x01;
    PORTJ_PIN6CTRL= 0x01;
    PORTJ_PIN7CTRL= 0x01;
    */

    PORTH_DIR= 0xFF;
    PORTH_OUT= 0x00;

	sei(); //enable interrupt system

	robotStateVar.globalTimeoutDirection= 'R';

	robotStateVar.nextState= 1;
    while(1)
    {

		switch(robotStateVar.nextState)
		{
			case 1: //scanState
				PORTH_OUT= 1;
				scanState(&robotStateVar);
				break;
			case 2://rotate state
				PORTH_OUT= 2;
				rotateState(&robotStateVar);
				break;
			case 3://moving state
				PORTH_OUT= 4;
				movingState(&robotStateVar);
				break;
			case 4: //final state
				//do something here n00b!
				break;
			default:
				PORTH_OUT= 0;
				robotStateVar.nextState= 0;
				break;
		}
    }
}
