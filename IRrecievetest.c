/*
 * IRrecievetest.c
 *
 * Created: 11/8/2012 2:12:42 PM
 *  Author: Nukesforbreakfast
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>

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
	
	
	
	sei();
	
	PORTH_DIR = 0xFF;
	PORTH_OUT = 0xAA;
	
    while(1)
    {
        ;
    }

}