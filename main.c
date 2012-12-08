/*
 */

//#include <stdlib.h>
#include <avr/io.h>
//#include <avr/iox128a1.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

//used for storing CCX register values
volatile unsigned int compareRegistervalue= 0;

// 0=move, 2=stop
volatile int timeOutFlag= 0;

// state 3(0=move, 1=stop), state 2(0= stop, 1=rotate)
volatile int sonarFlag= 0;

// 0=move, 2=stop
volatile int stopRotateFlag= 0;

// 0=start, 1=scanning, 2= rotating, 3= moving
returnPackage robotStateVar;

volatile int accum = 0; //used for LED's

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

			}
			else if(compareRegistervalue > 1740)
			{
				sonarFlag= 0;
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
            stopRotateFlag= 1;
            break;
	    case 3://moving state
            timeOutFlag= 1;
            break;
        default:
			//return;
            break;
	}


}
ISR(PORTJ_INT0_vect)
{
	PORTH_OUT= RTC_PER>>8;
	/*
	int pushbutton= PORTJ_IN;
	switch(pushbutton)
	{
		case 1:
			PORTH_OUT= RTC_CNT & 0x00FF;
			break;
		case 2:
			PORTH_OUT= RTC_CNT >> 8;
			break;
		case 4:
			PORTH_OUT= overflows;
			break;
		case 8:
			PORTH_OUT= restarts;
			break;
		case 16:
			PORTH_OUT= RTC_PER & 0x00FF;
			break;
		case 32:
			PORTH_OUT= RTC_PER >> 8;
		default:
			PORTH_OUT= 0;
			break;
	}
	*/
}

ISR(PORTJ_INT1_vect)
{
    PORTH_OUT=RTC_PER & 0x00FF;
	robotStateVar.nextState= 2;

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

}

int main(void)
{
	unsigned long sClk, pClk;
	cli();

	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);

	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; //set PMIC to on.

    PORTJ_DIR= 0x00;
    PORTJ_INTCTRL= 0x05; // pushbuttons interrupts low
    PORTJ_INT0MASK= 0x01;
    PORTJ_INT1MASK= 0x02;

    PORTJ_PIN0CTRL= 0x01;
    PORTJ_PIN1CTRL= 0x01;
    PORTJ_PIN2CTRL= 0x01;
    PORTJ_PIN3CTRL= 0x01;
    PORTJ_PIN4CTRL= 0x01;
    PORTJ_PIN5CTRL= 0x01;
    //PORTJ_PIN6CTRL= 0x01;
    //PORTJ_PIN7CTRL= 0x01;

    PORTH_DIR= 0xFF;
    PORTH_OUT= 0x00;

	sei(); //enable interrupt system

	robotStateVar.nextState= 3;
    while(1)
    {

		switch(robotStateVar.nextState)
		{
			//case 1:
				//break;
			case 2://rotate state
				PORTH_OUT= 2;
				robotStateVar=rotateState(robotStateVar);
				break;
			case 3://moving state
				PORTH_OUT= 3;
				robotStateVar=movingState();
				break;
			default:
				PORTH_OUT= 1;
				robotStateVar.nextState= 1;
				break;
		}
    }
}
