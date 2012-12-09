//#include <stdlib.h>
#include <avr/io.h>
#include <avr/iox128a1.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC0
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2

//in this state robot is either moving forward, sonar is active
//**************************************************************************************************
returnPackage movingState()
{
	returnPackage localStateVar;

	int haltFlag= 0;
	sonarFlag= 0;
	timeOutFlag= 0;


	setupMotors();
	enableSonar();

	//set RTC to count roughly 5 seconds
	//setRTC(2000);

	RTC_PER= 2000; // should be roughly x milliseconds

	CLK_RTCCTRL=0b00000101;

	RTC_INTCTRL= 0x02; //set overflow interrupt priority to med

	RTC_CTRL= 0x01; //set clock prescaler to one



	// motors A, B will have full duty cycle
    PWMTIMER_CC1= 10000;
    PWMTIMER_CC2= 10000;

	while(haltFlag == 0)
	{
		haltFlag= sonarFlag | timeOutFlag;

	};

	RTC_CTRL= 0x00;
    RTC_CNT= 0;


	switch(haltFlag)
	{
		case 1:
			// break
			//MOTORDIR_OUT |= 0b00111100; // 0x3C
			MOTORDIR_OUT &= STOPMOVING_AND;

			//go to rotate left until no obstacle
			localStateVar.nextState= 2;
			localStateVar.direction= 'L';
			localStateVar.rotateQuantity= 0;
			break;
		case 2:
		case 3:
			// break
			MOTORDIR_OUT &= STOPMOVING_AND;

			//go to rotate right x degrees
			localStateVar.nextState= 2;
			localStateVar.direction= 'R';
			localStateVar.rotateQuantity= 500;
			break;
		default:
			break;
	}

	localStateVar.prevState= 3;
	SONAR1ENABLE_OUT &= 0b11111110;
    return localStateVar;
}



//**************************************************************************************************
//robot is rotating  until haltFlag does not equal 2
//**************************************************************************************************
returnPackage rotateState(returnPackage localStateVar)
{
	RTC_CTRL= 0x00; //turn off RTC

	int rotateFlag= 0;
	stopRotateTimerFlag= 0;
	stopRotateSonarFlag= 0;

	setupMotors();
	enableSonar();


    switch(localStateVar.direction)
    {
        case 'l':
        case 'L':
            // if 0b11000011 is break mode, test for rotate left by forcing bits 3,4 low
            MOTORDIR_OUT |= ROTATELEFT_OR;
            MOTORDIR_OUT &= ROTATELEFT_AND;
            break;
        case 'r':
        case 'R':
            MOTORDIR_OUT |= ROTATERIGHT_OR;
            MOTORDIR_OUT &= ROTATERIGHT_AND;
            break;
        default:
            break;
    }

    // motors A, B will have full duty cycle
    PWMTIMER_CC1= 10000;
    PWMTIMER_CC2= 10000;

	if (localStateVar.rotateQuantity > 0)
	{
		//setRTC(localStateVar.rotateQuantity);
		RTC_PER= 500; // should be roughly x milliseconds

		CLK_RTCCTRL=0b00000101;

		RTC_INTCTRL= 0x02; //set overflow interrupt priority to med

		RTC_CTRL= 0x01; //set clock prescaler to one
	}

	do
	{
		rotateFlag= stopRotateTimerFlag | stopRotateSonarFlag;

	}while(rotateFlag == 0);

	RTC_CTRL= 0x00;
    RTC_CNT= 0;

	// test for break mode break mode
	MOTORDIR_OUT &= STOPMOVING_AND; // 0x3C

    switch(rotateFlag)
    {
        case 1:
            localStateVar.nextState= 3;
            break;
        case 2:
        case 3:
            localStateVar.nextState= 1;
            break;
        default:
            break;
    }
	localStateVar.direction= 'z';
	localStateVar.rotateQuantity= 0;
	return localStateVar;

}



//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void setupMotors()
{
	PWMPORT_DIR |= 0b00001100; //set port direction to output
	//PWMPORT_OUT= 0;

	PWMTIMER_CTRLA =  0x05; //set prescaler to clk/64
	PWMTIMER_CTRLB = 0xC3; //turn on capture C, and D and set waveform generation mode to single slope PWM
	PWMTIMER_CTRLD = 0x00; //turn off events
	PWMTIMER_CTRLE = 0x00; //turn off byte mode
	PWMTIMER_PER = 10000; //set the top of the period so we have a total period of 20ms
	PWMTIMER_INTCTRLA= 0x00;
	PWMTIMER_INTCTRLB= 0x00;

	MOTORDIR_DIR |= 0b11110000; //0xF0
	// force bits 2,6 low
	MOTORDIR_OUT &= MOVEFORWARD_AND; //0xAF
	// force bits 0,4(motor direction) high
	MOTORDIR_OUT |= MOVEFORWARD_OR; //0xA0
}



