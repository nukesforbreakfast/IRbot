/*
 * IRbotStates.c
 *
 * Created: 12/2/2012 2:02:10 PM
 *  Author: Nukesforbreakfast
 */ 

#include "IRbotStates.h"

void scanState()
{
	/************************************************
	* PWM Setup for the servo using PORTE, TCE0_CCA *
	************************************************/
	/*
	* Timer E0 setup for servo PWM
	*/
	SERVO_PWM.CTRLA = TC_CLKSEL_DIV64_gc; //set timer to div/64
	SERVO_PWM.CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	SERVO_PWM.CTRLC = 0x00; //turn off compares
	SERVO_PWM.CTRLD = 0x00; //turn off events
	SERVO_PWM.CTRLE = 0x00; //turn off byte mode
	SERVO_PWM.PER = 10000; //set the top of the period to 20ms
	SERVO_PWM.CCA = 350; //lower bound, upper bound should be 1150
	SERVO_PWM.INTCTRLA = 0x01; //turn on Overflow interrupt at low priority.
	/*
	* PORT E configuration
	*/
	PORTE_DIR = 0xFF; //set pin 0 to output without messing up other pins
	
	/**************************************************
	* Setup for IR receiver using pulse width capture *
	**************************************************/
	/*
	* IR timer capture configuration
	*/
	IR_PW_CAPTURE.CTRLA = TC_CLKSEL_DIV64_gc; //set clock source sysclk/64= 500KHz
	IR_PW_CAPTURE.CTRLB = 0x10 | TC_WGMODE_NORMAL_gc; //turn on capture channel A and set waveform generation mode normal
	IR_PW_CAPTURE.CTRLD = 0xC8; //set events to Pulse Width capture, no timer delay, and CCA listens to event channel 0, CCB to 1, etc... see datasheet
	IR_PW_CAPTURE.CTRLE = 0x00; //turn off byte mode
	IR_PW_CAPTURE.INTCTRLB = 0x01; //set CCA interrupt to low
	IR_PW_CAPTURE.PER = 0xFFFF; //set the top of the period to max 16-bit value
	/*
	* IR input port configuration
	*/
	IR_INPUT_PORT.DIR = 0x00; //all pins as input
	IR_INPUT_PORT.PIN2CTRL = 0x40; //set pin 2 to detect a rising and falling edges and invert the input to allow for pulse-width capture
	/*
	* Event System Configuration
	*/
	EVSYS_CH0MUX = EVSYS_CHMUX_PORTC_PIN2_gc; //set the event system to send events generated from PortC pin 2 to channel 0
	EVSYS_CH0CTRL = 0x00; //turn off sample filtering
	
	while(stateVar == 0)
	{
		;
	}
	
}

/*
* State for handling the acquisition of the signal and calculating the movement vector.
* Use the value of CCA to see what degree we are at.
*/
void acquireState()
{
	/********************************************************
	* Port J pushbutton setup								*
	* temporarily for restarting the scan state for testing *
	********************************************************/
	/*
	* Port J configuration
	*/
	PORTJ_DIR = 0x00; //all pins as input
	PORTJ_INTCTRL = 0x01; //turn on Interrupt 0 to low priority
	PORTJ_PIN0CTRL = 0x01; //set pin 0 so only rising edges trigger
	PORTJ_INT0MASK = 0x01; //mask interrupt 0 to only be fired by pin 0
	
	
	/*************************************************************
	* Section of state to handle calculating the degrees to turn *
	* as well as motor control									 *
	*************************************************************/
	degreeVar = TCE0_CCA * 2; //double TCE0_CCA gives you microseconds
	
	if(degreeVar > 1500)
	{
		//we need to turn right
		degreeVar = degreeVar - 1500; //normalize this to be between 0-900 microseconds
		degreeVar /= 10; //this will give us a value in degrees as 10 microseconds = 1 degree
		degreeSideVar = 1; //this indicates this will be degreeVar degrees to the right.
	}
	else
	{
		//we need to turn left
		degreeVar = 1500 - degreeVar; //normalize this to be between 0-900 microseconds
		degreeVar /= 10; //this will give us a value in degrees as 10 microseconds = 1 degree
		degreeSideVar = 0; //this indicates this will be degreeVar degrees to the left.
	}
	
	//need to do something with this value and motors here.
	//temporary code below:
	PORTH_OUT = degreeVar;
	PORTH_OUT |= degreeSideVar << 7;
	
	while(stateVar == 1)
	{
		;
	}
}

void setupTransmit()
{
	/*
	* Timer port F0 configuration
	* The current values of PER and CCA for this timer will result in the 38Khz oscillating signal needed
	* in order to generate 1's and 0's that the reciever recognizes. Do not change these values.
	*/
	TCF0_CTRLA = TC_CLKSEL_DIV64_gc; //set timer 
	TCF0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCF0_CTRLC = 0x00; //turn off compares
	TCF0_CTRLD = 0x00; //turn off events
	TCF0_CTRLE = 0x00; //turn off byte mode
	TCF0_PER = 12; //set the top of the period
	TCF0_CCA = 6; //set the compare register value to achieve 50% duty cycle at 
	TCF0_INTCTRLB = 0x00; //set the CCA interrupt to low priority.
	
	/*
	* Timer port F1 configuration
	*/
	TCF1_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially
	TCF1_CTRLB = TC_WGMODE_NORMAL_gc; //set timer to normal operation
	TCF1_CTRLC = 0x00; //turn off compares
	TCF1_CTRLD = 0x00; //turn off events
	TCF1_CTRLE = 0x00; //turn off byte mode
	TCF1_PER = 500; //1ms
	TCF1_INTCTRLA = 0x01; //set the overflow interrupt to low priority
	
	/*
	* PORT F setup
	* set direction for pin 1 without upsetting other pins
	*/
	PORTF_DIR |= 0x01;
}