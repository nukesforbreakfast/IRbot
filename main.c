/*
 */

//#include <stdlib.h>
#include <avr/io.h>
#include <avr/iox128a1.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

//used for storing CCX register values
volatile unsigned int compareRegistervalue= 0;

// used in RTC ISR and moving state. 0=move, 2=stop
volatile int timeOutFlag= 0;

// used in TIMERSONAR ISR, and in moving state. state 3(0=move, 1=stop), state 2(0= stop, 1=rotate)
volatile int sonarFlag= 0;

// used in RTC ISR and rotate state. 0=move, 1= timeout stop
volatile int stopRotateTimerFlag= 0;

// used in TIMERSONAR ISR, and in rotate state. 0=move, 1= sonar stop
volatile int stopRotateSonarFlag= 0;

// 0=start, 1=scanning, 2= rotating, 3= moving
returnPackage robotStateVar;

volatile int accum = 0; //used for LED's

volatile int scanVar; //used to control the function of the scan state

volatile int turn = 0; //used to handle servo scanning

volatile int swivels = 0; //used to count how many times we have swiveled

ISR(TIMERSONAR1_CCA_vect)
{
	compareRegistervalue= TIMERSONAR1_CCA;

	switch(robotStateVar.nextState)
	{
		case 2://rotate state
		case 3://moving state
			if(compareRegistervalue <= 870)
			{
				sonarFlag= 1;
				//stopRotateSonarFlag= 0;

			}
			else if(compareRegistervalue > 1740)
			{
				//sonarFlag= 0;
				stopRotateSonarFlag= 1;
			}
			break;
		default:
			//return;
			break;
	}

	//PORTH_OUT= compareRegistervalue/72.5;

	//PORTH_OUT= haltFlag;
}

ISR(RTC_OVF_vect)
{
	switch(robotStateVar.nextState)
	{
	    case 2://rotate state
            stopRotateTimerFlag= 2;
            break;
	    case 3://moving state
            timeOutFlag= 2;
            break;
        default:
			//return;
            break;
	}


}
ISR(PORTJ_INT0_vect)
{

	int pushbutton= PORTJ_IN;
	switch(pushbutton)
	{
		case 1:
			PORTH_OUT= RTC_CNT;
			break;
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
	}
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
			scanVar = 2; //we got no signal, indicate to the function as such
			swivels = 0; //reset swivels
		}
		break;

		default: //we are in any other state
		break;
	}
}

/*
* Interrupt for handling TCC1 capture complete event
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
		scanVar = 1;
		break;

		default: //we are in any other state
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

	robotStateVar.nextState= 1;
    while(1)
    {

		switch(robotStateVar.nextState)
		{
			case 1: //scanState
				PORTH_OUT= 1;
				robotStateVar = scanState();
				break;
			case 2://rotate state
				PORTH_OUT= 2;
				robotStateVar=rotateState(robotStateVar);
				break;
			case 3://moving state
				PORTH_OUT= 4;
				robotStateVar=movingState();
				break;
			default:
				PORTH_OUT= 0;
				robotStateVar.nextState= 0;
				break;
		}
    }
}
