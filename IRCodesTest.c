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

volatile XUSARTst serialStruct;
volatile int is1;
volatile int isOn;
volatile int temp;
volatile int edgeFlag;
volatile int timeoutFlag;
volatile int pollFlag;
volatile int accum;

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

ISR(USARTD0_TXC_vect)
{
	Tx_Handler(&serialStruct);
}

ISR(USARTD0_RXC_vect)
{
	Rx_Handler(&serialStruct);
}

/*
*Use this to handle the 0 or 1 detection using the following scheme:
*at the first falling edge detected start two timers, one for 1200
*microseconds and one for 900 microseconds. If another edge is not
*detected(high or low) within 1200 microseconds abort the receive. At the expiration
*of the 900 microseconds poll the input. If it is high, we have been sent
*a 0, if it is low, we have been sent a 1. Repeat for each new edge detected.
*/
ISR(PORTF_INT0_vect)
{
	/*
	* An edge was detected!
	* Start the timers, mark that we have received an edge.
	*/		
	if(!edgeFlag)
	{
		edgeFlag = 1; //true
		timeoutFlag = 0;//false
		pollFlag = 0;//false
		PORTH_OUT = 0x00;
		
		//start the timers!
		TCF0_CTRLA = TC_CLKSEL_DIV64_gc;
		TCF1_CTRLA = TC_CLKSEL_DIV64_gc;
	}
	//we have not run out of time to receive the next pulse
	if(edgeFlag && !timeoutFlag)
	{
		//stop the timeout timer
		TCF0_CTRLA = TC_CLKSEL_OFF_gc;
		edgeFlag = 0; //false
		
		if(pollFlag)
		{
			//we have detected a 1
			pollFlag = 0;
			PORTH_OUT = 0x01;
		}
		else
		{
			//we have detected a 0
			PORTH_OUT = 0x00;
		}			
	}	
}

/*
* Timeout timer interrupt vector
* If this overflows we have not detected an edge within
* 1200 microseconds of the first edge! Reset edge flag to false
* set timeout flag to true;
*/
ISR(TCF0_OVF_vect)
{
	TCF0_CTRLA = TC_CLKSEL_OFF_gc;
	timeoutFlag = 1; //true
	edgeFlag = 0; //false
	pollFlag = 0; //false
}

/*
* Polling timer interrupt vector
* If this overflows it is time to poll!
*/
ISR(TCF1_OVF_vect)
{
	TCF1_CTRLA = TC_CLKSEL_OFF_gc;
	pollFlag = 1; //true	
}

/*
* Pushbutton equivalent for sending a 0
*/
ISR(PORTJ_INT0_vect)
{
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
	is1 = 0; //false	
}

/*
* Pushbutton equivalent for sending a 1
*/
ISR(PORTJ_INT1_vect)
{
	TCC0_CTRLA = TC_CLKSEL_DIV64_gc;
	is1 = 1; //true	
}

void main(void)
{
	unsigned long sClk, pClk;
	char recieveString[100];
	is1 = 0; //false
	isOn = 0; //false
	temp = 0;
	accum = 0;
	
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
	* Timer port F0 configuration
	* use this timer as the 1200 microsecond receive time out check.
	*/
	TCF0_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially until a edge is detected
	TCF0_CTRLB = TC_WGMODE_NORMAL_gc; //set waveform generation mode to normal
	TCF0_CTRLC = 0x00; //turn off compares
	TCF0_CTRLD = 0x00; //turn off events
	TCF0_CTRLE = 0x00; //turn off byte mode
	TCF0_PER = 600; //set the top of the period to overflow at 1.2ms. NOTE: to get .6ms set this to 300.
	TCF0_INTCTRLA = 0x01; //set timer f0 overflow interrupts to low level
	
	/*
	* Timer port F1 configuration
	* use this timer for the 900 microsecond level check.
	*/
	TCF1_CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially until a edge is detected.
	TCF1_CTRLB = TC_WGMODE_NORMAL_gc; //set waveform generation mode to normal
	TCF1_CTRLC = 0x00; //turn off compares
	TCF1_CTRLD = 0x00; //turn off events
	TCF1_CTRLE = 0x00; //turn off byte mode
	TCF1_PER = 450; //set the top of the period to overflow at .9ms.
	TCF1_INTCTRLA = 0x01; //set timer f1 overflow interrupts to low level
	
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
	PORTH_OUT = 0xFF;
	
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
	* Port F configuration
	*/
	PORTF_DIR = 0x00; //all pins as input
	PORTF_INTCTRL = 0x01; //turn on interrupt 0 with a low priority
	PORTF_INT0MASK = 0x04; //mask so that only pin 2 can fire an interrupt
	PORTF_PIN2CTRL = 0x00; //set pin 2 to detect a rising and falling edges
	
	/*
	* Port J configuration
	*/
	PORTJ_DIR = 0x00;
	PORTJ_INTCTRL = 0x05;
	PORTJ_PIN0CTRL = 0x01;
	PORTJ_PIN1CTRL = 0x01;
	PORTJ_INT0MASK = 0x01;
	PORTJ_INT1MASK = 0x02;
	
		
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
    }

}
