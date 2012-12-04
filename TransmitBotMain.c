/*
 * TransmitBotMain.c
 *
 * Created: 12/4/2012 3:28:49 PM
 *  Author: Nukesforbreakfast
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>
#include <stdlib.h>

volatile int accum = 0;

ISR(TCC0_CCA_vect)
{
	if(accum == 24)
	{
		TCC0_CTRLA = TC_CLKSEL_OFF_gc;
		TCC1_CTRLA = TC_CLKSEL_DIV64_gc;
		PORTC_OUT &= ~0x01;
		accum = 0;
	}
	else
	{
		++accum;
	}
	
}

ISR(TCC1_OVF_vect)
{
	TCC1_CTRLA = TC_CLKSEL_OFF_gc;
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
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
	* Timer port C0 configuration
	* The current values of PER and CCA for this timer will result in the 38Khz oscillating signal needed
	* in order to generate 1's and 0's that the reciever recognizes. Do not change these values.
	*/
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; //set timer 
	TCC0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCC0_CTRLC = 0x00; //turn off compares
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 12; //set the top of the period
	TCC0_CCA = 6; //set the compare register value to achieve 50% duty cycle at 
	TCC0_INTCTRLB = 0x01; //set the CCA interrupt to low priority.
	
	/*
	* Timer port C1 configuration
	*/
	TCC1_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially
	TCC1_CTRLB = TC_WGMODE_NORMAL_gc; //set timer to normal operation
	TCC1_CTRLC = 0x00; //turn off compares
	TCC1_CTRLD = 0x00; //turn off events
	TCC1_CTRLE = 0x00; //turn off byte mode
	TCC1_PER = 500;
	TCC1_INTCTRLA = 0x01; //set the overflow interrupt to low priority
	
	/*
	* PORT C setup
	*/
	PORTC_DIR = 0xFF;
	
	while (1)
	{
		;
	}
}