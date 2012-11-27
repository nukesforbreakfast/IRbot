/*
 * SerialTest.c
 *
 * Created: 11/25/2012 1:11:50 PM
 *  Author: Nukesforbreakfast
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>

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
	unsigned long sClk, pClk;
	char recieveString[100];
	
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
	USART_init(&serialStruct, 0xD0, pClk, (_USART_RXCIL_LO | _USART_TXCIL_LO), 576, -4, _USART_CHSZ_8BIT, _USART_PM_DISABLED, _USART_SM_1BIT);
	USART_buffer_init(&serialStruct, 100, 100); //initialize the circular buffers
	USART_enable(&serialStruct, USART_TXEN_bm | USART_RXEN_bm); //enable the USART
	serialStruct.fOutMode = _OUTPUT_CRLF; //append a carriage return and a line feed to every output.
	serialStruct.fInMode = _INPUT_CR | _INPUT_TTY | _INPUT_ECHO; //echo input back to the terminal and set up for keyboard input.
	
	sei();

	
		
	while(1)
	{
		USART_send(&serialStruct, "Hey, am I working?");
		while (!(serialStruct.serStatus & _USART_TX_EMPTY)) { ; }
	}
}