//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void enableSonar()
{
	SONAR1OUTPORT_DIR &= 0b11111110; //0xFE
	SONAR1OUTPORT_PINCTRL= 0x00; // set input sense to both edges

	SONAR2OUTPORT_DIR &= 0b11101111; //0xFB
	SONAR2OUTPORT_PINCTRL= 0x00; // set input sense to both edges

	EVSYS_CH1MUX= 0x78;//0b01100010 set event channel 1 to portf pin 0
	EVSYS_CH3MUX= 0x7C;//0b01100010 set event channel 1 to port7 pin 4

	EVSYS_CH1CTRL=0x00; // set event channel 1 to no sample filtering
	EVSYS_CH3CTRL=0x00; // set event channel 3 to no sample filtering

	TIMERSONAR1_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR1_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR1_CTRLD = 0xC9; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR1_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR1_PER = 0xFFFF; //set the top of the period to max 16-bit value

	TIMERSONAR1_INTCTRLA= 0x00; // turn other interrupts off
	TIMERSONAR1_INTCTRLB= 0x03; //set capture interrupt to high


	TIMERSONAR2_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR2_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR2_CTRLD = 0xCB; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR2_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR2_PER = 0xFFFF; //set the top of the period to max 16-bit value

	// set pin 1 direction to output
	SONAR1ENABLE_DIR |= 0b00000010; // 0x02;

	// set pin 3 direction to output
	SONAR2ENABLE_DIR |= 0b00001000; // 0x08;

	// set pin 1 high
	SONAR1ENABLE_OUT |= 0b00000010; // 0x02;

	// set pin 3 high
	SONAR2ENABLE_OUT |= 0b00001000; // 0x08;

}

//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void setRTC(int topValue)
{
	RTC_PER= topValue; // should be roughly x milliseconds

	CLK_RTCCTRL=0b00000101;

	RTC_INTCTRL= 0x02; //set overflow interrupt priority to med

	RTC_CTRL= 0x01; //set clock prescaler to one

	RTC_CNT= 0;
}

