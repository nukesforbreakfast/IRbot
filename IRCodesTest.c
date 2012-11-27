/*
 * IRCodesTest.c
 *
 * Created: 11/17/2012 6:24:17 PM
 *  Author: Nukesforbreakfast
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/AVRXlib/AVRX_Clocks.h>
#include <avr/AVRXlib/AVRX_Serial.h>
#include <stdlib.h>

volatile XUSARTst serialStruct;
volatile int is1;
volatile int isOn;
volatile int temp;
volatile int accum;
volatile int isCapture;

/*
* This ISR is used to handle when to turn off the TCCA
* accum is used as an accumulator value. At the 39.55khz this
* modulates we need approx 24 periods of oscillation for a 1 and
* approximately 12 period of oscillation for a 0. The accumulator
* value lets us achieve this.
*/
ISR(TCC0_CCA_vect)
{
	if(is1 && accum == 24)
	{
		TCC0_CTRLA = TC_CLKSEL_OFF_gc;
		PORTC_OUT &= ~0x01;
		accum = 0;
	}
	else if(!is1 && accum == 12)
	{
		TCC0_CTRLA = TC_CLKSEL_OFF_gc;
		PORTC_OUT &= ~0x01;
		accum = 0;
	}
	else
	{
		++accum;
	}
	
}

/*
* Serial transmit interrupt handler
*/
ISR(USARTD0_TXC_vect)
{
	Tx_Handler(&serialStruct);
}

/*
* Serial receive interrupt handler
*/
ISR(USARTD0_RXC_vect)
{
	Rx_Handler(&serialStruct);
}

/*
* Pushbutton equivalent for sending a 0
*/
ISR(PORTJ_INT0_vect)
{
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
	is1 = 0; //false
	PORTH_OUT = 0x00;	
}

/*
* Pushbutton equivalent for sending a 1
*/
ISR(PORTJ_INT1_vect)
{
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
	is1 = 1; //true	
	PORTH_OUT = 0x01;
}

/*
* Interrupt for handling capture complete event
*/
ISR(TCC1_CCA_vect)
{
	PORTH_OUT = TCC1_CCA;
	isCapture = 1; //true
}

void main(void)
{
	unsigned long sClk, pClk;
	char recieveString[100];
	char captureString[100];
	is1 = 0; //false
	isOn = 0; //false
	temp = 0;
	accum = 0;
	isCapture = 0; //false
	
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
	TCC0_CCA = 6; //set the compare register value to achieve 50% duty cycle at 
	TCC0_INTCTRLB = 0x01; //set the CCA interrupt to low priority.
	
	/*
	* Timer C1 configuration
	*/
	TCC1_CTRLA = TC_CLKSEL_DIV64_gc; //set clock source sysclk/64= 500KHz
	TCC1_CTRLB = 0x10 | TC_WGMODE_NORMAL_gc; //turn on capture channel A and set waveform generation mode normal
	TCC1_CTRLD = 0xC8; //set events to Pulse Width capture, no timer delay, and CCA listens to event channel 0, CCB to 1, etc... see datasheet
	TCC1_CTRLE = 0x00; //turn off byte mode
	TCC1_INTCTRLB = 0x01; //set CCA interrupt to low
	TCC1_PER = 0xFFFF; //set the top of the period to max 16-bit value
	
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
	PORTH_OUT = 0xF0;
	
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
	* PortQ setup, needed for serial via usb on the green board
	*/
	PORTQ_DIR = 0x0F; //port q lower 3 bits control access to usb and other stuff so get access with these two lines
	PORTQ_OUT = 0x05; //if using port F make this hex 5. Otherwise, use hex 7 to get access to everything.
	
	/*
	* Port F configuration
	*/
	PORTF_DIR = 0x00; //all pins as input
	PORTF_INTCTRL = 0x00; //turn on interrupt 0 with a low priority
	PORTF_INT0MASK = 0x00; //mask so that only pin 2 can fire an interrupt
	PORTF_PIN2CTRL = 0x40; //set pin 2 to detect a rising and falling edges and invert the input to allow for pulse-width capture
	
	/*
	* Port J configuration
	*/
	PORTJ_DIR = 0x00; //all pins as input
	PORTJ_INTCTRL = 0x05; //turn on both interrupts to low
	PORTJ_PIN0CTRL = 0x01; //set pin 0 so only rising edges trigger
	PORTJ_PIN1CTRL = 0x01; //set pin 1 so only rising edges trigger
	PORTJ_INT0MASK = 0x01; //mask interrupt 0 to only be fired by pin 0
	PORTJ_INT1MASK = 0x02; //mask interrupt 1 to only be fired by pin 1
	
	/*
	* Event System Configuration
	*/
	EVSYS_CH0MUX = EVSYS_CHMUX_PORTF_PIN2_gc; //set the event system to send events generated from PortF pin 2 to channel 0
	EVSYS_CH0CTRL = 0x00; //turn off sample filtering
	
		
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
				USART_send(&serialStruct, "Sending a 0");
				is1 = 0; //false
				break;
				case '1':
				TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
				USART_send(&serialStruct, "Sending a 1");
				is1 = 1; //true
				break;
			}
		}
		
		if(isCapture)
		{
			itoa(TCC1_CCA, captureString, 10);
			USART_send(&serialStruct, captureString);
			isCapture = 0; //false
		}					
    }

}
