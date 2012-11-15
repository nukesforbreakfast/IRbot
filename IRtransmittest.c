/*
 * IRtransmittest.c
 *
 * Created: 11/13/2012 2:32:32 PM
 *  Author: Ryan Causey
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>

ISR(TCC1_OVF_vect)
{
	if(TCC1_PER == 600)
	{
		TCC0_CTRLA = TC_CLKSEL_OFF_gc;
		TCC1_PER = 300;
	}
	else
	{
		TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
		TCC1_PER = 600;
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
	* Timer port C0 configuration
	* The current values of PER and CCA for this timer will result in the 38Khz oscillating signal needed
	* in order to generate 1's and 0's that the reciever recognizes. Do not change these values.
	*/
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; //set prescaler to clk/64
	TCC0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCC0_CTRLC = 0x00; //turn off compares
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 12; //set the top of the period
	TCC0_CCA = 6; //set the compare register value to achieve 50% duty cycle at start
	
	/*
	* Timer port C1 configuration
	* This timer is used to turn the PWM on and off as to replicate 1's and 0's
	* The standard requires that a 1 be represented with 1.2ms of 38Khz oscillating signal followed by
	* .6ms of no signal. A 0 is represented with .6ms of 38Khz oscillating signal followed by .6ms of
	* no signal.
	*/
	TCC1_CTRLA = TC_CLKSEL_DIV64_gc; //set prescaler to clk/64
	TCC1_CTRLB = TC_WGMODE_NORMAL_gc; //set waveform generation mode to normal
	TCC1_CTRLC = 0x00; //turn off compares
	TCC1_CTRLD = 0x00; //turn off events
	TCC1_CTRLE = 0x00; //turn off byte mode
	TCC1_PER = 600; //set the top of the period to overflow at 1.2ms
	TCC1_INTCTRLA = 0x01; //set timer c1 overflow interrupts to low level
	
	/*
	* Port C configuration
	*/
	PORTC_DIR = 0xFF;
	PORTH_DIR = 0xFF;
	PORTH_OUT = 0x00;
	
	sei();
		
    while(1)
    {
        ;
    }

}