returnPackage scanState()
{
	/************************************************
	* PWM Setup for the servo using PORTE, TCE0_CCA *
	************************************************/
	/*
	* Timer E0 setup for servo PWM
	*/
	SERVO_PWM.CTRLA = TC_CLKSEL_DIV64_gc; //set timer to div/64
	SERVO_PWM.CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	SERVO_PWM.CTRLC = 0x00; //turn off compares
	SERVO_PWM.CTRLD = 0x00; //turn off events
	SERVO_PWM.CTRLE = 0x00; //turn off byte mode
	SERVO_PWM.PER = 10000; //set the top of the period to 20ms
	SERVO_PWM.CCA = 350; //lower bound, upper bound should be 1150
	SERVO_PWM.INTCTRLA = 0x01; //turn on Overflow interrupt at low priority.
	/*
	* PORT E configuration
	*/
	SERVO_PWM_PORT.DIR |= 0x01; //set pin 0 to output without messing up other pins

	/**************************************************
	* Setup for IR receiver using pulse width capture *
	**************************************************/
	/*
	* IR timer capture configuration
	*/
	IR_PW_CAPTURE.CTRLA = TC_CLKSEL_DIV64_gc; //set clock source sysclk/64= 500KHz
	IR_PW_CAPTURE.CTRLB = 0x10 | TC_WGMODE_NORMAL_gc; //turn on capture channel A and set waveform generation mode normal
	IR_PW_CAPTURE.CTRLD = 0xC8; //set events to Pulse Width capture, no timer delay, and CCA listens to event channel 0, CCB to 1, etc... see datasheet
	IR_PW_CAPTURE.CTRLE = 0x00; //turn off byte mode
	IR_PW_CAPTURE.INTCTRLB = 0x01; //set CCA interrupt to low
	IR_PW_CAPTURE.PER = 0xFFFF; //set the top of the period to max 16-bit value
	/*
	* IR input port configuration
	*/
	IR_INPUT_PORT.DIR = 0x00; //all pins as input
	IR_INPUT_PORT.PIN2CTRL = 0x40; //set pin 2 to detect a rising and falling edges and invert the input to allow for pulse-width capture
	/*
	* Event System Configuration
	*/
	EVSYS_CH0MUX = EVSYS_CHMUX_PORTC_PIN0_gc; //set the event system to send events generated from PortC pin 2 to channel 0
	EVSYS_CH0CTRL = 0x00; //turn off sample filtering

	/*************************
	* Locally Used Variables *
	*************************/
	int keepLooping = 1;//true
	returnPackage localStatePackage; //local struct to return at end of function
	int degreeVar = 0; //used for seeing which degree the servo is at.
	int degreeSideVar = 0; //used for determining left or right, 0 = left, 1 = right

	while(keepLooping)
	{
		switch(scanVar)
		{
			case 0: //we are still scanning
			break;

			case 1: //we got a pulse
			/*************************************************************
			* Section of state to handle calculating the degrees to turn *
			* and put it into the structure to return					 *
			*************************************************************/
			degreeVar = SERVO_PWM.CCA * 2; //double TCE0_CCA gives you microseconds

			if(degreeVar > 1500)
			{
				//we need to turn right
				degreeVar = degreeVar - 1500; //normalize this to be between 0-900 microseconds
				degreeVar /= 10; //this will give us a value in degrees as 10 microseconds = 1 degree
				degreeSideVar = 1; //this indicates this will be degreeVar degrees to the right.
			}
			else
			{
				//we need to turn left
				degreeVar = 1500 - degreeVar; //normalize this to be between 0-900 microseconds
				degreeVar /= 10; //this will give us a value in degrees as 10 microseconds = 1 degree
				degreeSideVar = 0; //this indicates this will be degreeVar degrees to the left.
			}

			//need to do something with this value and motors here.
			//temporary code below:
			PORTH_OUT = degreeVar;
			PORTH_OUT |= degreeSideVar << 7;

			localStatePackage.rotateQuantity = degreeVar; //give the degrees we need to turn
			
			if(degreeSideVar) //if we need to turn right
			{
				localStatePackage.direction = 'R'; //set direction to right
			}
			else //if we need to turn left
			{
				localStatePackage.direction = 'L'; //set direction to left
			}

			localStatePackage.prevState = 1; //indicate we were in the scan state
			localStatePackage.nextState = 2; //we need to go to rotate state

			keepLooping = 0; //false, exit the loop
			break;

			case 2: //we have finished scanning and have recieved no pulses
			localStatePackage.prevState = 1; //indicate we were in the scan state
			localStatePackage.nextState = 3; //we need to go to move state
			keepLooping = 0; //false, exit the loop
			break;

			default: //oh shit what the fucks
			break;
		}
	}

	return localStatePackage;
}


void setupTransmit()
{
	/*
	* Timer port F0 configuration
	* The current values of PER and CCA for this timer will result in the 38Khz oscillating signal needed
	* in order to generate 1's and 0's that the reciever recognizes. Do not change these values.
	*/
	TRANSMIT_OSCILLATOR.CTRLA = TC_CLKSEL_DIV64_gc; //set timer
	TRANSMIT_OSCILLATOR.CTRLB = 0x10 | TC_WGMODE_SS_gc; //turn on capture(CCAEN) and set waveform generation mode to PWM
	TRANSMIT_OSCILLATOR.CTRLC = 0x00; //turn off compares
	TRANSMIT_OSCILLATOR.CTRLD = 0x00; //turn off events
	TRANSMIT_OSCILLATOR.CTRLE = 0x00; //turn off byte mode
	TRANSMIT_OSCILLATOR.PER = 12; //set the top of the period
	TRANSMIT_OSCILLATOR.CCA = 6; //set the compare register value to achieve 50% duty cycle at
	TRANSMIT_OSCILLATOR.INTCTRLB = 0x00; //set the CCA interrupt to low priority.

	/*
	* Timer port F1 configuration
	*/
	TRANSMIT_TIMER.CTRLA = TC_CLKSEL_OFF_gc; //set timer to be off intially
	TRANSMIT_TIMER.CTRLB = TC_WGMODE_NORMAL_gc; //set timer to normal operation
	TRANSMIT_TIMER.CTRLC = 0x00; //turn off compares
	TRANSMIT_TIMER.CTRLD = 0x00; //turn off events
	TRANSMIT_TIMER.CTRLE = 0x00; //turn off byte mode
	TRANSMIT_TIMER.PER = 500; //1ms
	TRANSMIT_TIMER.INTCTRLA = 0x01; //set the overflow interrupt to low priority

	/*
	* PORT F setup
	* set direction for pin 1 without upsetting other pins
	*/
	TRANSMIT_PORT.DIR |= 0x01;
}
