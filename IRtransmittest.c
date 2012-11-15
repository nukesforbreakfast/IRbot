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

volatile XUSARTst serialStruct;
volatile int is1;
volatile int temp;

ISR(TCC1_OVF_vect)
{
	/* for when we use serial
	TCC0_CTRLA = TC_CLKSEL_OFF_gc;
	TCC1_CTRLA = TC_CLKSEL_OFF_gc;
	*/
	
	//repeatedly send 1's
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

ISR(USARTE0_TXC_vect)
{
	Tx_Handler(&serialStruct);
}

ISR(USARTE0_RXC_vect)
{
	Rx_Handler(&serialStruct);
}

ISR(PORTF_INT0_vect)
{
	/* for when we use serial
	USART_send(&serialStruct, "A falling edge was detected.");
	*/
	
	PORTH_OUT = ++temp;
}


void main(void)
{
	unsigned long sClk, pClk;
	char recieveString[100];
	is1 = 0; //false
	temp = 0;
	
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
	TCC0_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially until serial input happens.
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
	TCC1_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially until serial input happens.
	TCC1_CTRLB = TC_WGMODE_NORMAL_gc; //set waveform generation mode to normal
	TCC1_CTRLC = 0x00; //turn off compares
	TCC1_CTRLD = 0x00; //turn off events
	TCC1_CTRLE = 0x00; //turn off byte mode
	TCC1_PER = 600; //set the top of the period to overflow at 1.2ms. NOTE: to get .6ms set this to 300.
	TCC1_INTCTRLA = 0x01; //set timer c1 overflow interrupts to low level
	
	/*
	* Port C configuration
	* Set direction to output
	*/
	PORTC_DIR = 0xFF;
	
	/*
	* Port H configuration
	* Just turn off LED's
	*/
	PORTH_DIR = 0xFF; //output dir for all pins
	PORTH_OUT = 0x00;
	
	/*
	* Serial set up
	*/
	//initialize the usart e0 for 57600 baud with 8 data bits, no parity, and 1 stop bit, interrpts on low (porth set to this for debugging purposes)
	PORTH_OUT = USART_init(&serialStruct, 0xE0, pClk, (_USART_RXCIL_MED | _USART_TXCIL_MED), 576, -4, _USART_CHSZ_8BIT, _USART_PM_DISABLED, _USART_SM_1BIT);
	USART_buffer_init(&serialStruct, 100, 100); //initialize the circular buffers
	USART_enable(&serialStruct, USART_TXEN_bm | USART_RXEN_bm); //enable the USART
	serialStruct.fOutMode = _OUTPUT_CRLF; //append a carriage return and a line feed to every output.
	serialStruct.fInMode = _INPUT_CR | _INPUT_TTY; //echo input back to the terminal and set up for keyboard input.
	
	/*
	* Port F configuration
	*/
	PORTF_DIR = 0x00; //input dir for all pins
	PORTF_INTCTRL = 0x01; //turn on interrupt 0 with a low priority
	PORTF_INT0MASK = 0x01; //mask so that only pin 0 can fire an interrupt;
	PORTF_PIN0CTRL = 0x02; //set pin 0 to detect a falling edge as the receiver is active low.
	
	sei();
		
    while(1)
    {
        if(serialStruct.serStatus & _USART_RX_DONE)
		{
			USART_read(&serialStruct, recieveString);
			
			switch(recieveString[0])
			{
				case '0':
				TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
				TCC1_PER = 300;
				TCC1_CTRLA = TC_CLKSEL_DIV64_gc;
				USART_send(&serialStruct, "Sending a 0");
				is1 = 0; //false
				break;
				case '1':
				TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
				TCC1_PER = 600;
				TCC1_CTRLA = TC_CLKSEL_DIV64_gc;
				USART_send(&serialStruct, "Sending a 1");
				is1 = 1; //true
				break;
			}
		}		
    }

}