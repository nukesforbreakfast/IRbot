/*
 * ServoTest.c
 *
 * Created: 11/30/2012 9:07:36 PM
 *  Author: Nukesforbreakfast
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>
#include <stdlib.h>

volatile XUSARTst serialStruct;

ISR(USARTD0_RXC_vect)
{
	Rx_Handler(&serialStruct);	
}

ISR(USARTD0_TXC_vect)
{
	Tx_Handler(&serialStruct);
}

void main(void)
{
	volatile char receiveString[100];
	int receiveInt = 0;
	
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; //set timer to div/64
	TCC0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCC0_CTRLC = 0x00; //turn off compares
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 10000; //set the top of the period to 20ms
	TCC0_CCA = 600; //this is supposed to be 90 degrees left
	
	while(1)
	{
		 if(serialStruct.serStatus & _USART_RX_DONE)
		 {
			 USART_read(&serialStruct, receiveString);
			 receiveInt = atoi(receiveString);
			 TCC0_CCA = receiveInt;
		 }			 
	}
}