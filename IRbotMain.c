/*
 * IRbotMain.c
 *
 * Created: 12/2/2012 2:54:22 PM
 *  Author: Nukesforbreakfast
 */
#include "IRbotStates.h"

/*
* Global Variables go here
*/
volatile int turn = 0;
volatile int stateVar = 0; //used to switch between states
volatile int degreeVar = 0; //used for seeing which degree the servo is at.
volatile int degreeSideVar = 0; //used for determining left or right, 0 = left, 1 = right

/*
* TCE0_CCA overflow interrupt vector
* used to swivel the servo back and forth
*/
ISR(TCE0_OVF_vect)
{
	switch(stateVar)
	{
		case 0: //we are in the scan state
			if(turn) //if its true that we need to turn back the other way
			{
				if(!(TCE0_CCA <= 350)) //make sure we aren't going under the minimum value
				{
					TCE0_CCA -= 5;
					PORTH_OUT = TCE0_CCA/10;
				}
				else //if we are going under the minimum value we need to turn the other way
				{
					turn = 0; //false
				}
			}
			else //else we are still turning original direction
			{
				if(!(TCE0_CCA >= 1150)) //make sure we aren't going over the maximum value
				{
					TCE0_CCA += 5;
					PORTH_OUT = TCE0_CCA/10;
				}
				else //if we are going over the maximum value we need to turn the other way.
				{
					turn = 1; //true
				}
			}
			break;
			
		case 1: //we are in the acquire state
			break;
		
		default:
			break;
	}	
}

/*
* Interrupt for handling TCC1 capture complete event
* If we get a pulse we need to stop everything and attempt
* to acquire the signal
*/
ISR(TCC1_CCA_vect)
{
	switch(stateVar)
	{
		case 0: //we are in scanState
			//we first need to stop the servo where it is and hold its position
			//do this by turning off the overflow interrupt for TCE0
			TCE0_INTCTRLA = 0x00; //all interrupts off
			
			//need to signal to the scan function that we should change state here
			stateVar = 1;
			break;
			
		case 1: //we are in acquireState
			break;
			
		default:
			break;
	}	
}

/*
* Handle pushbutton 0 presses to reset the machine states
*/
ISR(PORTJ_INT0_vect)
{
	switch(stateVar)
	{
		case 0: //we are in scanState
			break;
		
		case 1: //we are in acquireState
			stateVar = 0; //reset to scanState
			break;
		
		default:
			break;
	}
}

void main(void)
{
	unsigned long sClk, pClk;
	
	cli(); //
	
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc,
	CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	
	/*
	* Programmable interrupt controller configuration
	*/
	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; //enable all levels of interrupts
	
	/*
	*Port H configuration
	*/
	PORTH_DIR = 0xFF; //set all pins to output so we can output onto the LED's
	
	/*
	* Port Q configuration for accessing Serial Via USB using USARTD0, analog stuff on PORTB, etc...
	*/
	PORTQ_DIR = 0x0F; //port q lower 3 bits control access to usb and other stuff so get access with these two lines
	PORTQ_OUT = 0x05; //if using port F make this hex 5.
	
	sei();
	
	while(1)
	{
		switch(stateVar)
		{
			case 0:
				scanState();
				break;
			
			case 1:
				PORTH_OUT = 0x05;
				acquireState();
				break;
			
			default:
				break;
		}
	}
}