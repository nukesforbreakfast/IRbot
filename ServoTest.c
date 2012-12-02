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
volatile int turn;
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
	if(!(TCE0_CCA >= 1150))
	{
		TCE0_CCA += 100;
		setInt = TCE0_CCA;
		setFlag = 1; //true
		PORTH_OUT = TCE0_CCA/10;
	}
}

/*
* Pushbutton to decrement CCA
*/
ISR(PORTJ_INT1_vect)
{
	if(!(TCE0_CCA <= 350))
	{
		TCE0_CCA -= 100;
		setInt = TCE0_CCA;
		setFlag = 1; //true
		PORTH_OUT = TCE0_CCA/10;
	}		
}

/*
* TCE0_CCA interrupt vector
*/
ISR(TCE0_OVF_vect)
{
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
}

void main(void)
{
	int receiveInt = 0;
	setInt = 0;
	setFlag = 0; //false
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
	USART_init(&serialStruct, 0xD0, pClk, (_USART_RXCIL_LO | _USART_TXCIL_LO), 576, -4, _USART_CHSZ_8BIT, _USART_PM_DISABLED, _USART_SM_1BIT);
	USART_buffer_init(&serialStruct, 100, 100); //initialize the circular buffers
	USART_enable(&serialStruct, USART_TXEN_bm | USART_RXEN_bm); //enable the USART
	serialStruct.fOutMode = _OUTPUT_CRLF; //append a carriage return and a line feed to every output.
	serialStruct.fInMode = _INPUT_CR | _INPUT_TTY | _INPUT_ECHO; //echo input back to the terminal and set up for keyboard input.
	
	/*
	* Timer E0 setup for servo PWM
	*/
	TCE0_CTRLA = TC_CLKSEL_DIV64_gc; //set timer to div/64
	TCE0_CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TCE0_CTRLC = 0x00; //turn off compares
	TCE0_CTRLD = 0x00; //turn off events
	TCE0_CTRLE = 0x00; //turn off byte mode
	TCE0_PER = 10000; //set the top of the period to 20ms
	TCE0_CCA = 350; //lower bound, datasheet says 600 microseconds(which should be 300) but that is to low so set it to this
	TCE0_INTCTRLA = 0x01; //turn on CCA interrupt at low priority.
	
	/*
	* Port J configuration for pushbutton incrementing
	*/
	PORTJ_DIR = 0x00; //all pins as input
	PORTJ_INTCTRL = 0x05; //turn on both interrupts to low
	PORTJ_PIN0CTRL = 0x01; //set pin 0 so only rising edges trigger
	PORTJ_PIN1CTRL = 0x01; //set pin 1 so only rising edges trigger
	PORTJ_INT0MASK = 0x01; //mask interrupt 0 to only be fired by pin 0
	PORTJ_INT1MASK = 0x02; //mask interrupt 1 to only be fired by pin 1
	
	/*
	* PORT E configuration
	*/
	PORTE_DIR = 0xFF;	
	
	
	PORTH_OUT = TCE0_CCA/10;
	
	sei();
	
	while(1)
	{
		 if(serialStruct.serStatus & _USART_RX_DONE)
		 {
			 USART_read(&serialStruct, receiveString);
			 receiveInt = atoi(receiveString);
			 if(receiveInt < 350 || receiveInt > 1150)
				USART_send(&serialStruct, "These are not the values you are looking for");
			 else
				TCE0_CCA = receiveInt;
		 }
		 
		 if(setFlag)
		 {
			 setFlag = 0; //false
			 itoa(setInt, sendString, 10);
			 while(1)
				USART_send(&serialStruct, sendString);
		 }
	}
}