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
volatile int setInt;
volatile int setFlag;
volatile char receiveString[100];
volatile char sendString[100];

ISR(USARTD0_RXC_vect)
{
	Rx_Handler(&serialStruct);	
}

ISR(USARTD0_TXC_vect)
{
	Tx_Handler(&serialStruct);
}

/*
* Pushbutton to increment CCA
*/
ISR(PORTJ_INT0_vect)
{
	if(!(TCC0_CCA >= 1200))
	{
		TCC0_CCA += 100;
		setInt = TCC0_CCA;
		setFlag = 1; //true
		PORTH_OUT = TCC0_CCA/10;
	}
}

/*
* Pushbutton to decrement CCA
*/
ISR(PORTJ_INT1_vect)
{
	if(!(TCC0_CCA <= 300))
	{
		TCC0_CCA -= 100;
		setInt = TCC0_CCA;
		setFlag = 1; //true
		PORTH_OUT = TCC0_CCA/10;
	}		
}

void main(void)
{
	int receiveInt = 0;
	int setInt = 0;
	int setFlag = 0; //false
	unsigned long sClk, pClk;
	
	cli(); //
	
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc,
	CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	
	/*
	* Programmable interrupt controller configuration
	*/
	PMIC_CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm; //enable all levels of interrupts
	
	PORTH_DIR = 0xFF;
	PORTQ_DIR = 0x0F; //port q lower 3 bits control access to usb and other stuff so get access with these two lines
	PORTQ_OUT = 0x07; //if using port F make this hex 5.
	/*
	* Serial set up
	*/
	//initialize the usart d0 for 57600 baud with 8 data bits, no parity, and 1 stop bit, interrupts on low (porth set to this for debugging purposes)
	/*USART_init(&serialStruct, 0xD0, pClk, (_USART_RXCIL_LO | _USART_TXCIL_LO), 576, -4, _USART_CHSZ_8BIT, _USART_PM_DISABLED, _USART_SM_1BIT);
	USART_buffer_init(&serialStruct, 100, 100); //initialize the circular buffers
	USART_enable(&serialStruct, USART_TXEN_bm | USART_RXEN_bm); //enable the USART
	serialStruct.fOutMode = _OUTPUT_CRLF; //append a carriage return and a line feed to every output.
	serialStruct.fInMode = _INPUT_CR | _INPUT_TTY | _INPUT_ECHO; //echo input back to the terminal and set up for keyboard input.*/
	
	/*
	* Timer C0 setup for servo PWM
	*/
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc; //set timer to div/64
	TCC0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCC0_CTRLC = 0x00; //turn off compares
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 10000; //set the top of the period to 20ms
	TCC0_CCA = 300; //this is supposed to be 90 degrees left(600 microseconds)
	
	/*
	* Port J configuration for pushbutton incrementing
	*/
	PORTJ_DIR = 0x00; //all pins as input
	PORTJ_INTCTRL = 0x05; //turn on both interrupts to low
	PORTJ_PIN0CTRL = 0x01; //set pin 0 so only rising edges trigger
	PORTJ_PIN1CTRL = 0x01; //set pin 1 so only rising edges trigger
	PORTJ_INT0MASK = 0x01; //mask interrupt 0 to only be fired by pin 0
	PORTJ_INT1MASK = 0x02; //mask interrupt 1 to only be fired by pin 1
	
	PORTH_OUT = TCC0_CCA/10;
	
	sei();
	
	while(1)
	{
		 /*if(serialStruct.serStatus & _USART_RX_DONE)
		 {
			 USART_read(&serialStruct, receiveString);
			 receiveInt = atoi(receiveString);
			 if(receiveInt < 300 || receiveInt > 1200)
				USART_send(&serialStruct, "These are not the values you are looking for");
			 else
				TCC0_CCA = receiveInt;
		 }
		 
		 if(setFlag)
		 {
			 setFlag = 0; //false
			 itoa(setInt, sendString, 10);
			 while(1)
				USART_send(&serialStruct, sendString);
		 }*/
		 ;
	}
}