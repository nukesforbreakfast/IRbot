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
	if(TCC1_PER == 150)
	{
		TCC0_CTRLA = TC_CLKSEL_OFF_gc;
		TCC1_PER = 75;
	}
	else
	{
		TCC0_CTRLA = TC_CLKSEL_DIV256_gc;
		TCC1_PER = 150;
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
	*/
	TCC0_CTRLA = TC_CLKSEL_DIV256_gc; //set prescaler to clk/256
	TCC0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCC0_CTRLC = 0x00; //turn off compares
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 4; //set the top of the period
	TCC0_CCA = 2; //set the compare register value to achieve 0% duty cycle at start
	
	/*
	* Timer port C1 configuration
	*/
	TCC1_CTRLA = TC_CLKSEL_DIV256_gc; //set prescaler to clk/256
	TCC1_CTRLB = TC_WGMODE_NORMAL_gc; //set waveform generation mode to normal
	TCC1_CTRLC = 0x00; //turn off compares
	TCC1_CTRLD = 0x00; //turn off events
	TCC1_CTRLE = 0x00; //turn off byte mode
	TCC1_PER = 150; //set the top of the period
	TCC1_INTCTRLA = 0x01; //set timer c1 overflow interrupts to low level
	
	/*
	* Port C configuration
	*/
	PORTC_DIR = 0xFF;
	
	sei();
		
    while(1)
    {
        ;
    }

}